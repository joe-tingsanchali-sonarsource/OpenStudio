# Next Steps: GitHub Actions Migration

## Immediate Actions (This Week)

### 1. Set Up Code Signing for AWS Signing Service ✅

**Your code signing setup identified:**
- ✅ Service: **AWS Code Signing** (Amazon Signer)
- ✅ Location: `C:/code-signing-client/code-signing.js`
- ✅ Credentials: Uses `ACCESS_KEY` and `SECRET_KEY` from `.env` file
- ✅ Storage: `.github/signing-client/` (to be checked into repository)

**Required Actions:**

1. **Copy the signing client from Jenkins to your repository:**
   ```powershell
   # On Jenkins machine
   cd C:\code-signing-client
   
   # Remove node_modules and sensitive files
   Remove-Item node_modules -Recurse -Force -ErrorAction SilentlyContinue
   Remove-Item .env -Force -ErrorAction SilentlyContinue
   
   # List what you're copying
   Get-ChildItem
   
   # Zip it up
   Compress-Archive -Path *.js,package.json,package-lock.json -DestinationPath code-signing-client.zip
   ```

2. **Add to your OpenStudio repository:**
   ```bash
   # In your local repo
   mkdir -p .github/signing-client
   # Copy the zip file and extract contents to .github/signing-client/
   # Should include: code-signing.js, package.json, package-lock.json
   
   git add .github/signing-client/
   git commit -m "Add AWS code signing client"
   ```

3. **Get credentials from Jenkins `.env` file:**
   - Locate the `ACCESS_KEY` value (AWS Access Key ID for signing)
   - Locate the `SECRET_KEY` value (AWS Secret Access Key for signing)

### 2. Set Up GitHub Secrets

Go to your repository: `Settings → Secrets and variables → Actions → New repository secret`

Add these secrets:
- [ ] `AWS_ACCESS_KEY_ID` - For S3 publishing (you have this)
- [ ] `AWS_SECRET_ACCESS_KEY` - For S3 publishing (you have this)
- [ ] `AWS_SIGNING_ACCESS_KEY` - From Jenkins `.env` file ACCESS_KEY (for code signing)
- [ ] `AWS_SIGNING_SECRET_KEY` - From Jenkins `.env` file SECRET_KEY (for code signing)
- [ ] `AWS_REGION` - Optional, use `us-west-2` if not specified

### 3. Verify Docker Images Are Public

```bash
# Test pulling the images
docker pull nrel/openstudio-cmake-tools:jammy
docker pull nrel/openstudio-cmake-tools:noble

# If these fail, check Docker Hub or make them public
```

### 4. Decision: Which Platforms to Build?

**Review the new workflow file:** `.github/workflows/full-build-github-hosted.yml`

**Currently includes:**
- ✅ Ubuntu 22.04 x64 (GitHub runner: ubuntu-22.04)
- ✅ Ubuntu 24.04 x64 (GitHub runner: ubuntu-24.04)
- ✅ macOS Intel x64 (GitHub runner: macos-13)
- ✅ macOS Apple Silicon ARM64 (GitHub runner: macos-14)
- ✅ Windows x64 (GitHub runner: windows-2022)

**Currently EXCLUDED (not available on free GitHub runners):**
- ❌ CentOS Stream 9 x64 (no CentOS runners on GitHub)
- ❌ Linux ARM64 Ubuntu 22.04 (no free ARM64 Linux runners)
- ❌ Linux ARM64 Ubuntu 24.04 (no free ARM64 Linux runners)

**Your decision:**
- [ ] Accept this and skip CentOS/ARM64 Linux builds
- [ ] Keep CentOS/ARM64 on self-hosted runners (hybrid approach)
- [ ] Pay for GitHub larger runners to get ARM64 Linux
- [ ] Build CentOS packages in container on Ubuntu (requires workflow changes)

## Week 1-2: Testing Phase

### Test 1: Ubuntu Only (Low Risk Test)

1. **Create test branch:**
   ```bash
   git checkout -b github-actions-test
   ```

2. **Edit the workflow to test ONE platform first:**
   ```bash
   # Edit .github/workflows/full-build-github-hosted.yml
   # Comment out all matrix entries except Ubuntu 22.04
   ```

3. **Push and trigger manually:**
   ```bash
   git add .github/workflows/full-build-github-hosted.yml
   git commit -m "Test: Add GitHub Actions workflow (Ubuntu 22.04 only)"
   git push -u origin github-actions-test
   
   # Trigger via GitHub UI:
   # Actions → Full Build (GitHub-Hosted Runners) → Run workflow
   # Select branch: github-actions-test
   # Set publish_to_s3: false
   ```

4. **Monitor the build:**
   - Watch for disk space warnings
   - Check build time (expect 2-3x slower than Jenkins with 16 cores)
   - Verify artifacts are created

5. **Review results:**
   - [ ] Build completed (even if tests failed)
   - [ ] Artifacts uploaded successfully
   - [ ] Downloaded artifacts and verified contents
   - [ ] Documented any issues

### Test 2: Add macOS (If Test 1 Passed)

1. **Uncomment macOS builds in workflow**
2. **Push and trigger again**
3. **Verify macOS-specific steps work:**
   - [ ] pyenv installs Python 3.12.2
   - [ ] Conan installs via pip
   - [ ] Build completes
   - [ ] Packages created (.dmg, .tar.gz)

### Test 3: Add Windows Without Signing (If Test 2 Passed)

1. **Uncomment Windows build**
2. **Comment out the "Code sign installer" step temporarily**
3. **Push and trigger**
4. **Verify:**
   - [ ] Windows paths work (no more D:/OSN/ issues)
   - [ ] Build completes
   - [ ] Unsigned .exe installer created
   - [ ] Artifacts upload

### Test 4: Enable Windows Code Signing (AWS Signer)

**Prerequisites:**
- [ ] Signing client copied to `.github/signing-client/` in repository
- [ ] `AWS_SIGNING_ACCESS_KEY` secret added to GitHub
- [ ] `AWS_SIGNING_SECRET_KEY` secret added to GitHub
- [ ] Workflow updated with signing steps (see below)

**The workflow already includes the necessary steps:**

The `full-build-github-hosted.yml` file includes these steps:

1. **Setup Node.js** - Already included in workflow
2. **Install Signing Client Dependencies** - Runs `npm install` in `.github/signing-client`
3. **Create .env file** - Creates `.env` with `ACCESS_KEY` and `SECRET_KEY` from secrets
4. **Code sign installer** - Runs your `code-signing.js` script

**To enable signing:**

1. **Verify signing client is in repository:**
   ```bash
   # Check that these files exist:
   ls -la .github/signing-client/
   # Should show: code-signing.js, package.json, package-lock.json
   ```

2. **Verify GitHub secrets are configured:**
   - Go to: Settings → Secrets and variables → Actions
   - Confirm `AWS_SIGNING_ACCESS_KEY` exists
   - Confirm `AWS_SIGNING_SECRET_KEY` exists

3. **Test the signing:**
   - Push the workflow with signing steps enabled
   - Trigger a Windows build
   - Monitor the "Code sign installer" step in the logs
   - Verify signed files appear in the `signed/` directory
   - Download and verify signature:
     ```powershell
     Get-AuthenticodeSignature OpenStudio-*.exe | Format-List
     ```

**Troubleshooting:**

If signing fails:
- Check that `.env` file is created correctly (step should show no errors)
- Verify AWS credentials are valid and have signing permissions
- Check the signing script output for error messages
- Increase timeout if needed (currently 4800000ms = 80 minutes)

### Test 5: S3 Publishing Test

1. **Verify AWS credentials in GitHub secrets**
2. **Trigger build with manual override:**
   - Set `publish_to_s3: true`
3. **Check S3 bucket:**
   ```bash
   aws s3 ls s3://openstudio-ci-builds/github-actions-test/
   ```
4. **Verify files uploaded correctly**

## Week 3: Production Deployment

### Option A: Gradual Migration (RECOMMENDED)

1. **Rename current workflow:**
   ```bash
   git checkout develop
   git mv .github/workflows/full-build.yml .github/workflows/full-build-jenkins.yml
   ```

2. **Deploy GitHub-hosted workflow:**
   ```bash
   git mv .github/workflows/full-build-github-hosted.yml .github/workflows/full-build.yml
   git commit -m "Deploy GitHub Actions workflow (alongside Jenkins)"
   git push
   ```

3. **Run both in parallel for 2 weeks:**
   - Compare results
   - Identify any discrepancies
   - Build confidence

4. **Eventually disable Jenkins workflow:**
   ```bash
   # Delete or rename the Jenkins workflow
   git rm .github/workflows/full-build-jenkins.yml
   ```

### Option B: Clean Switch (Higher Risk)

1. **Back up original:**
   ```bash
   git checkout develop
   cp .github/workflows/full-build.yml .github/workflows/full-build-jenkins-backup.yml
   git add .github/workflows/full-build-jenkins-backup.yml
   ```

2. **Replace with GitHub-hosted version:**
   ```bash
   cp .github/workflows/full-build-github-hosted.yml .github/workflows/full-build.yml
   git add .github/workflows/full-build.yml
   git commit -m "Switch to GitHub-hosted runners"
   git push
   ```

3. **Monitor first build very closely**
4. **Keep Jenkins backup available for quick rollback**

## Reference Documentation Created

1. **CODE_SIGNING_SETUP.md** - Detailed code signing configuration options
2. **MIGRATION_GUIDE.md** - Complete migration guide with all technical details
3. **DEPLOYMENT_CHECKLIST.md** - Step-by-step checklist for deployment
4. **full-build-github-hosted.yml** - Ready-to-use workflow file

## Key Differences from Jenkins

| Aspect | Jenkins (Self-Hosted) | GitHub Actions |
|--------|----------------------|----------------|
| **Runners** | Custom hardware, 16 cores | Standard VMs, 4 cores |
| **Build Time** | Baseline | 2-3x slower expected |
| **Disk Space** | Large | ~14GB limit |
| **Persistence** | Cached between builds | Clean slate each run |
| **Cost** | Infrastructure maintenance | Free (public repo) |
| **Platforms** | All platforms | Ubuntu, macOS, Windows only |

## Common Issues & Solutions

### "Disk space full"
- Clean Conan cache between steps
- Remove build intermediates before packaging
- Split build and test into separate jobs

### "Build timeout (6 hours)"
- Reduce parallelism
- Split build/test into separate jobs
- Use paid larger runners (more cores)

### "Code signing fails"
- Verify CODE_SIGNING_TOKEN secret is set
- Check signing client is properly installed
- Increase timeout in signing script
- Check network access to signing service

### "Conan dependencies fail to download"
- Check if NREL Conan repo requires authentication from GitHub
- May need to add Conan credentials as secrets

## Success Metrics

After migration is complete, track:
- [ ] Build success rate comparable to Jenkins
- [ ] All platforms building successfully
- [ ] Artifacts uploading to S3 correctly
- [ ] Code signing working
- [ ] Team comfortable with new system
- [ ] Build times acceptable (< 6 hours per platform)

## Timeline Estimate

- **Week 1:** Investigation + Initial testing (Steps 1-4 above)
- **Week 2:** Full platform testing + code signing
- **Week 3:** Production deployment (gradual or clean switch)
- **Week 4:** Monitoring and optimization

## Need Help?

- **GitHub Actions Docs:** https://docs.github.com/en/actions
- **Workflow Syntax:** https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions
- **Runner Specs:** https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners

## Questions to Answer

Before proceeding, determine:

1. **Code Signing:** What service are you using? _________________
2. **Platforms:** Skip ARM64/CentOS or keep as self-hosted? _________________
3. **Deployment:** Gradual or clean switch? _________________
4. **Timeline:** When do you want to complete migration? _________________
5. **Rollback:** Keep Jenkins available how long? _________________

---

---

## Code Signing Summary (AWS Signer)

**Service Identified:** AWS Code Signing (Amazon Signer)

**Migration Steps:**
1. ✅ Copy `code-signing.js` and dependencies to `.github/signing-client/`
2. ✅ Add `AWS_SIGNING_ACCESS_KEY` and `AWS_SIGNING_SECRET_KEY` to GitHub secrets
3. ✅ Workflow creates `.env` file dynamically from secrets
4. ✅ Workflow runs signing script from repository location

**No longer needed:**
- ❌ `C:/code-signing-client/` path (script runs from repo)
- ❌ `CODE_SIGNING_TOKEN` (uses AWS access/secret keys instead)
- ❌ `D:/OSN/` paths (uses `${{ github.workspace }}`)

**Start with:** Copying signing client to repository and configuring AWS secrets!