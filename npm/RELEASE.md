
# Semi automated release process

## Requirements 
 * Owner permission

## Steps
 1. Go to Actions
 2. Click on the bump version action in the sidebar
 3. Run workflow (choose major, minor, patch) & wait to finish
 4. Go to pull requests
 5. Merge the `Bumping package to...` pull request
 6. Check releases, there should be a draft release
 7. Update content
 8. Publish
 9. Wait for the release action to finish
 10. Verify on NPM (might take a bitto propagte) 