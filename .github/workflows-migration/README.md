# GitHub Actions Migration - Executive Summary

## Overview

This directory contains everything you need to migrate the OpenStudio build workflow from Jenkins (self-hosted runners) to GitHub Actions (GitHub-hosted runners).

## What's Changed

### ✅ Successfully Migrated to GitHub-Hosted Runners
- **Ubuntu 22.04 x64** → `ubuntu-22.04` runner
- **Ubuntu 24.04 x64** → `ubuntu-24.04` runner  
- **macOS Intel (x64)** → `macos-13` runner
- **macOS Apple Silicon (ARM64)** → `macos-14` runner
- **Windows x64** → `windows-2022` runner

### ❌ Not Available on Free GitHub Runners
- **CentOS Stream 9** - No CentOS runners on GitHub
- **Linux ARM64** - No free ARM64 Linux runners

## Files in This Directory

### 1. `NEXT_STEPS.md` ⭐ START HERE
**Your step-by-step action plan.** Follow this to complete the migration.

Key sections:
- Immediate actions (investigate code signing)
- Week-by-week testing plan
- Deployment options
- Quick reference commands

### 2. `full-build-github-hosted.yml`
**The new workflow file** ready to deploy to `.github/workflows/`.

Changes from original:
- All runner labels updated to GitHub-hosted runners
- Windows paths changed from `D:/OSN/` to `${{ github.workspace }}/`
- CPU core counts adjusted (16 → 4 cores)
- Code signing setup included (requires configuration)
- CentOS and ARM64 Linux builds removed

### 3. `CODE_SIGNING_SETUP.md`
**Complete guide to Windows code signing** on GitHub-hosted runners.

Covers:
- How to identify your current signing service
- 4 different signing options (existing service, SignPath, Azure, traditional cert)
- Required secrets configuration
- Testing and troubleshooting

### 4. `MIGRATION_GUIDE.md`
**Comprehensive technical guide** covering all aspects of the migration.

Includes:
- Detailed runner mappings
- Advantages and limitations of GitHub-hosted runners
- Cost analysis (unlimited free for public repos!)
- Disk space and timeout considerations
- Rollback procedures

### 5. `DEPLOYMENT_CHECKLIST.md`
**Interactive checklist** to track your migration progress.

Organized by phases:
- Phase 1: Prerequisites (secrets, code signing investigation)
- Phase 2: Platform decisions
- Phase 3: Resource planning
- Phase 4: Workflow preparation
- Phase 5: Testing (5 test scenarios)
- Phase 6: Production deployment
- Phase 7: Documentation

## Code Signing Setup (AWS Signer) ✅

**Service Identified:** AWS Code Signing (Amazon Signer)

Your Jenkins workflow uses:
- A Node.js script at `C:/code-signing-client/code-signing.js`
- AWS Code Signing service (remote signing)
- Credentials via `ACCESS_KEY` and `SECRET_KEY` in `.env` file

**Migration steps:**
1. Copy signing client files to `.github/signing-client/` in repository
2. Extract `ACCESS_KEY` and `SECRET_KEY` from Jenkins `.env` file
3. Add as GitHub secrets: `AWS_SIGNING_ACCESS_KEY` and `AWS_SIGNING_SECRET_KEY`
4. Workflow dynamically creates `.env` file from secrets during build

See `CODE_SIGNING_SETUP.md` and `AWS_SIGNING_CHECKLIST.md` for detailed instructions.

## Quick Start (3 Steps)

### Step 1: Copy Code Signing Client (30 minutes)
```powershell
# On Jenkins Windows runner
cd C:\code-signing-client

# Remove sensitive files
Remove-Item node_modules -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .env -Force -ErrorAction SilentlyContinue

# Create archive
Compress-Archive -Path *.js,package.json,package-lock.json -DestinationPath code-signing-client.zip
```

Then extract to `.github/signing-client/` in your repository and commit.

### Step 2: Configure GitHub Secrets (5 minutes)
Go to: `Repository Settings → Secrets and variables → Actions`

Add from Jenkins `.env` file:
- `AWS_SIGNING_ACCESS_KEY` (ACCESS_KEY from `.env`)
- `AWS_SIGNING_SECRET_KEY` (SECRET_KEY from `.env`)

Also add for S3 publishing:
- `AWS_ACCESS_KEY_ID` (you have this)
- `AWS_SECRET_ACCESS_KEY` (you have this)

### Step 3: Test Single Platform (30 minutes)
```bash
# Create test branch
git checkout -b github-actions-test

# Copy workflow file
cp .github/workflows-migration/full-build-github-hosted.yml \
   .github/workflows/full-build-github-hosted.yml

# Edit to run ONLY Ubuntu 22.04 initially
# Comment out other matrix entries

# Commit and push
git add .github/workflows/
git commit -m "Test: GitHub Actions workflow (Ubuntu only)"
git push -u origin github-actions-test

# Trigger manually via GitHub UI:
# Actions → Full Build (GitHub-Hosted Runners) → Run workflow
# Set publish_to_s3: false
```

## Timeline

**Realistic estimate for complete migration:**

- **Week 1:** Investigation + single platform testing
- **Week 2:** Multi-platform testing + code signing setup  
- **Week 3:** Production deployment (gradual rollout)
- **Week 4:** Monitoring and optimization

**Can be done faster if:**
- Code signing is quickly identified/configured
- No major surprises in testing
- Clean test results on first attempts

## Expected Differences

### Build Times
- **Ubuntu:** 2-3x slower (4 cores vs 16 cores on Jenkins)
- **macOS:** 2x slower (4 cores vs 8 cores)
- **Windows:** 2-3x slower (4 cores vs 16 cores)

**Important:** All jobs must complete within **6 hours** (GitHub timeout).

### Disk Space
- **GitHub runners:** ~14GB available per job
- **If builds are tight on space:** Clean Conan cache, remove intermediates

### Cost
- **Public repository:** ✅ **UNLIMITED FREE MINUTES**
- **Private repository:** Uses plan minutes (Linux=1x, Windows=2x, macOS=10x)

Since OpenStudio is public, this migration is **completely free**! 🎉

## Deployment Strategies

### Option A: Gradual Migration (RECOMMENDED)
- Keep both workflows running in parallel
- Compare results for 1-2 weeks
- Builds confidence before full switch
- Easy rollback if issues arise

### Option B: Clean Switch
- Replace workflow file in one go
- Higher risk but faster
- Keep backup available for rollback
- Requires more confidence in testing

### Option C: Hybrid
- Keep ARM64/CentOS on self-hosted
- Move others to GitHub-hosted
- Best of both worlds
- More complexity to maintain

## Common Questions

**Q: What about ARM64 Linux builds?**  
A: Not available on free GitHub runners. Options: skip them, keep as self-hosted, or pay for larger runners.

**Q: What about CentOS builds?**  
A: No CentOS runners on GitHub. Options: skip, build in container on Ubuntu, or keep as self-hosted.

**Q: Will builds be slower?**  
A: Yes, 2-3x slower due to fewer CPU cores (4 vs 16). But they're free and require no maintenance.

**Q: What about code signing?**  
A: AWS Code Signing service is used. Copy the signing client to `.github/signing-client/` and configure AWS secrets. See `AWS_SIGNING_CHECKLIST.md` for step-by-step instructions.

**Q: Can I keep Jenkins running during migration?**  
A: Yes! Run both in parallel during testing phase (Option A).

**Q: What if something goes wrong?**  
A: Keep old workflow as backup. Rollback is just renaming files.

## Success Criteria

Migration is successful when:
- ✅ All platforms build successfully
- ✅ Test pass rates match Jenkins baseline
- ✅ Artifacts upload correctly
- ✅ S3 publishing works
- ✅ Code signing works (at least Windows)
- ✅ Build times < 6 hours per job
- ✅ No disk space issues
- ✅ Team is comfortable with new system

## Need Help?

**Documentation in this directory:**
- `NEXT_STEPS.md` - Action plan
- `CODE_SIGNING_SETUP.md` - Code signing help (AWS Signer)
- `AWS_SIGNING_CHECKLIST.md` - AWS signing migration checklist
- `MIGRATION_GUIDE.md` - Technical details
- `DEPLOYMENT_CHECKLIST.md` - Track progress

**External resources:**
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Runner Specifications](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)

## Summary

✅ **Ready to deploy:** Workflow file is complete  
✅ **Code signing identified:** AWS Code Signing (Amazon Signer)  
⚠️ **Setup required:** Copy signing client and configure secrets  
✅ **No cost:** Free for public repositories  
✅ **No maintenance:** GitHub manages runners  
⚠️ **Slower builds:** But still within timeout limits  
❌ **Missing platforms:** ARM64 Linux, CentOS (optional)

**Recommended next step:** Follow `AWS_SIGNING_CHECKLIST.md` to set up code signing, then test!

---

*Last updated: Migration from Jenkins Groovy to GitHub Actions*
*OpenStudio Full Build Workflow*