# GitHub Actions Full Build Plan

## Overview
- Transition the Jenkins full build pipeline to GitHub Actions while preserving stage parity and reporting.
- Trigger the workflow only on pushes to the `develop` branch (one run per merged PR) and optional release tags.
- Deliver multi-platform coverage matching the reference Jenkins job, including packaging, testing, artifact publishing, and downstream docker updates.

## Task Checklist

### 🧭 Preparation
- [x] Extract required behavior from `cbci_jenkins_libs/vars/openstudio_full_build_develop.groovy` for each platform.
- [x] Catalogue reusable pieces from `.github/workflows/incremental-build.yml` (concurrency, caching, test reporting).

### 🏗️ Workflow Design
- [x] Define workflow triggers, permissions, concurrency, and shared environment variables.
- [x] Design job matrix for platforms (CentOS 9, Ubuntu 22.04/24.04 x86, Ubuntu 22.04/24.04 ARM, macOS x64/ARM, Windows 2019).
- [x] Specify per-platform dependency bootstrap and build/test/package steps mirroring Jenkins.
- [x] Plan artifact uploads (installers, `_CPack_Packages`, `Testing` trees) and reporting (JUnit dashboard).
- [x] Outline optional AWS S3 publishing and docker workflow trigger steps with required secrets.

### 🧪 Implementation & Validation
- [x] Implement new workflow file in `.github/workflows/` following the design.
- [x] Update plan progress checkboxes as tasks complete.
- [ ] Dry run via branch push or `workflow_call` testing (document limitations if not executed).
- [ ] Verify artifact contents, logs, and downstream triggers (note any follow-up actions).

### 📦 Post-Implementation
- [ ] Document differences vs Jenkins and any residual manual steps.
- [ ] Coordinate disabling or archiving the Jenkins job once Actions pipeline is stable.
- [ ] Remove this temporary plan file after completion.

## Research Notes

### Jenkins Full Build Behavior (Develop)
- **Shared utilities**: `checkFileExistsS3` uses `osci-s3` credentials to skip duplicate uploads; `getOSversion` parses `Products/openstudio --version`; `failingTestExcludeRegexMac` currently unused but lists `BCLFixture.BCLComponent` for potential exclusion.
- **Ubuntu 22.04 (docker `kuangwenyi/openstudio-cmake-tools:jammy`)**: cleans mounted workspace when `ifclean`, clones branch, adds Conan remote `nrel-v2`, runs `conan install` for Ninja toolchain, configures CMake Release enabling tests, Python bindings, pip package, and packaging (`.deb` + `x86_64` tarball); executes `ctest` with rerun fallback, archives `Testing-Ubuntu-2204`, publishes CTest XML via Jenkins `xunit`; uploads `.deb` and tarball to S3 branch root; triggers downstream `docker-openstudio` workflow (`manual_update_develop`) via `gh workflow run` using computed installer URL/tag.
- **Ubuntu 24.04 (docker `kuangwenyi/openstudio-cmake-tools:noble`)**: same clean/clone pattern, prints Conan `global.conf`, builds Release with `.deb` and tarball outputs (pip package disabled), identical test rerun logic archiving `Testing-Ubuntu-2404`, uploads artifacts to S3 (no docker trigger).
- **Windows Server 2019**: wipes `D:/OSN`, clones repo, Conan install to Ninja, CMake config disables C# bindings but enables docs/tests/package, runs `ninja -j 16 package`; executes `ctest` with rerun fallback archiving `Testing-Windows`; performs code-signing via `code-signing-client` Node script (zip exe, sign, rerun for installer), runs `cpack`, uploads signed installer `.exe` plus `_CPack_Packages/win64/TGZ/*.tar.gz` to `openstudio-ci-builds/${BRANCH_NAME}/signed/`.
- **macOS x64 (10.15 node)**: validates `pyenv` 3.12.2, installs Python deps and Bundler 2.4.10 via RVM Ruby 3.2.2, cleans workspace, clones repo, runs Conan install, configures CMake with `DCMAKE_OSX_DEPLOYMENT_TARGET=10.15`, enables IFW + TGZ packaging with Python bindings; builds with Ninja, runs `ctest` under Ruby env with rerun fallback archiving `Testing-x64-mac`; uploads `.dmg` installer and `*x86_64.tar.gz` to S3 branch root.
- **Commented pipelines**: CentOS 7 (RPM + tarball) and macOS ARM64 variants retain detailed steps if additional platforms are restored.

### Incremental Workflow Reusables
- **Top-level controls**: `concurrency` keyed on workflow + PR/ref, scoped permissions block, job guard ensuring PR opt-in label—reusable for serialize-on-branch semantics.
- **Environment layout**: centralized `env` definition for thread scaling, compiler launcher, docker volume paths; containerized job referencing `nrel/openstudio-cmake-tools:jammy` with mounted Conan cache/workspace.
- **Tooling setup**: ccache install/config pipeline, Conan remote bootstrap with insecure toggles, profile detection guard, standard `conanbuild.sh` sourcing before CMake/Ninja.
- **Testing & reporting**: multi-pass `ctest` with rerun fallback writing JUnit XML, `test-summary/action`, custom dashboard generation, PR comment via `marocchino/sticky-pull-request-comment`, artifact uploads for dashboards and build outputs.
- **Git hygiene**: `actions/checkout@v4` full fetch, `git config --global --add safe.directory`, explicit bot identity, PR ref fetch/checkout for container volumes.

## Workflow Design Draft

### Workflow Skeleton
- **Triggers**: `push` to `develop` branch; `push` tags matching `v*` for releases; manual `workflow_dispatch` with branch/tag inputs; optional `workflow_call` interface for dry-run testing.
- **Concurrency**: `group: full-build-${{ github.ref }}` with `cancel-in-progress: false` so merged commits queue sequentially while preserving release runs.
- **Permissions**: minimum required—`contents: read`, `packages: write` (for future GHCR usage), `id-token: write` (for OIDC to AWS), `actions: read`, `checks: write`, `pull-requests: write` (for manual triggers or reusable use).
- **Global env**: `BUILD_TYPE=Release`, `PY_VERSION=3.12.2`, `MAX_BUILD_THREADS: ${{ matrix.max_jobs || 8 }}`, `CONAN_HOME: ${{ github.workspace }}/.conan2`, `CTEST_PARALLEL_LEVEL=${{ matrix.ctest_threads || matrix.max_jobs }}`, `OPENSTUDIO_SOURCE=OpenStudio`, `OPENSTUDIO_BUILD=OS-build-release-v2`, `AWS_S3_BUCKET=openstudio-ci-builds`.
- **Shared steps**: checkout with full history, configure git safe-directory, Conan remotes (`nrel-v2`), `conan install` with Ninja generator, CMake configure/build using `conanbuild.sh`, standardized test harness with rerun fallback.

### Job Matrix
- **Strategy**: matrix on `platform` with include metadata: runner label/OS, container image (if any), artifact patterns, optional features (pip pkg, code signing, docker trigger).
- **Entries**:
	- `centos-9` → `runs-on: [self-hosted, linux, x86_64, centos9]`, container `kuangwenyi/openstudio-cmake-tools:centos9`, artifacts `.rpm` + tarball, S3 upload path root.
	- `ubuntu-2204-x64` → `runs-on: linux-openstudio-2` (existing self-hosted) with container `nrel/openstudio-cmake-tools:jammy`, enable pip package + docker trigger.
	- `ubuntu-2404-x64` → similar runner container `nrel/openstudio-cmake-tools:noble`, pip package off.
	- `ubuntu-2204-arm64`/`ubuntu-2404-arm64` → `runs-on: [self-hosted, linux, arm64]`, native runner without container (use pre-provisioned toolchain) or container if available; adjust `max_jobs` and dependency install via apt.
	- `macos-x64` → `runs-on: [self-hosted, macos, x64, 10_15]`, install pyenv python, bundler; produce `.dmg` + tarball.
	- `macos-arm64` → `runs-on: [self-hosted, macos, arm64]`, similar steps with tarball `arm64` patterns.
	- `windows-2019` → `runs-on: [self-hosted, Windows, x64, 2019]`, use `shell: powershell` by default, pipeline replicating Jenkins.
- **Matrix controls**: `fail-fast: false`, `max-parallel: 2` (tunable). Use `include` with per-platform overrides: `pip_package`, `code_sign`, `docker_trigger`, `s3_prefix`, `toolchain_generator`.

### Platform Step Outline
- **Common**: checkout, configure git, ensure `conan profile detect` (if missing), install dependencies (pyenv or chocolatey), run `conan install` pointing build dir, source `conanbuild` script, run `cmake` with toggles derived from matrix flags, `ninja -j ${{ env.MAX_BUILD_THREADS }} package`.
- **Linux variants**: pre-stage apt/yum installs for prerequisites (`python3-pip`, `ruby`, `awscli`), ensure `ruby -v` prior to tests, use `ctest` with `--no-compress-output` and `--output-junit Testing/${{ matrix.platform }}/run1.xml`. Capture `Testing` directory copy renamed with platform suffix, upload as artifact.
- **Windows**: use PowerShell to call `conan install`, `cmake`, `ninja`, run `ctest` via `cmd`, call signing client if `code_sign` true (credentials via secret `CODE_SIGNING_TOKEN`), run `cpack`, stage signed output under `signed/`.
- **macOS**: install bundler + Ruby env, run tests inside RVM, ensure `hdiutil` available for DMG packaging. Provide `codesign` placeholder (future) but not in Jenkins parity.
- **ARM runners**: cross-check dependencies (pyenv path or system Python), adjust CMake flags for `-DCMAKE_OSX_ARCHITECTURES` or `CMAKE_SYSTEM_PROCESSOR` as needed.

### Reporting & Artifact Plan
- **Testing**: run `ctest` up to three attempts writing `Testing/run{1,2,3}/results.xml`; aggregate using Python script (adapted from incremental workflow) to produce Markdown dashboard per job; upload `Testing/` tree artifact for Jenkins parity.
- **Artifacts**: upload installers (`*.deb`, `*.rpm`, `*.dmg`, `*.exe`), tarballs (`*_x86_64.tar.gz`, `*_arm64.tar.gz`), `_CPack_Packages/**` directories, `Testing` tree, and test dashboard. Use `if: always()` to ensure collection even on failure.
- **Publishing**: guard S3 uploads behind `if: ${{ inputs.publish_to_s3 || github.ref == 'refs/heads/develop' }}` and `secrets.AWS_S3_UPLOAD_ROLE_ARN`; use AWS CLI with OIDC to assume role; compute MD5 after upload; skip upload if object exists (use `aws s3api head-object` equivalent of Jenkins `checkFileExistsS3`).
- **Notifications**: on develop branch success, trigger `docker-openstudio` workflow via `gh workflow run` using `github-token` secret; optionally emit Slack webhook (future enhancement noted).

### Optional External Steps
- **AWS S3 Publishing**: assume role using `aws-actions/configure-aws-credentials@v4`, then run upload script reading matrix artifact lists, performing head/object check, uploading with `--acl public-read`, writing MD5 to logs.
- **Docker Workflow Trigger**: for `ubuntu-2204-x64` job success, run CLI step using `GITHUB_TOKEN` with repo scope to trigger `manual_update_develop`, templated with artifact URL.
- **Release Tag Handling**: when triggered by tag, publish to S3 under `releases/${tag}` prefix and skip docker update; future addition to notify downstream packaging.

## Implementation Notes

- Added workflow at `.github/workflows/full-build.yml` implementing matrix jobs for Linux (x64/ARM), macOS (x64/ARM), and Windows with Conan-driven builds, repeated CTest retry logic, artifact uploads, optional S3 publishing via OIDC, and docker trigger stage for Ubuntu 22.04 x64; reusable Markdown dashboard script lives at `.github/workflows/scripts/render_dashboard.py`.
- Docker trigger assumes `gh` CLI is present on the Ubuntu runner/container; if absent we must install it or swap for REST API call. S3 publishing requires `AWS_S3_UPLOAD_ROLE_ARN` and `AWS_REGION` secrets; lack of role will skip uploads gracefully but should be documented for ops.
- macOS steps rely on pre-provisioned `pyenv` 3.12.2 and RVM Ruby 3.2.2, mirroring Jenkins. Confirm these versions exist on each self-hosted runner to avoid setup failures; adjust `PY_VERSION` if mismatch.
- Windows job copies repo to `D:/OSN` to mirror Jenkins paths and invokes local code-signing client; ensure `CODE_SIGNING_TOKEN` secret is configured and directories exist before first run.
- Outstanding validation: full workflow has not been exercised yet, so artifact packaging formats, S3 uploads, and docker trigger results remain to be verified in a dry run.
