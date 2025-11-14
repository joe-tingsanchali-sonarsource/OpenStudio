# Migration Guide: Self-Hosted to GitHub-Hosted Runners

## Overview

This guide covers the migration of the OpenStudio build workflow from self-hosted Jenkins runners to GitHub-hosted runners.

## Summary of Changes

### Runner Mappings

| Original (Self-Hosted) | GitHub-Hosted | Notes |
|------------------------|---------------|-------|
| `linux-openstudio-2` | `ubuntu-22.04` or `ubuntu-24.04` | Standard GitHub runner |
| `linux-openstudio-centos9` | **SKIP or use container** | No CentOS GitHub runners available |
| `[self-hosted, linux, arm64, ubuntu-22]` | **NOT AVAILABLE** | No free ARM64 Linux runners |
| `[self-hosted, linux, arm64, ubuntu-24]` | **NOT AVAILABLE** | No free ARM64 Linux runners |
| `[self-hosted, macos, x64]` | `macos-13` | Intel-based macOS |
| `[self-hosted, macos, arm64]` | `macos-14` or `macos-latest` | Apple Silicon |
| `[self-hosted, Windows, X64, 2019]` | `windows-2022` or `windows-latest` | Windows Server 2022 |

### Working Directory Changes

| Platform | Self-Hosted | GitHub-Hosted |
|----------|-------------|---------------|
| Linux | `$GITHUB_WORKSPACE/OS-build-release-v2` | Same |
| macOS | `$GITHUB_WORKSPACE/OS-build-release-v2` | Same |
| Windows | `D:/OSN/OpenStudio` | `${{ github.workspace }}` or `C:\workspace` |

### Key Differences in GitHub-Hosted Runners

#### Advantages:
- ✅ No maintenance required
- ✅ Clean environment for every run
- ✅ Automatic updates
- ✅ No infrastructure costs for standard runners
- ✅ Better security isolation

#### Limitations:
- ❌ No ARM64 Linux support (free tier)
- ❌ No CentOS runners
- ❌ 6-hour job timeout limit
- ❌ Disk space: ~14GB available on Linux/Windows, ~14GB on macOS
- ❌ Cannot customize hardware specs (unless using paid larger runners)
- ❌ No persistent storage between runs

## Builds to Keep vs Skip

### ✅ Keep (Available on GitHub-Hosted Runners)

1. **Ubuntu 22.04 x64** - `ubuntu-22.04`
2. **Ubuntu 24.04 x64** - `ubuntu-24.04`
3. **macOS x64** - `macos-13`
4. **macOS ARM64** - `macos-14` or `macos-latest`
5. **Windows x64** - `windows-2022` or `windows-latest`

### ⚠️ Decision Required

1. **CentOS Stream 9** - Options:
   - Skip entirely (most common for GitHub-hosted setups)
   - Build RPM packages inside a CentOS container on Ubuntu
   - Use paid GitHub larger runners or keep as self-hosted

2. **Linux ARM64 (Ubuntu 22/24)** - Options:
   - Skip entirely (ARM64 packages less common)
   - Use GitHub's paid larger runners (4-core ARM64 available)
   - Keep these as self-hosted runners only

## Code Signing Setup

### Current System Analysis

Your workflow uses a **custom Node.js-based remote signing service**:
- Located at `C:/code-signing-client/code-signing.js`
- Uses `CODE_SIGNING_TOKEN` for authentication
- Sends zip files to remote service for signing

### Migration Steps for Code Signing

#### Step 1: Identify Your Signing Service

On your current Jenkins Windows runner:

1. Navigate to `C:/code-signing-client/`
2. Check `code-signing.js` for clues about the service:
   ```powershell
   Get-Content C:/code-signing-client/code-signing.js | Select-String -Pattern "http|api|endpoint|url"
   ```
3. Look for README or package.json:
   ```powershell
   Get-ChildItem C:/code-signing-client/ -Recurse
   ```

#### Step 2: Package the Signing Client

**Option A: Include in Repository (if allowed)**
```bash
# On Jenkins server
cd C:/code-signing-client/
# Remove sensitive data, node_modules
rm -rf node_modules/
# Copy to repository
```

**Option B: Download During Workflow**
Store the signing client in a secure location (Azure Blob Storage, S3, etc.) and download during workflow.

**Option C: Use Alternative Signing Service**
- SignPath.io (free for OSS)
- Azure Trusted Signing
- DigiCert ONE
- SSL.com eSigner

#### Step 3: Modify Workflow

Add before code signing step:
```yaml
- name: Setup code signing client
  shell: pwsh
  run: |
    # Install Node.js dependencies
    Set-Location "${{ github.workspace }}/.github/code-signing-client"
    npm ci
    
    # Or download from secure location
    # Invoke-WebRequest -Uri "${{ secrets.SIGNING_CLIENT_URL }}" -OutFile "client.zip"
    # Expand-Archive -Path "client.zip" -DestinationPath "$env:TEMP/code-signing-client"
```

Update signing step paths from `D:/OSN/...` to `${{ github.workspace }}/...`

## Required GitHub Secrets

Configure these in your repository settings (`Settings > Secrets and variables > Actions`):

### Essential Secrets:
- ✅ `AWS_ACCESS_KEY_ID` - For S3 uploads
- ✅ `AWS_SECRET_ACCESS_KEY` - For S3 uploads
- ✅ `CODE_SIGNING_TOKEN` - For Windows code signing
- ⚠️ `AWS_REGION` - Optional (defaults to 'us-west-2')
- ⚠️ `GH_DOCKER_TRIGGER_TOKEN` - Optional (defaults to GITHUB_TOKEN)

### For Alternative Code Signing (if switching):
- `SIGNPATH_API_TOKEN` - If using SignPath
- `AZURE_TENANT_ID`, `AZURE_CLIENT_ID`, `AZURE_CLIENT_SECRET` - If using Azure
- `CERTIFICATE_BASE64`, `CERTIFICATE_PASSWORD` - If using traditional certificate

## Windows Build Path Migration

### Current Setup:
- Working directory: `D:/OSN/OpenStudio`
- Assumes D: drive exists and has space

### GitHub-Hosted Runner:
- D: drive may not exist or have different structure
- Use `${{ github.workspace }}` instead

### Required Changes:

**Before:**
```yaml
working-directory: D:/OSN/${{ env.OPENSTUDIO_BUILD }}
```

**After:**
```yaml
working-directory: ${{ github.workspace }}/${{ env.OPENSTUDIO_BUILD }}
```

**Workspace preparation changes:**
```yaml
# Before (self-hosted)
- name: Prepare workspace
  run: |
    if (Test-Path 'D:/OSN') { Remove-Item 'D:/OSN' -Recurse -Force }
    New-Item -ItemType Directory -Path 'D:/OSN' | Out-Null
    Copy-Item -Path '${{ github.workspace }}' -Destination 'D:/OSN/OpenStudio' -Recurse

# After (GitHub-hosted)
- name: Prepare workspace
  run: |
    git config --global --add safe.directory "$env:GITHUB_WORKSPACE"
    New-Item -ItemType Directory -Path "$env:GITHUB_WORKSPACE/${{ env.OPENSTUDIO_BUILD }}" -Force
```

## Testing Strategy

### Phase 1: Single Platform Test
1. Create a test branch: `github-actions-migration`
2. Modify workflow to run **only Ubuntu 22.04** build
3. Set `publish_to_s3: false` initially
4. Trigger via `workflow_dispatch`
5. Monitor build time and resource usage

### Phase 2: Multi-Platform Test
1. Add macOS and Windows builds
2. Test code signing separately
3. Verify artifact uploads work
4. Test S3 publishing to a test bucket/prefix

### Phase 3: Full Migration
1. Enable all supported platforms
2. Update `develop` branch to use new workflow
3. Remove self-hosted runners (if fully migrating)
4. Document any platform-specific issues

## Pre-Migration Checklist

- [ ] Identify code signing service and obtain credentials
- [ ] Copy/document code signing client from Jenkins
- [ ] Configure all GitHub secrets
- [ ] Verify S3 bucket permissions work with GitHub OIDC
- [ ] Verify Docker Hub containers are publicly accessible
- [ ] Decide on CentOS build strategy (skip or container)
- [ ] Decide on ARM64 Linux builds (skip or paid runners)
- [ ] Update working directory paths for Windows
- [ ] Test Conan repository access from GitHub IPs
- [ ] Review disk space requirements vs GitHub runner limits (~14GB)
- [ ] Estimate build times (6-hour limit per job)

## Post-Migration Tasks

- [ ] Monitor first few builds for failures
- [ ] Update documentation/README with new CI info
- [ ] Set up branch protection rules if needed
- [ ] Configure notification preferences
- [ ] Archive/document old Jenkins setup
- [ ] Update team on new workflow triggers
- [ ] Create runbook for common build issues

## Disk Space Considerations

GitHub-hosted runners have limited disk space:
- Ubuntu/Windows: ~14GB available
- macOS: ~14GB available

Your build uses:
- Source checkout: ~XXX MB
- Conan cache: ~XXX MB
- Build artifacts: ~XXX MB
- Test results: ~XXX MB

**If space is tight:**
1. Clean Conan cache between steps
2. Remove build intermediates before packaging
3. Use separate jobs for build vs test
4. Consider using paid larger runners

## Cost Considerations

### Free Tier Limits (per month):
- Public repositories: **Unlimited** minutes
- Private repositories: 
  - 2,000 minutes for free accounts
  - 3,000 minutes for Pro
  - 50,000 minutes for Team

### Multipliers for Different Runners:
- Linux: 1x
- Windows: 2x
- macOS: 10x

**Example:** A 1-hour macOS build = 10 hours of free minutes

Since OpenStudio is **public**, you get unlimited free minutes! 🎉

## Rollback Plan

If migration fails:
1. Keep old workflow as `full-build-jenkins.yml`
2. Rename new workflow to `full-build-github.yml`
3. Both can coexist during transition
4. Switch back by renaming files or using different triggers

## Getting Help

Common issues and solutions:

### Build Timeout
- Split into separate jobs (build + test)
- Reduce parallelism if hitting CPU limits
- Consider paid larger runners

### Disk Space
- Clean intermediate files
- Use artifact caching strategically
- Split jobs to reduce concurrent disk usage

### Code Signing Fails
- Verify token is correct
- Check network access to signing service
- Increase timeout values
- Test signing locally first

### Container Issues
- Verify images are public on Docker Hub
- Test container pull manually
- Check for architecture mismatches

## Additional Resources

- [GitHub Actions Runner Specifications](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners)
- [GitHub Actions Usage Limits](https://docs.github.com/en/actions/learn-github-actions/usage-limits-billing-and-administration)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Encrypted Secrets](https://docs.github.com/en/actions/security-guides/encrypted-secrets)