# Quick Start Guide: GitHub Actions Migration

## TL;DR - Get Running in 3 Steps

This is the fastest path to migrate OpenStudio builds from Jenkins to GitHub Actions.

---

## Step 1: Copy AWS Code Signing Client (15 min)

### On Jenkins Windows Runner:

```powershell
cd C:\code-signing-client

# Remove sensitive files
Remove-Item node_modules -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .env -Force -ErrorAction SilentlyContinue

# Verify files
Get-ChildItem *.js,package.json,package-lock.json

# Package it
Compress-Archive -Path *.js,package.json,package-lock.json -DestinationPath code-signing-client.zip
```

### Extract credentials from .env:

```powershell
Get-Content .env
# Copy the ACCESS_KEY value
# Copy the SECRET_KEY value
```

### In Your OpenStudio Repository:

```bash
# Create directory
mkdir -p .github/signing-client

# Extract zip contents to .github/signing-client/
# Files needed: code-signing.js, package.json, package-lock.json

# Commit to repository
git add .github/signing-client/
git commit -m "Add AWS code signing client"
git push
```

---

## Step 2: Configure GitHub Secrets (5 min)

Go to: **Repository Settings → Secrets and variables → Actions → New repository secret**

Add these 4 secrets:

| Secret Name | Value | Source |
|-------------|-------|--------|
| `AWS_SIGNING_ACCESS_KEY` | `<ACCESS_KEY>` | From Jenkins `.env` file |
| `AWS_SIGNING_SECRET_KEY` | `<SECRET_KEY>` | From Jenkins `.env` file |
| `AWS_ACCESS_KEY_ID` | `<your-aws-key>` | For S3 publishing (you have this) |
| `AWS_SECRET_ACCESS_KEY` | `<your-aws-secret>` | For S3 publishing (you have this) |

**Verify:**
```bash
gh secret list
```

---

## Step 3: Deploy and Test Workflow (30 min)

### Copy the Workflow File:

```bash
# Copy the ready-to-use workflow
cp .github/workflows-migration/full-build-github-hosted.yml \
   .github/workflows/full-build-github-hosted.yml

# Create test branch
git checkout -b github-actions-test
git add .github/workflows/
git commit -m "Add GitHub Actions workflow"
git push -u origin github-actions-test
```

### Test Ubuntu Build First:

**Option A: Via GitHub CLI**
```bash
gh workflow run full-build-github-hosted.yml \
  --ref github-actions-test \
  -f publish_to_s3=false \
  -f skip_docker_trigger=true
```

**Option B: Via GitHub Web UI**
1. Go to **Actions** tab
2. Select **Full Build (GitHub-Hosted Runners)**
3. Click **Run workflow**
4. Select branch: `github-actions-test`
5. Set `publish_to_s3`: `false`
6. Click **Run workflow**

### Monitor the Build:

```bash
# Watch status
gh run list --workflow=full-build-github-hosted.yml

# View logs
gh run view <run-id> --log

# Download artifacts when done
gh run download <run-id>
```

---

## What to Expect

### Build Times (vs Jenkins):
- **Ubuntu:** 2-3x slower (4 cores vs 16 cores)
- **macOS:** 2x slower (4 cores vs 8 cores)  
- **Windows:** 2-3x slower (4 cores vs 16 cores)

Still well within 6-hour timeout limit.

### Platforms Included:
- ✅ Ubuntu 22.04 x64
- ✅ Ubuntu 24.04 x64
- ✅ macOS Intel (x64)
- ✅ macOS Apple Silicon (ARM64)
- ✅ Windows x64

### Platforms Excluded:
- ❌ CentOS Stream 9 (no GitHub runner)
- ❌ Linux ARM64 (no free ARM64 runners)

### Cost:
- **$0** - OpenStudio is public = unlimited free minutes! 🎉

---

## Troubleshooting

### Build fails with "code-signing.js not found"
```bash
# Verify files are committed
git ls-files .github/signing-client/

# Should show:
# .github/signing-client/code-signing.js
# .github/signing-client/package.json
# .github/signing-client/package-lock.json
```

### Build fails with "ACCESS_KEY not defined"
```bash
# Verify secrets are set
gh secret list

# Should include:
# AWS_SIGNING_ACCESS_KEY
# AWS_SIGNING_SECRET_KEY
```

### npm install fails in signing client
- Check that `package.json` is valid JSON
- Verify Node.js version (using v18)
- Check workflow logs for specific npm error

### Signature verification fails
```powershell
# Download signed .exe and check
Get-AuthenticodeSignature .\OpenStudio-*.exe | Format-List

# Should show Status: Valid
```

---

## Testing Progression

### Phase 1: Ubuntu Only (Low Risk)
1. Edit workflow to keep only Ubuntu 22.04 matrix entry
2. Comment out macOS and Windows jobs
3. Test basic build works
4. ✅ Verify artifacts upload

### Phase 2: Add macOS
1. Uncomment macOS job
2. Test both platforms build
3. ✅ Verify .dmg and .tar.gz created

### Phase 3: Add Windows (No Signing)
1. Uncomment Windows job  
2. Comment out signing steps temporarily
3. Test Windows build works
4. ✅ Verify unsigned .exe created

### Phase 4: Enable Signing
1. Uncomment all signing steps
2. Test full signing process
3. Download and verify signature
4. ✅ Signed installer works

### Phase 5: Enable S3 Publishing
1. Set `publish_to_s3: true`
2. Test uploads to S3
3. ✅ Verify public download links work

---

## Production Deployment

### Option A: Gradual (Recommended)

```bash
git checkout develop

# Keep old workflow as backup
git mv .github/workflows/full-build.yml \
       .github/workflows/full-build-jenkins.yml

# Deploy new workflow
git mv .github/workflows/full-build-github-hosted.yml \
       .github/workflows/full-build.yml

git commit -m "Deploy GitHub Actions (alongside Jenkins)"
git push
```

Run both for 1-2 weeks, then remove Jenkins workflow.

### Option B: Clean Switch

```bash
git checkout develop

# Backup original
cp .github/workflows/full-build.yml \
   .github/workflows/full-build-jenkins-backup.yml

# Replace with new
cp .github/workflows/full-build-github-hosted.yml \
   .github/workflows/full-build.yml

git add .github/workflows/
git commit -m "Switch to GitHub-hosted runners"
git push
```

Monitor first build closely. Keep backup for quick rollback if needed.

---

## Success Checklist

- [x] Code signing client copied to repository
- [x] AWS signing secrets configured
- [x] Workflow file deployed
- [ ] Ubuntu test build passes
- [ ] macOS test build passes
- [ ] Windows test build passes (unsigned)
- [ ] Windows signing works
- [ ] Signature verified as valid
- [ ] S3 publishing works
- [ ] Team notified and comfortable
- [ ] Production deployment successful

---

## Additional Resources

**In this directory:**
- `AWS_SIGNING_CHECKLIST.md` - Detailed signing setup
- `NEXT_STEPS.md` - Complete action plan
- `CODE_SIGNING_SETUP.md` - Code signing documentation
- `MIGRATION_GUIDE.md` - Technical deep dive
- `DEPLOYMENT_CHECKLIST.md` - Full migration checklist

**GitHub Docs:**
- [GitHub Actions](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [GitHub Hosted Runners](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners)

---

## Getting Help

**Common commands:**
```bash
# List workflows
gh workflow list

# Run workflow
gh workflow run <workflow-name> --ref <branch>

# List recent runs
gh run list --limit 10

# View run details
gh run view <run-id>

# View logs
gh run view <run-id> --log

# Download artifacts
gh run download <run-id>

# List secrets
gh secret list
```

**Questions?** Review the detailed guides in this directory or check GitHub Actions documentation.

---

**You're ready to go! Start with Step 1 above.** 🚀