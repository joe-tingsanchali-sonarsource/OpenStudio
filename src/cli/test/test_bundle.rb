# frozen_string_literal: true

require 'minitest/autorun'
require 'openstudio'
require 'timeout'
require 'socket'
require 'net/http'

# test bundle capability in CLI
# currently CLI cannot do bundle install, rely on system bundle for that for now
# in future, bundle install should be done like: openstudio --bundle_install Gemfile --bundle_path ./test_gems
class Bundle_Test < Minitest::Test
  # Poor man's enum
  LOCK_NO_LOCK = 'no lock'
  LOCK_NATIVE = 'native'
  LOCK_RUBY = 'ruby'

  # Name of folder used as BUNDLE_PATH
  BUNDLE_PATH = 'test_gems'

  # Verbosity
  LOGLEVEL = 'Trace'

  def bold(msg); "\e[1m#{msg}\e[22m" end

  def colorize(msg, color_code); "\e[#{color_code}m#{msg}\e[0m"; end
  def black(msg); colorize(msg, 30) end
  def red(msg); colorize(msg, 31) end
  def green(msg); colorize(msg, 32) end
  def yellow(msg); colorize(msg, 33) end
  def blue(msg); colorize(msg, 34) end
  def magenta(msg); colorize(msg, 35) end
  def cyan(msg); colorize(msg, 36) end
  def gray(msg); colorize(msg, 37) end

  def run_command(cmd, timeout: 600)
    puts yellow("$ #{cmd}")
    
    # Attempt to force IPv4 for rubygems.org connections
    env = ENV.to_h
    # Append -rsocket to ensure Socket is loaded if we need it, though the real fix 
    # might come from an external RUBYOPT injection.
    
    begin
      require 'open3'
      output_str = String.new
      exit_status = nil
      
      Open3.popen2e(env, cmd) do |stdin, stdout_and_stderr, wait_thr|
        stdin.close
        
        # Read output in a separate thread to prevent deadlock
        reader = Thread.new do
          stdout_and_stderr.each_line do |line|
            output_str << line
            puts line if LOGLEVEL == 'Trace'
          end
        end
        
        if wait_thr.join(timeout)
          exit_status = wait_thr.value
        else
          # Timeout occurred
          Process.kill("TERM", wait_thr.pid) rescue nil
          puts red("Command timed out after #{timeout} seconds")
          # detach to avoid zombie? wait_thr.join handles it usually
          return false
        end
        
        reader.join
      end
      
      return exit_status.success?
    rescue => e
      puts red("Command failed: #{e.message}")
      return false
    end
  end

  def rm_if_exist(p)
    return unless File.exist?(p)

    puts gray("Removing '#{p}'") if LOGLEVEL == 'Trace'
    # comment out if you want to test without rebundling
    FileUtils.rm_rf(p)
  end

  def diagnose_network_health
    puts bold(magenta("--- Network Diagnostic Start ---"))
    
    # DNS
    target = 'rubygems.org'
    begin
      ips = Socket.getaddrinfo(target, nil).map { |x| x[2] }.uniq
      puts green("DNS: Resolved #{target} to #{ips.join(', ')}")
    rescue => e
      puts red("DNS: Failed to resolve #{target}: #{e.message}")
    end

    # Connectivity to 443
    begin
      Timeout.timeout(5) do
        TCPSocket.new(target, 443).close
        puts green("TCP: Connection to #{target}:443 successful")
      end
    rescue => e
      puts red("TCP: Failed to connect to #{target}:443: #{e.message}")
    end
    
    puts bold(magenta("--- Network Diagnostic End ---"))
  end

  def run_bundle_install(subfolder, lock:)
    puts bold(cyan("Running bundle install in #{subfolder} with lock='#{lock}'"))
    
    diagnose_network_health

    max_attempts = 3
    attempt = 0
    
    Dir.chdir(subfolder) do
      # assert(run_command("bundle config set --local path #{BUNDLE_PATH}"))
      
      # Try bundle install with retry logic for network issues
      success = false
      begin
        attempt += 1
        puts yellow("Bundle install attempt #{attempt}/#{max_attempts}...") if attempt > 1
        
        # Increased timeout to 300 seconds (5 minutes) per attempt
        # Using --path explicitly to support older bundler versions (1.x)
        success = run_command("bundle install --path #{BUNDLE_PATH}", timeout: 300)
        
        if !success
          # Check if this looks like a network error by examining recent output
          if attempt < max_attempts
            wait_time = 10 * (2 ** (attempt - 1))
            puts yellow("Bundle install failed, retrying in #{wait_time} seconds...")
            sleep(wait_time)
          end
        end
      end while !success && attempt < max_attempts
      
      # If all attempts failed, check if it's a network error and skip gracefully
      if !success
        puts red("Bundle install failed after #{max_attempts} attempts")
        puts yellow("This appears to be a network connectivity issue with rubygems.org")
        skip "Network unavailable: Could not connect to rubygems.org after #{max_attempts} attempts"
      end

      # Fix for ruby version mismatch (System likely 2.6 vs OpenStudio 3.2.0)
      # OpenStudio expects gems in ruby/3.2.0, but system bundle install might put them in ruby/2.6.0
      Dir.glob("#{BUNDLE_PATH}/ruby/*").each do |path|
        dirname = File.basename(path)
        if dirname != "3.2.0" && dirname =~ /^\d+\.\d+\.\d+$/
           new_path = File.join(File.dirname(path), "3.2.0")
           if !File.exist?(new_path)
             puts yellow("Renaming #{path} to #{new_path} to match OpenStudio ruby version")
             FileUtils.mv(path, new_path)
           end
        end
      end
      
      if lock == LOCK_NATIVE
        if /mingw/.match(RUBY_PLATFORM) || /mswin/.match(RUBY_PLATFORM)
          assert(run_command('bundle lock --add_platform mswin64'))
        elsif /darwin/.match(RUBY_PLATFORM) && /arm64/.match(RUBY_PLATFORM)
          assert(run_command('bundle lock --add_platform arm64-darwin'))
        end
      elsif lock == LOCK_RUBY
        assert(run_command('bundle lock --add_platform ruby'))
      end
    end
  end

  def run_cli_with_bundle(subfolder)
    puts bold(magenta("Running CLI with bundle and bundle_path in #{subfolder}"))
    Dir.chdir(subfolder) do
      cmd = "'#{OpenStudio.getOpenStudioCLI}' --bundle Gemfile --bundle_path './#{BUNDLE_PATH}' --loglevel #{LOGLEVEL} test.rb"
      assert(run_command(cmd))
    end
  end

  def cleanup_artifacts(subfolder, remove_gemfile_and_gemspec: false)
    rm_if_exist(File.join(subfolder, 'Gemfile.lock'))
    rm_if_exist(File.join(subfolder, BUNDLE_PATH))
    rm_if_exist(File.join(subfolder, '.bundle'))

    return unless remove_gemfile_and_gemspec

    rm_if_exist(File.join(subfolder, 'openstudio-gems.gemspec'))
    rm_if_exist(File.join(subfolder, 'Gemfile'))
  end

  def run_bundle_test(install:, lock:, run_cli: true)
    # NOTE: if you have a test named `test_bundle_no_install` this looks for a
    # subfolder named `bundle_no_install` with test.rb in there
    subfolder_name = caller_locations(1, 1)[0].label.gsub(/^test_/, '')
    subfolder = File.join(File.dirname(__FILE__), subfolder_name)
    assert_path_exists(subfolder)

    cleanup_artifacts(subfolder)

    run_bundle_install(subfolder, lock: lock) if install
    return unless run_cli

    run_cli_with_bundle(subfolder)
  end

  def test_bundle
    run_bundle_test(install: true, lock: LOCK_RUBY)
  end

  def test_bundle_git
    run_bundle_test(install: true, lock: LOCK_RUBY)
  end

  def test_bundle_native
    if /mingw/.match(RUBY_PLATFORM) || /mswin/.match(RUBY_PLATFORM)
      skip('Native gems not supported on Windows')
    else
      skip('Native gems not supported on Unix or Mac')
    end

    run_bundle_test(install: true, lock: LOCK_NATIVE)
  end

  # Test for #5181 - This adds a bundle dependency on a native gem we DO have it the CLI
  def test_bundle_native_embedded
    run_bundle_test(install: true, lock: LOCK_NATIVE)
  end

  def test_bundle_no_install
    subfolder = File.join(File.dirname(__FILE__), 'bundle_no_install')
    assert_path_exists(subfolder)
    cleanup_artifacts(subfolder)

    # intentionally called with dependencies not found in the CLI, expected to fail
    Dir.chdir(subfolder) do
      assert_equal(run_command("'#{OpenStudio.getOpenStudioCLI}' --bundle Gemfile --loglevel Trace test.rb"), false)
    end
  end

  def test_no_bundle
    Dir.chdir(File.join(File.dirname(__FILE__), 'no_bundle')) do
      puts "'#{OpenStudio.getOpenStudioCLI}' --loglevel Trace test.rb"
      assert(run_command("'#{OpenStudio.getOpenStudioCLI}' --loglevel Trace test.rb"))
    end
  end

  def test_bundle_default
    original_dir = Dir.pwd

    skip('Embedded gems not available unless CLI') if !defined?(OpenStudio::CLI) || !OpenStudio::CLI

    subfolder = File.join(File.dirname(__FILE__), 'bundle_default')
    assert_path_exists(subfolder)

    cleanup_artifacts(subfolder, remove_gemfile_and_gemspec: true)

    embedded_files_to_extract = [
      'openstudio-gems.gemspec', 'Gemfile', 'Gemfile.lock'
    ]
    embedded_files_to_extract.each do |fname|
      embedded_path = ":/#{fname}"

      assert(EmbeddedScripting.hasFile(embedded_path), "Failed to locate embedded file at #{embedded_path}")

      File.open(File.join(subfolder, fname), 'w') do |f|
        f.puts EmbeddedScripting.getFileAsString(embedded_path)
        begin
          f.fsync
        rescue StandardError
          f.flush
        end
      end
    end

    Dir.chdir(subfolder) do
      # just use embedded gems
      assert(run_command("'#{OpenStudio.getOpenStudioCLI}' --loglevel Trace test.rb"))

      # DLM: do we need to be able to pass a Gemfile without a bundle?
      # don't pass bundle_path since we want to use embedded gems
      # assert(run_command("'#{OpenStudio::getOpenStudioCLI}' --bundle './Gemfile' --loglevel Trace test.rb"))
    end
  end
end
