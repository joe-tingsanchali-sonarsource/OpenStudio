#!/usr/bin/groovy

String failingTestExcludeRegex() {
  // If re-enabling this, append `-E \"${failingTestExcludeRegex()}\"` to the ctest command below
  failingTests = [
  ]

  String result = '^(' + failingTests.join('|') + ")\$"

  return result
}

String failingTestExcludeRegexMac() {
  // If re-enabling this, append `-E \"${failingTestExcludeRegex()}\"` to the ctest command below
  failingTests = [
     'BCLFixture.BCLComponent' // Test is failing after updating from 10.14 to 10.15. It seems related to copying the bcl resource content during the build"
   ]

  String result = '^(' + failingTests.join('|') + ")\$"

  return result
}

Boolean checkFileExistsS3(String filename, String bucket, String bucket_path) {
  withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
    s3files = s3FindFiles bucket: "${bucket}", glob: "${bucket_path}/${filename}"

    if (s3files.length > 0 ) {
      return true
    }
    else {
      return false
    }
  }
}

def getOSversion(openstudio_cli_path) {
  os_version = sh(returnStdout: true, script: "${openstudio_cli_path} --version")
  println("OS Version is ${os_version}")
  // split version and sha
  os_version_elements = os_version.split('\\+')
  // os_version_version will have version and extension
  if (os_version_elements[0].contains('-')) {
    os_version_with_ext = os_version_elements[0].split('-')
  }
  else {
    os_version_with_ext = [ os_version_elements[0] ,'']
  }
  // Build array with Version, Version-Ext and sha
  def versionArray = [os_version_with_ext[0], os_version_with_ext[1], os_version_elements[0]  ]
  return versionArray
}

def call() {
  Boolean ifclean = true
  parallel(
    'ubuntu-2204':  {
      // The vm node is running ubuntu 18.04 but using a 22.04 docker image on the vm
      node('nrel_docker_vcpu36_1') {
        String build_folder = 'OS-build-release-v2'
        String source_folder = 'Openstudio'
        String linux_base = '/srv/data/jenkins/docker-volumes/ubuntu-2204-nightly'
        String linux_base_build = "/srv/data/jenkins/docker-volumes/ubuntu-2204-nightly/${build_folder}"
        String linux_source = "/srv/data/jenkins/docker-volumes/ubuntu-2204-nightly/${source_folder}"

        dir(linux_base)  {
          // Need to mount the conan directory for data cache
          docker.image('kuangwenyi/openstudio-cmake-tools:jammy').inside('-v /srv/data/jenkins/docker-volumes/conan-data/:/home/root -u root -d') {
            if (ifclean) {
                sh('rm -rf * && rm -rf .[a-g]*')
                sh("git clone --single-branch --branch ${env.BRANCH_NAME} https://github.com/NREL/OpenStudio ${source_folder}")
            }
            stage('check enviroment') {
              dir(linux_source) {
                sh('conan --version')
                sh('conan remote add -f nrel-v2 http://conan.openstudio.net/artifactory/api/conan/conan-v2')
                sh('ninja --version')
              // sh('conan profile detect')
              }
            }

            stage('setup conan') {
              dir(linux_source) {
                sh("conan install . --output-folder=../${build_folder} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
              }
            }
            stage('build openstudio') {
              dir(linux_base_build) {
                sh('''
                chmod +x ./conanbuild.sh && ./conanbuild.sh  &&
                . ./conanbuild.sh && env
                ''')
                sh('env')
                sh("""
                  . ./conanbuild.sh &&
                  cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE:STRING=Release \
                    -DBUILD_TESTING:BOOL=ON -DCPACK_BINARY_TGZ:BOOL=ON -DCPACK_BINARY_DEB:BOOL=ON \
                    -DCPACK_BINARY_IFW:BOOL=OFF -DCPACK_BINARY_NSIS:BOOL=OFF -DCPACK_BINARY_RPM:BOOL=OFF -DCPACK_BINARY_STGZ:BOOL=OFF \
                    -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF \
                    -DBUILD_PYTHON_BINDINGS:BOOL=ON -DBUILD_PYTHON_PIP_PACKAGE:BOOL=ON \
                    -DPYTHON_VERSION=3.12.2 -DPython_ROOT_DIR:PATH=$HOME/.pyenv/versions/3.12.2  \
                    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON \
                  ${linux_source} &&
                    ninja -j 16 package
                """)
              }
            }
            stage('ctests openstudio') {
              dir(linux_base_build) {
                  try {
                  sh('''
                    #!/bin/bash
                     ruby -v
                     ''')
                  sh '''
                     ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
                     '''
                  currentBuild.result = 'SUCCESS'
                } catch (Exception err) {
                  try {
                    sh '''#!/bin/bash
                        ruby -v
                        ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
                        '''
                    currentBuild.result = 'SUCCESS'
                  } catch (Exception err_2) {
                    currentBuild.result = 'FAILURE'
                  }
                } finally {
                  try {
                    sh("cp -r ${linux_base_build}/Testing ${linux_base_build}/Testing-Ubuntu-2204")
                    archiveArtifacts artifacts: 'Testing-Ubuntu-2204/', fingerprint: true

                    xunit(
                      testTimeMargin: '9000',
                      thresholdMode: 1,
                      thresholds: [
                        skipped(failureThreshold: '0'),
                        failed(failureThreshold: '0')
                      ],
                    tools: [CTest(
                      pattern: 'Testing-Ubuntu-2204/**/*.xml',
                      deleteOutputFiles: true,
                      failIfNotNew: false,
                      skipNoTestFiles: true,
                      stopProcessingIfError: false
                    )])
                  } catch (Exception e) {
                    e.printStackTrace()
                  }
                  }
              }
            }
            stage('package and upload openstudio') {
              dir(linux_base_build) {
              files = findFiles(glob: '*.deb')
              println "Found Files: ${files}"
              if (files.length == 1) {
                if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
                  withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                    s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*.deb',  acl:'PublicRead')
                  }
                  sh(script: "md5sum ${files[0].path}",
                    label: 'Compute MD5 Hash for DEB')
                }
                else {
                  println "File ${files[0].name} already in s3 bucket. Skipping upload"
                }
              }

              files = findFiles(glob: '*OpenStudio*x86_64.tar.gz')
              println "Found Files: ${files}"
              if (files.length == 1) {
                if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
                  withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                    s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*OpenStudio*x86_64.tar.gz',  acl:'PublicRead')
                  }
                  sh(script: "md5sum ${files[0].path}",
                    label: 'Compute MD5 Hash for tar.gz')
                }
                else {
                  println "File ${files[0].name} already in s3 bucket. Skipping upload"
                }
              }
              }
            }

            stage('build docker container') {
              dir(linux_base_build) {
                def files = findFiles(glob: '*22.04-x86_64.deb')

                def binaryUrl = "https://openstudio-ci-builds.s3.amazonaws.com/${env.BRANCH_NAME}/${files[0].name}"
                binaryUrl = binaryUrl.replace('+', '%2B')
                println("${binaryUrl}")
                // get version
                //2 situtaion:
                // 1. 3.3.0+1c1b0d7e3e
                // 2. 3.3.0-rc1+1c1b0d7e3e
                os_version = sh(returnStdout: true, script: './Products/openstudio --version')
                println("OS Version is ${os_version}")
                // split version and sha
                os_version_elements = os_version.split('\\+')
                println(os_version_elements.toString())
                // os_version_version will have version and extension
                if (os_version_elements[0].contains('-')) {
                  os_version_with_ext = os_version_elements[0].split('-')
                }
                else {
                  os_version_with_ext = [ os_version_elements[0] ,os_version_elements[1]]
                }

                println("OS Version is ${os_version_with_ext[0]}")
                println("OS Version ext is ${os_version_with_ext[1]}")

                if (env.BRANCH_NAME == 'develop') {
                  println('This is develop branch')
                  docker_image_tag = 'develop'
                } else {
                  println('This is not develop branch')
                  docker_image_tag = "${os_version_with_ext[0]}-${os_version_with_ext[1]}"
                }

                println("env.BRANCH_NAME is ${env.BRANCH_NAME}, ${env.BRANCH_NAME == 'develop'}")
                println("docker_image_tag is ${docker_image_tag}")
                sh('rm -rf docker-openstudio')
                sh('git clone --single-branch --branch develop https://github.com/NREL/docker-openstudio')

                withCredentials([string(credentialsId: 'github_actions', variable: 'GITHUB_TOKEN')]) {
                  sh("""
                  echo ${docker_image_tag}
                  cd docker-openstudio
                  gh workflow run 'manual_update_develop' -f docker_image_tag=${docker_image_tag} -f os_installer_link=${binaryUrl} -f os_version=${os_version_with_ext[0]} -f os_version_ext=${os_version_with_ext[1]}
                  """)
                }
              }
            }
          }
        }
      }
    },

    'ubuntu-2404':{
            // The vm node is running ubuntu 18.04 but using a 24.04 docker image on the vm
      node("openstudio-ubuntu-1804-nrel") {

        String linux_base = "/srv/jenkins/openstudio/git/nightly/ubuntu_2404";
        String build_folder = 'OS-build-release-v2'
        String source_folder = 'Openstudio'
        String linux_base_build = "${linux_base}/${build_folder}"
        String linux_source = "${linux_base}/${source_folder}"

        dir(linux_base)  {
          docker.image('kuangwenyi/openstudio-cmake-tools:noble').inside('-v /srv/jenkins/openstudio/docker-volumes/conan-data/:/home/root -u root') {
              stage("clean workspace") {

              // not ideal but we are in relative dir inside the container. The linux_base is mounted here so this is what we are
              // deleting
              if(ifclean){
                sh("rm -rf * && rm -rf .[a-g]*")
                sh("git clone --single-branch --branch ${env.BRANCH_NAME} https://github.com/NREL/OpenStudio ${source_folder}")
                println("Cleaning workspace")

              }
            }
            stage('check enviroment') {
                dir(linux_source) {
                  sh('conan --version')
                  sh('cat /home/root/.conan2/global.conf')
                  sh('conan remote add -f nrel-v2 http://conan.openstudio.net/artifactory/api/conan/conan-v2')
                  sh('ninja --version')
                  // sh('conan profile detect') need run in the initial the conan
                  //then we need modify the global.conf
                }
            }
            stage('setup conan'){
              dir(linux_source){
                sh("conan install . --output-folder=../${build_folder} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
              }
            }
            stage('build openstudio') {
              dir(linux_base_build) {
                 sh("""
                  . ./conanbuild.sh &&
                  cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE:STRING=Release \
                  -DBUILD_TESTING:BOOL=ON -DCPACK_BINARY_DEB:BOOL=ON -DCPACK_BINARY_TGZ:BOOL=ON \
                  -DCPACK_BINARY_IFW:BOOL=OFF -DCPACK_BINARY_NSIS:BOOL=OFF -DCPACK_BINARY_RPM:BOOL=OFF -DCPACK_BINARY_STGZ:BOOL=OFF \
                  -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF \
                  -DBUILD_PYTHON_BINDINGS:BOOL=ON -DBUILD_PYTHON_PIP_PACKAGE:BOOL=OFF -DPYTHON_VERSION:STRING=3.12.2 \
                    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON \
                  ${linux_source} &&
                    ninja -j 16 package
                """)
                }
              }
            stage("ctests openstudio"){
              dir(linux_base_build) {
                                try {
                  sh("""
                    #!/bin/bash
                     ruby -v
                  """)

                  sh """#!/bin/bash
                     ruby -v
                     ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
                     """
                  currentBuild.result = "SUCCESS"
                } catch (Exception err) {

                  try {
                    sh """#!/bin/bash
                        ruby -v
                        ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
                        """
                    currentBuild.result = "SUCCESS"
                  } catch (Exception err_2) {
                    currentBuild.result = "FAILURE"
                  }
                } finally {

                  try {

                    sh("cp -r ${linux_base_build}/Testing ${linux_base_build}/Testing-Ubuntu-2404")
                    archiveArtifacts artifacts: 'Testing-Ubuntu-2404/', fingerprint: true

                    xunit (
                      testTimeMargin: '9000',
                      thresholdMode: 1,
                      thresholds: [
                        skipped(failureThreshold: '0'),
                        failed(failureThreshold: '0')
                      ],
                    tools: [CTest(
                      pattern: 'Testing-Ubuntu-2404/**/*.xml',
                      deleteOutputFiles: true,
                      failIfNotNew: false,
                      skipNoTestFiles: true,
                      stopProcessingIfError: false
                    )])
                  } catch (Exception e) {
                    e.printStackTrace();
                  }
                }
              }
            }

            stage("package and upload openstudio"){
              dir(linux_base_build){

              files = findFiles(glob: "*.deb")
              println "Found Files: ${files}"
              if (files.length == 1) {
                if (!checkFileExistsS3(files[0].name, "openstudio-ci-builds", env.BRANCH_NAME )) {
                  withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                    s3Upload(bucket:"openstudio-ci-builds", path:"${env.BRANCH_NAME}/", workingDir: ".", includePathPattern:"*.deb",  acl:"PublicRead")
                  }
                  sh(script: "md5sum ${files[0].path}",
                    label: "Compute MD5 Hash for DEB");
                }
                else {
                  println "File ${files[0].name} already in s3 bucket. Skipping upload"
                }
              }

              files = findFiles(glob: '*OpenStudio*x86_64.tar.gz')
              println "Found Files: ${files}"
              if (files.length == 1) {
                if (!checkFileExistsS3(files[0].name, "openstudio-ci-builds", env.BRANCH_NAME )) {
                  withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                    s3Upload(bucket:"openstudio-ci-builds", path:"${env.BRANCH_NAME}/", workingDir: ".", includePathPattern:"*OpenStudio*x86_64.tar.gz",  acl:"PublicRead")
                  }
                  sh(script: "md5sum ${files[0].path}",
                    label: "Compute MD5 Hash for tar.gz");
                }
                else {
                  println "File ${files[0].name} already in s3 bucket. Skipping upload"
                }
              }
              }
            }

          }
        }
      }
    },

    // 'centos-7': {
    //     node('openstudio-ubuntu-1804-nrel') {
    //     String linux_base = '/srv/jenkins/openstudio/git/nightly/centos7'
    //     String build_folder = 'OS-build-release-v2'
    //     String source_folder = 'Openstudio'
    //     String linux_base_build = "/srv/jenkins/openstudio/git/nightly/centos7/${build_folder}"
    //     String linux_source = "/srv/jenkins/openstudio/git/nightly/centos7/${source_folder}"
    //     dir(linux_base)  {
    //       docker.image('kuangwenyi/openstudio-cmake-tools:centos7').inside('-v /srv/jenkins/openstudio/docker-volumes/conan-data-centos7/:/root -u root') {
    //         stage('clean workspace') {
    //           // not ideal but we are in relative dir inside the container. The linux_base is mounted here so this is what we are
    //           // deleting
    //           if (ifclean) {
    //             println('Cleaning workspace')
    //             sh('rm -rf * && rm -rf .[a-g]*')
    //             sh("git clone --single-branch --branch ${env.BRANCH_NAME} https://github.com/NREL/OpenStudio ${source_folder}")
    //           }
    //         }
    //         stage('check enviroment') {
    //           dir(linux_source) {
    //             sh('pip uninstall -y conan')
    //             sh('pip install conan')
    //             sh('conan --version')
    //             // sh('conan profile detect') initial
    //             sh('cat /root/.conan2/global.conf')
    //             sh('conan remote add -f nrel-v2 http://conan.openstudio.net/artifactory/api/conan/conan-v2')
    //             sh('ninja --version')
    //             sh('scl enable devtoolset-10 bash')
    //           }
    //         }

    //         stage('setup conan') {
    //           dir(linux_source) {
    //             sh("conan install . --output-folder=../${build_folder} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
    //           }
    //         }

    //         stage('build openstudio') {
    //           dir(linux_base_build) {
    //             sh('''

    //             chmod +x ./conanbuild.sh && ./conanbuild.sh  &&
    //             . ./conanbuild.sh && env
    //             ''')
    //             sh('env')
    //             sh("""
    //               . ./conanbuild.sh &&
    //               cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE:STRING=Release \
    //               -DBUILD_TESTING:BOOL=ON -DCPACK_BINARY_RPM:BOOL=ON -DCPACK_BINARY_TGZ:BOOL=ON \
    //               -DCPACK_BINARY_IFW:BOOL=OFF -DCPACK_BINARY_DEB:BOOL=OFF -DCPACK_BINARY_NSIS:BOOL=OFF \
    //               -DCPACK_BINARY_STGZ:BOOL=OFF -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF \
    //               -DBUILD_PYTHON_BINDINGS:BOOL=ON -DPYTHON_VERSION:STRING=3.8 \
    //                 -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON \
    //               ${linux_source} &&
    //                 ninja -j 16 package
    //             """)
    //           }
    //         }

    //         stage('ctests openstudio') {
    //           dir(linux_base_build) {
    //               try {
    //               sh('''
    //               #!/bin/bash
    //                  ruby -v
    //               ''')
    //               sh '''#!/bin/bash
    //                  ruby -v
    //                  ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
    //                  '''
    //               currentBuild.result = 'SUCCESS'
    //             } catch (Exception err) {
    //               try {
    //                 sh '''#!/bin/bash
    //                     ruby -v
    //                     ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
    //                     '''
    //                 currentBuild.result = 'SUCCESS'
    //               } catch (Exception err_2) {
    //                 currentBuild.result = 'FAILURE'
    //               }
    //             } finally {
    //               try {
    //                 sh("cp -r ${linux_base_build}/Testing ${linux_base_build}/Testing-centos-7")
    //                 archiveArtifacts artifacts: 'Testing-centos-7/', fingerprint: true

    //                 xunit(
    //                   testTimeMargin: '9000',
    //                   thresholdMode: 1,
    //                   thresholds: [
    //                     skipped(failureThreshold: '0'),
    //                     failed(failureThreshold: '0')
    //                   ],
    //                 tools: [CTest(
    //                   pattern: 'Testing-centos-7/**/*.xml',
    //                   deleteOutputFiles: true,
    //                   failIfNotNew: false,
    //                   skipNoTestFiles: true,
    //                   stopProcessingIfError: false
    //                 )])
    //               } catch (Exception e) {
    //                 e.printStackTrace()
    //               }
    //               }
    //           }
    //         }

    //         stage('package and upload openstudio') {
    //           dir(linux_base_build) {
    //           files = findFiles(glob: '*.rpm')
    //           println "Found Files: ${files}"
    //           if (files.length == 1) {
    //             if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
    //               withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
    //                 s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*.rpm',  acl:'PublicRead')
    //               }
    //               sh(script: "md5sum ${files[0].path}",
    //                 label: 'Compute MD5 Hash for DEB')
    //             }
    //             else {
    //               println "File ${files[0].name} already in s3 bucket. Skipping upload"
    //             }
    //           }

    //           files = findFiles(glob: '*OpenStudio*x86_64.tar.gz')
    //           println "Found Files: ${files}"
    //           if (files.length == 1) {
    //             if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
    //               withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
    //                 s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*OpenStudio*x86_64.tar.gz',  acl:'PublicRead')
    //               }
    //               sh(script: "md5sum ${files[0].path}",
    //                 label: 'Compute MD5 Hash for tar.gz')
    //             }
    //             else {
    //               println "File ${files[0].name} already in s3 bucket. Skipping upload"
    //             }
    //           }
    //           }
    //         }
    //       }
    //     }
    //     }
    // },
    'windows': {
      node('openstudio-win-server-vs-2019-ssh-incr-on-prem') {
        String windows_base = 'D:/OSN'
        String source_folder = 'Openstudio'
        String windows_source = "${windows_base}/${source_folder}"
        String windows_base_build = "${windows_base}/OS-build-release-v2"
        String nuget_test_path = "${windows_base_build}/csharp/nuget_test"
        String nuget_install_path = 'D:/jenkins/nuget_packages'

        stage('setup enviroment') {
          dir(windows_base) {
            if (ifclean) {
              println('Cleaning workspace')
              deleteDir()
              powershell("git clone --single-branch --branch ${env.BRANCH_NAME} https://github.com/NREL/OpenStudio ${source_folder}")
            }
          }
        }

        stage('setup conan') {
          dir(windows_source) {
            powershell("conan install . --output-folder=${windows_base_build} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
          }
        }

        stage('build windows') {
          dir(windows_base_build) {
            bat('''
              call ".\\conanbuild.bat"
              cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DBUILD_CSHARP_BINDINGS:BOOL=OFF -DBUILD_DOCUMENTATION:BOOL=ON -DBUILD_TESTING:BOOL=ON -DBUILD_PACKAGE:BOOL=ON -DCMAKE_BUILD_TYPE=Release -DCPACK_BINARY_DEB:BOOL=OFF -DCPACK_BINARY_IFW:BOOL=ON -DCPACK_BINARY_NSIS:BOOL=OFF -DCPACK_BINARY_RPM:BOOL=OFF -DCPACK_BINARY_STGZ:BOOL=OFF -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TGZ:BOOL=ON -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF  -DBUILD_PYTHON_BINDINGS:BOOL=ON -DBUILD_PYTHON_PIP_PACKAGE:BOOL=OFF -DPYTHON_VERSION:STRING=3.12 -DPython_ROOT_DIR:PATH=C:/Python312 ../Openstudio
              ninja -j 16 package
            ''')
          }
        }

        stage('ctest openstudio') {
          dir(windows_base_build) {
            try {
              bat('''
                ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
              ''')
              currentBuild.result = 'SUCCESS'
            } catch (Exception err) {
              try {
                bat('''
                  ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
                ''')
                currentBuild.result = 'SUCCESS'
              } catch (Exception err_2) {
                currentBuild.result = 'FAILURE'
              }
            } finally {
              try {
                bat("cp -r ${windows_base_build}/Testing ${windows_base_build}/Testing-Windows")
                archiveArtifacts artifacts: 'Testing-Windows/', fingerprint: true
                xunit(
                  testTimeMargin: '9000',
                  thresholdMode: 1,
                  thresholds: [
                    skipped(failureThreshold: '0'),
                    failed(failureThreshold: '0')
                  ],
                  tools: [CTest(
                    pattern: 'Testing-Windows/**/*.xml',
                    deleteOutputFiles: true,
                    failIfNotNew: false,
                    skipNoTestFiles: true,
                    stopProcessingIfError: false
                  )]
                )
              } catch (Exception e) {
                e.printStackTrace()
              }
            }
          }
        }

        stage('code sign openstudio') {
          dir(windows_base_build) {
            zip zipFile: "build-${env.BUILD_ID}.zip", archive: false, glob: '*.exe'
          }

          dir('C:/code-signing-client') {
            powershell("node C:/code-signing-client/code-signing.js ${windows_base_build}/build-${env.BUILD_ID}.zip -t 4800000")
          }

          dir(windows_base_build) {
              unzip zipFile: "build-${env.BUILD_ID}.signed.zip"
          }

          dir(windows_base_build) {
            powershell('cpack')
            zip zipFile: "OpenStudio-Installer-${env.BUILD_ID}.zip", glob: 'OpenStudio*.exe'
          }

          dir('C:/code-signing-client') {
            powershell("node C:/code-signing-client/code-signing.js ${windows_base_build}/OpenStudio-Installer-${env.BUILD_ID}.zip -t 4800000")
          }

          dir(windows_base_build) {
            unzip zipFile: "OpenStudio-Installer-${env.BUILD_ID}.signed.zip", dir:'signed'
          }
        }

        stage('publish signed openstudio') {
          dir(windows_base_build) {
            def files = findFiles(glob: 'signed/OpenStudio*.exe')
            if (files.length == 1) {
              if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', "${env.BRANCH_NAME}/signed/")) {
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                  s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/signed/", workingDir: "${windows_base_build}/signed", includePathPattern:'OpenStudio*.exe',  acl:'PublicRead')
                }
                powershell(script: "Get-FileHash ${files[0].path} -Algorithm MD5",
                           label: 'Compute MD5 Hash for EXE')
              }
              else {
                println "File ${files[0].name} already in s3 bucket. Skipping upload"
              }
            }

            files = findFiles(glob: '_CPack_Packages/win64/TGZ/*.tar.gz')
            if (files.length == 1) {
              if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', "${env.BRANCH_NAME}/signed/")) {
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                  s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/signed/", workingDir: "${windows_base_build}/_CPack_Packages/win64/TGZ", includePathPattern:'*.tar.gz',  acl:'PublicRead')
                }
                powershell(script: "Get-FileHash ${files[0].path} -Algorithm MD5",
                           label: 'Compute MD5 Hash for tar.gz')
              }
              else {
                println "File ${files[0].name} already in s3 bucket. Skipping upload"
              }
            }
          }
        }
      }
    },

    'mac-os-x64': {
      node('openstudio_develop_osx_10_15') {
        String mac_base = '/Users/jenkins/git/OpenStudioFull'
        String build_folder = 'OS-build-release-v2'
        String source_folder = 'Openstudio'
        String mac_base_build = "/Users/jenkins/git/OpenStudioFull/${build_folder}"
        String mac_base_source = "/Users/jenkins/git/OpenStudioFull/${source_folder}"

        stage('environment setup') {
          dir(mac_base)  {
            // deleteDir()
            //the conan installed in mac is installed by brew and hardcoded into bash_profile
            //TODO: find a better approach to install conan
            sh('ls -la ~/.pyenv || true')
            sh('ls -la ~/.pyenv/versions || true')
            sh("ls -la ~/.pyenv || true");
            sh("ls -la ~/.pyenv/versions || true");
            sh("~/.pyenv/versions/3.12.2/bin/python3.12 -m pip install requests packaging twine pytest");
            sh("~/.pyenv/versions/3.12.2/bin/python3.12 --version");
            sh("conan --version");
            sh("~/.pyenv/versions/3.12.2/bin/python3.12 -c \"from distutils import sysconfig; print(';'.join([sysconfig.PREFIX,sysconfig.EXEC_PREFIX,sysconfig.BASE_EXEC_PREFIX]))\" || true");

            if (ifclean) {
              println('Cleaning workspace')
              sh('rm -rf * && rm -rf .[a-g]*')
              sh("git clone --single-branch --branch ${env.BRANCH_NAME} https://github.com/NREL/OpenStudio ${source_folder}" )
            }
          }
        }
      stage('setup conan') {
        dir(mac_base_source) {
            sh("conan install . --output-folder=../${build_folder} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
        }
      }

      stage('build openstudio') {
        dir(mac_base_build) {
          sh('''
          chmod +x ./conanbuild.sh && ./conanbuild.sh  &&
          . ./conanbuild.sh && env
          ''')
          sh('env')
          sh("""
            . ./conanbuild.sh &&
            cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DENABLE_COVERAGE:BOOL=OFF -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.15 \
                -DBUILD_TESTING:BOOL=ON -DCPACK_BINARY_IFW:BOOL=ON -DCPACK_BINARY_TGZ:BOOL=ON \
                -DCPACK_BINARY_DEB:BOOL=OFF -DCPACK_BINARY_NSIS:BOOL=OFF -DCPACK_BINARY_RPM:BOOL=OFF -DCPACK_BINARY_STGZ:BOOL=OFF \
                -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF \
                -DBUILD_PYTHON_BINDINGS:BOOL=ON -DBUILD_PYTHON_PIP_PACKAGE:BOOL=OFF -DPYTHON_VERSION:STRING=3.12.2 -DPython_ROOT_DIR:PATH=$HOME/.pyenv/versions/3.12.2 \
                ../${source_folder} &&
            ninja -j 16 package
          """)
        }
      }

      stage('ctests openstudio') {
        dir(mac_base_build) {
          try {
            sh '''#!/bin/bash
               source /usr/local/rvm/environments/ruby-3.2.2
               echo "Ensuring Bundler v2.4.10 is installed for RVM Ruby 3.2.2..."
               gem install bundler -v 2.4.10 --conservative --no-document
               bundle -v
               ruby -v
               ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
               '''
            currentBuild.result = 'SUCCESS'
          } catch (Exception err) {
            try {
              sh '''#!/bin/bash
                 source /usr/local/rvm/environments/ruby-3.2.2
                 echo "Ensuring Bundler v2.4.10 is installed for RVM Ruby 3.2.2..."
                 gem install bundler -v 2.4.10 --conservative --no-document
                 bundle -v
                 ruby -v
                 ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
                 '''
              currentBuild.result = 'SUCCESS'
            } catch (Exception err_2) {
              currentBuild.result = 'FAILURE'
            }
          } finally {
            try {
              sh("cp -r ${mac_base_build}/Testing ${mac_base_build}/Testing-x64-mac")
              archiveArtifacts artifacts: 'Testing-x64-mac/', fingerprint: true
              xunit(
                testTimeMargin: '9000',
                thresholdMode: 1,
                thresholds: [
                  skipped(failureThreshold: '0'),
                  failed(failureThreshold: '0')
                ],
              tools: [CTest(
                pattern: 'Testing-x64-mac/**/*.xml',
                deleteOutputFiles: true,
                failIfNotNew: false,
                skipNoTestFiles: true,
                stopProcessingIfError: false
              )])
            } catch (Exception e) {
              e.printStackTrace()
            }
          }
        }
      }

      stage('package and upload openstudio') {
        dir(mac_base_build) {
          files = findFiles(glob: '*.dmg')
          println "Found Files: ${files}"
          if (files.length == 1) {
            if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
              withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*.dmg',  acl:'PublicRead')
              }
              sh(script: "md5sum ${files[0].path}",
                label: 'Compute MD5 Hash for tar.gz')
            }
            else {
              println "File ${files[0].name} already in s3 bucket. Skipping upload"
            }
          }

          files = findFiles(glob: '*OpenStudio*x86_64.tar.gz')
          println "Found Files: ${files}"
          if (files.length == 1) {
            if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
              withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
                s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*OpenStudio*x86_64.tar.gz',  acl:'PublicRead')
              }
              sh(script: "md5sum ${files[0].path}",
                label: 'Compute MD5 Hash for tar.gz')
            }
            else {
              println "File ${files[0].name} already in s3 bucket. Skipping upload"
            }
          }
        }
      }//end package and upload openstudio

      }
  },

    // 'mac-os-m1': {
    //   node('m1-mac-node-1') {
    //     String mac_base = '/Users/jenkins/git/OpenStudioFull'
    //     String build_folder = 'OS-build-release-v2'
    //     String source_folder = 'Openstudio'
    //     String mac_base_build = "/Users/jenkins/git/OpenStudioFull/${build_folder}"
    //     String mac_base_source = "/Users/jenkins/git/OpenStudioFull/${source_folder}"

    //   stage('environment setup') {
    //       dir(mac_base)  {
    //         sh('ruby --version')
    //         sh('ls -la ~/.pyenv || true')
    //         sh('ls -la ~/.pyenv/versions || true')
    //         sh('~/.pyenv/versions/3.12.1/bin/python3.12 -m pip install requests packaging twine')
    //         sh('~/.pyenv/versions/3.12.1/bin/python3.12 --version')
    //         sh('conan --version')
    //         sh('pwd')
    //         // sh("~/.pyenv/versions/3.8.13/bin/python3.8 -c \"from distutils import sysconfig; print(';'.join([sysconfig.PREFIX,sysconfig.EXEC_PREFIX,sysconfig.BASE_EXEC_PREFIX]))\" || true");
    //         sh('git config --global core.compression 0')

    //         if (ifclean || !findFiles().any()) { //if there is thing and we don't want to clean it up, we don't clone
    //           //if thetere is nothing, we clone
    //           sh('rm -rf * && rm -rf .[a-g]*')
    //           println('Cleaning workspace')
    //         }
    //       }
    //       dir(mac_base_source) {
    //         checkout([
    //             $class: 'GitSCM',
    //             branches:  [[name: "*/${env.BRANCH_NAME}"]],
    //             doGenerateSubmoduleConfigurations: false,
    //             submoduleCfg: [],
    //             userRemoteConfigs: [[credentialsId: 'ci-commercialbuildings-github',
    //             url: 'git@github.com:NREL/OpenStudio.git']]
    //         ])
    //         sh('conan remote add -f nrel-v2 http://conan.openstudio.net/artifactory/api/conan/conan-v2')
    //       }
    //   }

    //   stage('setup conan') {
    //     dir(mac_base_source) {
    //       sh("conan install . --output-folder=../${build_folder} --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja -s compiler.cppstd=20 -s build_type=Release")
    //     }
    //   }

    //   stage('build openstudio') {
    //     dir(mac_base_build) {
    //       sh('''
    //       chmod +x ./conanbuild.sh && ./conanbuild.sh  &&
    //       . ./conanbuild.sh && env
    //       ''')
    //       sh('env')
    //       sh("""
    //         . ./conanbuild.sh &&
    //         cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DENABLE_COVERAGE:BOOL=OFF -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.15 \
    //             -DBUILD_TESTING:BOOL=ON -DCPACK_BINARY_IFW:BOOL=ON -DCPACK_BINARY_TGZ:BOOL=ON \
    //             -DCPACK_BINARY_DEB:BOOL=OFF -DCPACK_BINARY_NSIS:BOOL=OFF -DCPACK_BINARY_RPM:BOOL=OFF -DCPACK_BINARY_STGZ:BOOL=OFF \
    //             -DCPACK_BINARY_TBZ2:BOOL=OFF -DCPACK_BINARY_TXZ:BOOL=OFF -DCPACK_BINARY_TZ:BOOL=OFF \
    //             -DBUILD_PYTHON_BINDINGS:BOOL=ON -DBUILD_PYTHON_PIP_PACKAGE:BOOL=OFF -DPYTHON_VERSION:STRING=3.12 -DPython_ROOT_DIR:PATH=$HOME/.pyenv/versions/3.12.1 \
    //             ../${source_folder} &&
    //         ninja package
    //       """)
    //     }
    //   }

    //   stage('ctests openstudio') {
    //     dir(mac_base_build) {
    //       try {
    //         sh '''#!/bin/bash
    //            ruby -v
    //            ctest -j 16 -C Release -T test --no-compress-output --output-on-failure
    //            '''
    //         currentBuild.result = 'SUCCESS'
    //       } catch (Exception err) {
    //         try {
    //           sh '''#!/bin/bash
    //               ruby -v
    //               ctest -j 16 --rerun-failed -C Release -T test --no-compress-output --output-on-failure
    //               '''
    //           currentBuild.result = 'SUCCESS'
    //         } catch (Exception err_2) {
    //           currentBuild.result = 'FAILURE'
    //         }
    //       } finally {
    //         try {
    //           sh("cp -r ${mac_base_build}/Testing ${mac_base_build}/Testing-m1-mac")
    //           archiveArtifacts artifacts: 'Testing-m1-mac/', fingerprint: true
    //           xunit(
    //             testTimeMargin: '9000',
    //             thresholdMode: 1,
    //             thresholds: [
    //               skipped(failureThreshold: '0'),
    //               failed(failureThreshold: '0')
    //             ],
    //           tools: [CTest(
    //             pattern: 'Testing-m1-mac/**/*.xml',
    //             deleteOutputFiles: true,
    //             failIfNotNew: false,
    //             skipNoTestFiles: true,
    //             stopProcessingIfError: false
    //           )])
    //         } catch (Exception e) {
    //           e.printStackTrace()
    //         }
    //       }
    //     }
    //   }

    //   stage('package and upload openstudio') {
    //     dir(mac_base_build) {
    //       files = findFiles(glob: '*.dmg')
    //       println "Found Files: ${files}"
    //       if (files.length == 1) {
    //         if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
    //           withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
    //             s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*.dmg',  acl:'PublicRead')
    //           }
    //           sh(script: "md5sum ${files[0].path}",
    //             label: 'Compute MD5 Hash for tar.gz')
    //         }
    //         else {
    //           println "File ${files[0].name} already in s3 bucket. Skipping upload"
    //         }
    //       }

    //       files = findFiles(glob: '*OpenStudio*arm64.tar.gz')
    //       println "Found Files: ${files}"
    //       if (files.length == 1) {
    //         if (!checkFileExistsS3(files[0].name, 'openstudio-ci-builds', env.BRANCH_NAME )) {
    //           withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'osci-s3']]) {
    //             s3Upload(bucket:'openstudio-ci-builds', path:"${env.BRANCH_NAME}/", workingDir: '.', includePathPattern:'*OpenStudio*arm64.tar.gz',  acl:'PublicRead')
    //           }
    //           sh(script: "md5sum ${files[0].path}",
    //             label: 'Compute MD5 Hash for tar.gz')
    //         }
    //         else {
    //           println "File ${files[0].name} already in s3 bucket. Skipping upload"
    //         }
    //       }
    //     }
    //   }//end package and upload openstudio
    //   }
    // }
    // next parallel
    ) // end all parrell nodes
}
