#!/bin/bash

set -euo pipefail

read -p "commit message: " msg

git add .
git commit -m "$msg"
git push

sleep 1

latest=$(gh run list --limit 1 --json databaseId -q '.[0].databaseId')

gh run watch "$latest"

conclusion=$(gh run view "$latest" --json conclusion -q '.conclusion')

if [[ "$conclusion" == "success" ]]; then
  echo "Run succeeded, downloading artifact..."
  gh run download "$latest" -n APK -D ../../storage/downloads/
  echo "Artifacts downloaded to ./artifacts"
else
  echo "Run did not succeed (conclusion: $conclusion). No artifacts downloaded."
  exit 1
fi
