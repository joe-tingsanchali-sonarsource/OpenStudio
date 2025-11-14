# AWS Code Signing Migration Checklist

## Quick Reference: AWS Signer Setup for GitHub Actions

This checklist covers the specific steps to migrate your AWS code signing from Jenkins to GitHub Actions.

---

## ✅ Pre-Migration (On Jenkins Server)

### 1. Locate and Copy Signing Client
- [ ] Access Jenkins Windows runner
- [ ] Navigate to `C:\code-signing-client`
- [ ] Verify these files exist:
  - [ ] `code-signing.js` (main signing script)
  - [ ] `package.json` (Node.js dependencies)
  - [ ] `package-lock.json` (locked dependency versions)
  - [ ] `.env` (contains ACCESS_KEY and SECRET_KEY - DO NOT COPY)

### 2. Extract Credentials from .env File
- [ ] Open `C:\code-signing-client\.env`
- [ ] Copy the `ACCESS_KEY` value → Save securely
- [ ] Copy the `SECRET_KEY` value → Save securely
- [ ] **DO NOT** commit `.env` file to repository
- [ ] Verify these credentials are for AWS Signer service

### 3. Package Signing Client
```powershell
cd C:\code-signing-client

# Remove sensitive files
Remove-Item node_modules -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .env -Force -ErrorAction SilentlyContinue

# Verify what will be copied
Get-ChildItem *.js,package.json,package-lock.json

# Create archive
Compress-Archive -Path *.js,package.json,package-lock.json -DestinationPath code-signing-client.zip
```

- [ ] Archive created successfully
- [ ] Transfer `code-signing-client.zip` to your local machine

---

## 📦 Repository Setup

### 4. Add Signing Client to Repository
```bash
# In your OpenStudio repository root
mkdir -p .github/signing-client

# Extract the zip contents
# On Windows:
# Expand-Archive -Path code-signing-client.zip -DestinationPath .github/signing-client

# On macOS/Linux:
# unzip code-signing-client.zip -d .github/signing-client/

# Verify files are in place
ls -la .github/signing-client/
```

- [ ] Directory `.github/signing-client/` created
- [ ] `code-signing.js` present
- [ ] `package.json` present
- [ ] `package-lock.json` present
- [ ] **NO** `.env` file present (good!)
- [ ] **NO** `node_modules/` directory (good!)

### 5. Commit Signing Client
```bash
git add .github/signing-client/
git commit -m "Add AWS code signing client for GitHub Actions"

# Optional: Create test branch first
# git checkout -b github-actions-signing-test
# git push -u origin github-actions-signing-test
```

- [ ] Files committed to git
- [ ] Pushed to repository
- [ ] Verified files appear on GitHub

---

## 🔐 GitHub Secrets Configuration

### 6. Add AWS Signing Secrets
Go to: **Repository Settings → Secrets and variables → Actions → New repository secret**

Add these secrets:

**For AWS Code Signing:**
- [ ] **Secret name:** `AWS_SIGNING_ACCESS_KEY`
  - **Value:** The `ACCESS_KEY` from Jenkins `.env` file
  - **Verified:** Secret added successfully

- [ ] **Secret name:** `AWS_SIGNING_SECRET_KEY`
  - **Value:** The `SECRET_KEY` from Jenkins `.env` file
  - **Verified:** Secret added successfully

**For S3 Publishing (if not already configured):**
- [ ] **Secret name:** `AWS_ACCESS_KEY_ID`
  - **Value:** Your AWS access key for S3
  - **Verified:** Secret added successfully

- [ ] **Secret name:** `AWS_SECRET_ACCESS_KEY`
  - **Value:** Your AWS secret key for S3
  - **Verified:** Secret added successfully

- [ ] **Secret name:** `AWS_REGION` (optional)
  - **Value:** e.g., `us-west-2`
  - **Note:** Defaults to `us-west-2` if not set

### 7. Verify Secrets
```bash
# Using GitHub CLI
gh secret list

# Expected output should include:
# AWS_SIGNING_ACCESS_KEY
# AWS_SIGNING_SECRET_KEY
# AWS_ACCESS_KEY_ID
# AWS_SECRET_ACCESS_KEY
```

- [ ] All required secrets listed
- [ ] No typos in secret names
- [ ] Secret values are correct (no extra spaces/characters)

---

## 🔧 Workflow Configuration

### 8. Verify Workflow File
The workflow `full-build-github-hosted.yml` should include these steps:

- [ ] **Setup Node.js step** exists:
```yaml
- name: Setup Node.js
  uses: actions/setup-node@v4
  with:
    node-version: "18"
```

- [ ] **Install Signing Client Dependencies step** exists:
```yaml
- name: Install Signing Client Dependencies
  run: npm install
  working-directory: ./.github/signing-client
```

- [ ] **Create .env file step** exists:
```yaml
- name: Create .env file for Signing
  run: |
    echo "ACCESS_KEY=${{ secrets.AWS_SIGNING_ACCESS_KEY }}" > .env
    echo "SECRET_KEY=${{ secrets.AWS_SIGNING_SECRET_KEY }}" >> .env
  shell: pwsh
  working-directory: ./.github/signing-client
```

- [ ] **Code sign installer step** exists and uses correct paths:
```yaml
- name: Code sign installer
  shell: pwsh
  working-directory: ${{ github.workspace }}/${{ env.OPENSTUDIO_BUILD }}
  run: |
    # Check signing client exists
    # Run signing process
    node "${{ github.workspace }}/.github/signing-client/code-signing.js" ...
```

### 9. Path Verification
Ensure workflow uses GitHub-hosted paths (NOT Jenkins paths):

- [ ] ❌ NO references to `C:/code-signing-client` (old Jenkins path)
- [ ] ❌ NO references to `D:/OSN/` (old Jenkins path)
- [ ] ✅ Uses `${{ github.workspace }}/.github/signing-client/`
- [ ] ✅ Uses `${{ github.workspace }}/${{ env.OPENSTUDIO_BUILD }}/`

---

## 🧪 Testing

### 10. Test Build (Windows Only, No Signing)
First test without signing to verify build works:

- [ ] Edit workflow to comment out all signing steps
- [ ] Trigger workflow manually (workflow_dispatch)
- [ ] Select `publish_to_s3: false`
- [ ] Build completes successfully
- [ ] Unsigned `.exe` files created
- [ ] Artifacts uploaded to GitHub

### 11. Test Signing Client Installation
Test that signing dependencies install correctly:

- [ ] Uncomment "Setup Node.js" step
- [ ] Uncomment "Install Signing Client Dependencies" step
- [ ] Uncomment "Create .env file" step
- [ ] Comment out the actual signing execution (temporarily)
- [ ] Trigger workflow
- [ ] Check logs: `npm install` completes successfully
- [ ] Check logs: `.env` file created (don't print contents!)

### 12. Test Full Signing Process
- [ ] Uncomment all signing steps
- [ ] Trigger workflow manually
- [ ] Monitor "Code sign installer" step logs
- [ ] Verify no errors in signing process
- [ ] Check artifacts include `signed/` directory
- [ ] Download signed executable

### 13. Verify Signature
Download the signed installer and verify:

```powershell
# Check signature
Get-AuthenticodeSignature .\OpenStudio-*.exe | Format-List

# Expected output:
# Status        : Valid
# StatusMessage : Signature verified.
# SignerCertificate : [Your certificate info]
```

- [ ] Signature status is "Valid"
- [ ] Certificate matches expected signer
- [ ] Timestamp is present and valid
- [ ] No warnings or errors

---

## 🚀 Production Deployment

### 14. Enable Automatic Signing
- [ ] All tests passed successfully
- [ ] Signing works reliably
- [ ] Team notified of change
- [ ] Workflow enabled for `develop` branch

### 15. Monitor First Production Build
- [ ] First develop branch build triggered
- [ ] Signing completes successfully
- [ ] Signed artifacts uploaded to S3
- [ ] Downloaded and verified signature
- [ ] No issues reported

---

## 📝 Documentation

### 16. Document for Team
- [ ] Update internal docs with new signing process
- [ ] Document how to troubleshoot signing issues
- [ ] Document secret rotation procedure
- [ ] Share this checklist with team

### 17. Credential Management
- [ ] Document where AWS signing credentials are stored
- [ ] Set reminder for credential rotation (if applicable)
- [ ] Document who has access to manage secrets
- [ ] Backup credentials securely (encrypted vault)

---

## 🔄 Cleanup

### 18. Post-Migration Cleanup
- [ ] Jenkins workflow disabled or removed (optional)
- [ ] Old `C:/code-signing-client` backed up (if needed)
- [ ] Test branches deleted
- [ ] Migration notes archived

---

## ⚠️ Troubleshooting

### Common Issues and Solutions

**Issue:** "code-signing.js not found"
- **Solution:** Verify `.github/signing-client/code-signing.js` exists in repository
- **Check:** `git ls-files .github/signing-client/`

**Issue:** "ACCESS_KEY is not defined"
- **Solution:** Verify `.env` file creation step completed
- **Check:** Logs should show `.env` file created (but not contents)
- **Verify:** Secrets `AWS_SIGNING_ACCESS_KEY` and `AWS_SIGNING_SECRET_KEY` are set

**Issue:** "npm install failed"
- **Solution:** Check `package.json` is valid and committed
- **Check:** Node.js version compatibility (using v18)
- **Verify:** GitHub runner has internet access to npm registry

**Issue:** "Signing timeout"
- **Solution:** Increase timeout in command (currently 4800000ms = 80 min)
- **Check:** AWS Signer service status
- **Verify:** Network connectivity to AWS from GitHub runners

**Issue:** "Signature invalid"
- **Solution:** Verify AWS signing profile configuration
- **Check:** Certificate hasn't expired
- **Verify:** Correct signing profile being used

---

## ✅ Final Verification

All items checked? You're ready for production!

- [ ] Signing client in repository
- [ ] GitHub secrets configured
- [ ] Workflow updated with correct paths
- [ ] Test builds successful
- [ ] Signatures verified
- [ ] Team documented
- [ ] Production deployment successful

---

## Quick Command Reference

**Verify secrets:**
```bash
gh secret list
```

**Trigger test build:**
```bash
gh workflow run full-build-github-hosted.yml \
  --ref your-test-branch \
  -f publish_to_s3=false \
  -f skip_docker_trigger=true
```

**Check build status:**
```bash
gh run list --workflow=full-build-github-hosted.yml --limit 5
gh run view <run-id> --log
```

**Download artifacts:**
```bash
gh run download <run-id> --name packages-windows-2019-<sha>
```

**Verify signature:**
```powershell
Get-AuthenticodeSignature .\OpenStudio-*.exe | Format-List
signtool verify /pa /v .\OpenStudio-*.exe
```

---

**Migration Complete!** 🎉

Your AWS code signing is now fully integrated with GitHub Actions.