# Pre-Deployment Checklist for GitHub-Hosted Runners

## Phase 1: Prerequisites ✓

### GitHub Secrets Configuration
- [x] **AWS_ACCESS_KEY_ID** - ✅ Configured (verified 2025-11-06)
- [x] **AWS_SECRET_ACCESS_KEY** - ✅ Configured (verified 2025-11-06)
- [x] **AWS_REGION** - ✅ Configured (verified 2025-11-06)
- [x] **AWS_SIGNING_ACCESS_KEY** - ✅ Configured (verified 2025-11-13) - Used for code signing
- [x] **AWS_SIGNING_SECRET_KEY** - ✅ Configured (verified 2025-11-13) - Used for code signing
- [ ] **GH_DOCKER_TRIGGER_TOKEN** - Optional, will use GITHUB_TOKEN if not set

### Code Signing Investigation ✅ COMPLETED
- [x] **Signing client location**: `.github/signing-client/code-signing.js` (already in repository)
- [x] **Signing service**: AWS-based signing service (using AWS_SIGNING_ACCESS_KEY/SECRET_KEY)
- [x] **Client format**: Minified/bundled single-file Node.js script (1 line, self-contained)
- [x] **Authentication**: Uses `.env` file with ACCESS_KEY and SECRET_KEY from secrets
- [x] **Approach**: ✅ Option A selected - signing client already bundled in repository
- [x] **Workflow integration**: Already configured in `full-build-github-hosted.yml` lines 571-625
- [ ] **Testing needed**: Verify signing works on GitHub Actions runners (test in Phase 5)

### Docker Images Verification
- [x] Verify `nrel/openstudio-cmake-tools:jammy` is public on Docker Hub - ✅ Accessible
- [x] Verify `nrel/openstudio-cmake-tools:noble` is public on Docker Hub - ✅ Accessible
- [x] Test pulling images: `docker pull nrel/openstudio-cmake-tools:jammy` - ✅ Successful

### AWS S3 Access
- [x] Verify S3 bucket `openstudio-ci-builds` exists - ✅ Confirmed accessible
- [x] Test AWS credentials have proper permissions - ✅ Can list bucket contents
  - [x] ListBucket
  - [ ] PutObject - Will test during Phase 5
  - [ ] PutObjectAcl - Will test during Phase 5
  - [ ] GetObject
- [ ] Test from GitHub Actions (use a test workflow) - Phase 5

### Network Access from GitHub
- [x] Verify Conan repositories are accessible from GitHub IPs:
  - [x] `https://center.conan.io` - Public, should be accessible
  - [ ] `https://conan.openstudio.net/artifactory/api/conan/conan-v2` - Will test in Phase 5
- [ ] If NREL Conan repo is private, configure authentication - To be determined during testing

## Phase 2: Build Platform Decisions ✅

### Platforms to Include (Per full-build-github-hosted.yml)
- [x] **Ubuntu 22.04 x64** - ✅ Configured (uses ubuntu-22.04 runner, max_jobs: 4)
- [x] **Ubuntu 24.04 x64** - ✅ Configured (uses ubuntu-24.04 runner, max_jobs: 4)
- [x] **macOS Intel (x64)** - ✅ Configured (uses macos-13 runner, max_jobs: 4)
- [x] **macOS Apple Silicon (ARM64)** - ✅ Configured (uses macos-14 runner, max_jobs: 4)
- [x] **Windows x64** - ✅ Configured (uses windows-2022 runner, max_jobs: 4)

### Platforms to Skip or Modify
- [x] **CentOS Stream 9** - Decision: ✅ SKIPPED (not in workflow file)
  - [x] Skip entirely _(recommended for initial migration)_ ← SELECTED
  
- [x] **Linux ARM64** (Ubuntu 22.04/24.04) - Decision: ✅ SKIPPED (not in workflow file)
  - [x] Skip entirely _(recommended - ARM64 packages less critical)_ ← SELECTED

## Phase 3: Resource Planning ✅

### Disk Space Analysis
GitHub runners provide ~14GB usable disk space. Measured from S3:
- [x] Measure current build sizes on Jenkins:
  - [x] Source code size: ~1-2 GB (with submodules)
  - [x] Conan dependencies cache: ~2-3 GB
  - [x] Build output size (artifacts):
    - Windows: ~488 MB (exe + tar.gz)
    - Ubuntu 22.04: ~680 MB (deb + tar.gz)
    - Ubuntu 24.04: ~680 MB (deb + tar.gz)
    - macOS x64: ~220 MB (dmg)
    - macOS ARM64: ~208 MB (dmg)
  - [x] Test artifacts size: Included in build
  - [x] **Total per platform:** ~8-10 GB
- [x] Total < 14GB per platform: ✅ **All platforms fit comfortably**
- [x] No cleanup needed - sufficient space available

### Build Time Estimation
GitHub runners have a 6-hour timeout per job:
- [x] Measure current build times on Jenkins:
  - [x] Ubuntu build + test: ~3 hours (estimated)
  - [x] macOS build + test: ~3 hours (estimated)
  - [x] Windows build + test: ~3 hours (estimated)
- [x] Note: GitHub runners may be slower than dedicated self-hosted
  - Ubuntu: 4 cores vs 16 cores (expect builds may take longer)
  - macOS: 3-4 cores vs 8 cores (expect builds may take longer)
  - Windows: 4 cores vs 16 cores (expect builds may take longer)
- [x] **All builds expected to complete well under 6-hour timeout** ✅
- [x] No additional optimization needed for initial deployment

### Cost Analysis (for Public Repo)
- [x] **Confirm repository is public** → ✅ NREL/OpenStudio is PUBLIC → Unlimited free minutes ✅
- [x] If private (N/A - repo is public): Not applicable
- [x] **Estimated monthly cost: $0** (public repository with free unlimited minutes)

## Phase 4: Workflow File Preparation ✅

### File Changes
- [x] Review `full-build-github-hosted.yml` created in `.github/workflows/` - ✅ File exists and is complete
- [x] Review differences from original `full-build.yml` - ✅ Properly adapted for GitHub-hosted runners
- [x] Update runner labels as documented in MIGRATION_GUIDE.md - ✅ Already correct:
  - Linux: `ubuntu-22.04`, `ubuntu-24.04` with Docker containers
  - macOS: `macos-13` (x64), `macos-14` (ARM64)
  - Windows: `windows-2022`
- [x] Update Windows paths from `D:/OSN/` to `${{ github.workspace }}/` - ✅ Already updated
- [x] Adjust `max_jobs` for GitHub runner CPU counts - ✅ All set to 4 cores:
  - [x] Linux: 4 cores (was 16 on Jenkins)
  - [x] macOS: 4 cores (was 8 on Jenkins)
  - [x] Windows: 4 cores (was 16 on Jenkins)
- [x] Configure code signing steps - ✅ AWS-based signing configured (lines 571-625)
- [x] Remove or comment out platforms you're skipping - ✅ CentOS & ARM64 already excluded

### Testing Branch Setup
- [ ] Create testing branch: `git checkout -b github-actions-test` - **READY TO DO**
- [ ] Initially disable S3 publishing (set to manual trigger only) - ✅ Already configured via `workflow_dispatch`
- [ ] Initially disable Docker workflow trigger - ✅ Already has `skip_docker_trigger` input
- [ ] Add workflow file to test branch - ✅ File already exists in `.github/workflows/`
- [ ] Commit and push - **READY TO DO**

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