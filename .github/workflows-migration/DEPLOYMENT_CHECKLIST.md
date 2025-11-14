# Pre-Deployment Checklist for GitHub-Hosted Runners

## Phase 1: Prerequisites ✓

### GitHub Secrets Configuration
- [ ] **AWS_ACCESS_KEY_ID** - Added to repository secrets
- [ ] **AWS_SECRET_ACCESS_KEY** - Added to repository secrets  
- [ ] **AWS_REGION** - Added (or will use default: us-west-2)
- [ ] **CODE_SIGNING_TOKEN** - Research and configure (see below)
- [ ] **GH_DOCKER_TRIGGER_TOKEN** - Optional, will use GITHUB_TOKEN if not set

### Code Signing Investigation (CRITICAL - Windows builds will fail without this)
- [ ] Locate `C:/code-signing-client/` on Jenkins Windows runner
- [ ] Document what signing service is being used:
  - [ ] Check `code-signing.js` for API endpoints/URLs
  - [ ] Check for README, package.json, or config files
  - [ ] Identify service: DigiCert ONE / SSL.com / Azure / SignPath / Other: ___________
- [ ] Obtain current `CODE_SIGNING_TOKEN` value from Jenkins credentials
- [ ] Decide on approach:
  - [ ] Option A: Copy signing client to repository (in `.github/code-signing-client/`)
  - [ ] Option B: Host signing client in secure location and download during workflow
  - [ ] Option C: Switch to different signing service (SignPath.io, Azure, etc.)
  - [ ] Option D: Skip signing initially for testing, add later
- [ ] Test signing client setup on a test branch before production

### Docker Images Verification
- [ ] Verify `nrel/openstudio-cmake-tools:jammy` is public on Docker Hub
- [ ] Verify `nrel/openstudio-cmake-tools:noble` is public on Docker Hub
- [ ] Test pulling images: `docker pull nrel/openstudio-cmake-tools:jammy`

### AWS S3 Access
- [ ] Verify S3 bucket `openstudio-ci-builds` exists
- [ ] Test AWS credentials have proper permissions:
  - [ ] ListBucket
  - [ ] PutObject
  - [ ] PutObjectAcl
  - [ ] GetObject
- [ ] Test from GitHub Actions (use a test workflow)

### Network Access from GitHub
- [ ] Verify Conan repositories are accessible from GitHub IPs:
  - [ ] `https://center.conan.io`
  - [ ] `https://conan.openstudio.net/artifactory/api/conan/conan-v2`
- [ ] If NREL Conan repo is private, configure authentication

## Phase 2: Build Platform Decisions ✓

### Platforms to Include
- [ ] **Ubuntu 22.04 x64** - ✅ Keep (uses ubuntu-22.04 runner)
- [ ] **Ubuntu 24.04 x64** - ✅ Keep (uses ubuntu-24.04 runner)  
- [ ] **macOS Intel (x64)** - ✅ Keep (uses macos-13 runner)
- [ ] **macOS Apple Silicon (ARM64)** - ✅ Keep (uses macos-14 runner)
- [ ] **Windows x64** - ✅ Keep (uses windows-2022 runner)

### Platforms to Skip or Modify
- [ ] **CentOS Stream 9** - Decision:
  - [ ] Skip entirely _(recommended for initial migration)_
  - [ ] Build in container on Ubuntu _(requires significant changes)_
  - [ ] Keep as self-hosted only _(hybrid approach)_
  
- [ ] **Linux ARM64** (Ubuntu 22.04/24.04) - Decision:
  - [ ] Skip entirely _(recommended - ARM64 packages less critical)_
  - [ ] Use GitHub larger runners (paid, 4-core ARM64) _(cost: check pricing)_
  - [ ] Keep as self-hosted only _(hybrid approach)_

## Phase 3: Resource Planning ✓

### Disk Space Analysis
GitHub runners provide ~14GB usable disk space. Estimate your needs:
- [ ] Measure current build sizes on Jenkins:
  - [ ] Source code size: _______ MB
  - [ ] Conan dependencies cache: _______ MB
  - [ ] Build output size: _______ MB
  - [ ] Test artifacts size: _______ MB
  - [ ] **Total per platform:** _______ MB
- [ ] If total > 12GB per platform:
  - [ ] Clean Conan cache after build
  - [ ] Remove intermediate build files before packaging
  - [ ] Split build and test into separate jobs
  - [ ] Consider paid larger runners

### Build Time Estimation
GitHub runners have a 6-hour timeout per job:
- [ ] Measure current build times on Jenkins:
  - [ ] Ubuntu build + test: _______ minutes
  - [ ] macOS build + test: _______ minutes  
  - [ ] Windows build + test: _______ minutes
- [ ] Note: GitHub runners may be slower than dedicated self-hosted
  - Ubuntu: 4 cores vs 16 cores (expect 2-3x longer)
  - macOS: 3-4 cores vs 8 cores (expect 2x longer)
  - Windows: 4 cores vs 16 cores (expect 2-3x longer)
- [ ] If any build > 5 hours, consider:
  - [ ] Reduce test parallelism
  - [ ] Split into separate build/test jobs
  - [ ] Use paid larger runners

### Cost Analysis (for Public Repo)
- [ ] **Confirm repository is public** → Unlimited free minutes ✅
- [ ] If private:
  - Linux: 1x multiplier
  - Windows: 2x multiplier  
  - macOS: 10x multiplier
  - [ ] Calculate monthly minute usage
  - [ ] Verify within plan limits

## Phase 4: Workflow File Preparation ✓

### File Changes
- [ ] Review `full-build-github-hosted.yml` created in `.github/workflows/`
- [ ] Review differences from original `full-build.yml`
- [ ] Update runner labels as documented in MIGRATION_GUIDE.md
- [ ] Update Windows paths from `D:/OSN/` to `${{ github.workspace }}/`
- [ ] Adjust `max_jobs` for GitHub runner CPU counts:
  - [ ] Linux: 4 cores (was 16)
  - [ ] macOS: 4 cores (was 8)
  - [ ] Windows: 4 cores (was 16)
- [ ] Configure code signing steps (placeholder included, needs customization)
- [ ] Remove or comment out platforms you're skipping

### Testing Branch Setup
- [ ] Create testing branch: `git checkout -b github-actions-test`
- [ ] Initially disable S3 publishing (set to manual trigger only)
- [ ] Initially disable Docker workflow trigger
- [ ] Add workflow file to test branch
- [ ] Commit and push

## Phase 5: Initial Testing ✓

### Test Run 1: Single Platform (Ubuntu 22.04)
- [ ] Modify workflow to only run Ubuntu 22.04 job
- [ ] Trigger via workflow_dispatch with `publish_to_s3: false`
- [ ] Monitor build progress in Actions tab
- [ ] Check for issues:
  - [ ] Checkout succeeds
  - [ ] Conan install succeeds
  - [ ] CMake configure succeeds
  - [ ] Build completes (watch for disk space)
  - [ ] Tests run (may have failures - that's ok for now)
  - [ ] Artifacts upload successfully
- [ ] Download and verify artifacts
- [ ] Review build logs for warnings/errors

### Test Run 2: Add macOS
- [ ] Enable macOS builds in workflow
- [ ] Trigger workflow_dispatch
- [ ] Verify both platforms build
- [ ] Check macOS-specific issues:
  - [ ] pyenv installs Python correctly
  - [ ] Bundler setup works
  - [ ] Conan installation via pip works
  - [ ] Signing (macOS may need code signing too)

### Test Run 3: Add Windows (Without Code Signing)
- [ ] Enable Windows build
- [ ] Comment out or skip code signing step initially
- [ ] Trigger workflow_dispatch
- [ ] Verify Windows build:
  - [ ] Workspace paths work correctly
  - [ ] Conan install succeeds on Windows
  - [ ] Build completes
  - [ ] Unsigned installers created
  - [ ] Artifacts upload

### Test Run 4: Enable Code Signing (Windows)
- [ ] Configure code signing client based on research
- [ ] Add CODE_SIGNING_TOKEN secret
- [ ] Enable code signing step
- [ ] Trigger workflow
- [ ] Verify:
  - [ ] Signing client installs/downloads correctly
  - [ ] Executables get signed
  - [ ] Signed artifacts created
  - [ ] Check signature: `Get-AuthenticodeSignature file.exe`

### Test Run 5: S3 Publishing Test
- [ ] Configure AWS credentials
- [ ] Test with `publish_to_s3: true` (manual trigger)
- [ ] Verify uploads to S3 succeed
- [ ] Check files appear in bucket with correct paths
- [ ] Verify public-read ACL is set correctly
- [ ] Test downloading published artifacts from S3

## Phase 6: Production Deployment ✓

### Final Preparations
- [ ] Review all test results
- [ ] Document any known issues or workarounds
- [ ] Update README if CI badge or documentation needs changes
- [ ] Notify team of upcoming changes

### Deployment Options

**Option A: Clean Switch**
- [ ] Rename `full-build.yml` → `full-build-jenkins-backup.yml`
- [ ] Rename `full-build-github-hosted.yml` → `full-build.yml`
- [ ] Merge to develop branch
- [ ] Monitor first automatic build on develop

**Option B: Gradual Migration (Recommended)**
- [ ] Keep `full-build.yml` (Jenkins/self-hosted) as-is
- [ ] Deploy `full-build-github-hosted.yml` as separate workflow
- [ ] Run both in parallel for 1-2 weeks
- [ ] Compare results, identify any issues
- [ ] Once confident, disable Jenkins workflow
- [ ] Eventually remove old workflow file

**Option C: Hybrid Approach**
- [ ] Keep some platforms on self-hosted (e.g., ARM64, CentOS)
- [ ] Move others to GitHub-hosted
- [ ] Create separate workflows or use conditional logic

### Post-Deployment Monitoring
- [ ] Monitor first 5 builds closely
- [ ] Check for timeout issues
- [ ] Check for disk space issues
- [ ] Verify all artifacts are created correctly
- [ ] Verify S3 publishing works on develop branch
- [ ] Check Docker workflow trigger (if enabled)
- [ ] Monitor build times vs Jenkins baseline
- [ ] Gather team feedback

## Phase 7: Documentation ✓

### Update Project Documentation
- [ ] Update CI/CD documentation
- [ ] Update README.md with new build badge (if applicable)
- [ ] Document how to trigger manual builds
- [ ] Document how to download artifacts
- [ ] Create troubleshooting guide for common issues
- [ ] Document code signing setup for future maintainers

### Team Knowledge Transfer
- [ ] Share MIGRATION_GUIDE.md with team
- [ ] Share CODE_SIGNING_SETUP.md with team
- [ ] Demonstrate workflow_dispatch usage
- [ ] Show how to view logs and artifacts
- [ ] Explain differences from Jenkins

## Rollback Plan

If issues occur:
- [ ] Keep `full-build-jenkins-backup.yml` available
- [ ] Can revert by renaming files
- [ ] Jenkins runners should still be available during transition
- [ ] Document rollback procedure: 
  1. Rename `full-build.yml` → `full-build-github-temp.yml`
  2. Rename `full-build-jenkins-backup.yml` → `full-build.yml`
  3. Push to develop
  4. Verify Jenkins builds resume

## Success Criteria

Migration is successful when:
- [ ] All enabled platforms build successfully
- [ ] Test pass rates are comparable to Jenkins
- [ ] Artifacts are created and uploaded correctly
- [ ] S3 publishing works for develop branch
- [ ] Code signing works (Windows at minimum)
- [ ] Build times are acceptable (< 6 hours)
- [ ] No disk space issues
- [ ] Team is comfortable with new system
- [ ] Documentation is complete

## Notes & Issues Log

Use this space to track problems and solutions during migration:

```
Date: ___________
Issue: 
Solution:

---

Date: ___________
Issue:
Solution:

---
```

## Quick Reference

### Trigger Manual Build
```bash
# Via GitHub CLI
gh workflow run full-build-github-hosted.yml \
  --ref github-actions-test \
  -f publish_to_s3=false \
  -f skip_docker_trigger=true

# Via Web UI
Actions → Full Build (GitHub-Hosted Runners) → Run workflow
```

### Check Build Status
```bash
gh run list --workflow=full-build-github-hosted.yml
gh run view <run-id> --log
```

### Download Artifacts
```bash
gh run download <run-id>
```

### View Secrets
```bash
gh secret list
```

## Contact & Support

- GitHub Actions Documentation: https://docs.github.com/en/actions
- OpenStudio Team: [add contact info]
- Code Signing Support: [add contact for your signing service]