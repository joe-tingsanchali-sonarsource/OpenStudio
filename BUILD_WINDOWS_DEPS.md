# Building and Uploading Windows C++20 Dependencies

We have encountered an issue where the Windows build fails because it requires C++20 compatible binaries for dependencies (like Ruby), but only C++14 binaries are available on our Conan remote. To fix this, we need to manually build these dependencies with C++20 and upload them to the remote.

A dedicated workflow file has been created for this purpose: `.github/workflows/build-windows-deps.yml`.

## Prerequisites

Before running the workflow, you must ensure the following secrets are configured in the GitHub repository:

1.  **Navigate to Secrets:**
    *   Go to your GitHub repository.
    *   Click on `Settings` > `Secrets and variables` > `Actions`.

2.  **Add/Verify Secrets:**
    *   `CONAN_USER`: The username for the `nrel-v2` Artifactory remote.
    *   `CONAN_PASSWORD`: The password or API key for the `nrel-v2` remote.

    *> **Note:** You will need to obtain these credentials from a team member if you do not have them.*

## Instructions

1.  **Go to Actions:**
    *   Click on the **Actions** tab in the repository.

2.  **Select Workflow:**
    *   Select **Build and Upload Windows Dependencies** from the list of workflows on the left.

3.  **Run Workflow:**
    *   Click the **Run workflow** button on the right.
    *   **Crucial Step:** Check the box labeled **Upload packages to remote**.
    *   Click the green **Run workflow** button.

## Verification

Once the workflow completes successfully:
1.  The new C++20 binaries for Windows will be available on the `nrel-v2` remote.
2.  You can then re-run the main `full-build.yml` workflow (or wait for the next scheduled run), and it should now succeed on Windows.
