#!/bin/bash
set -e

export PATH=${HOME}/osgeo4travis/bin:${PATH}


echo "Spell check"

if [[ !  -z  $TRAVIS_PULL_REQUEST_BRANCH  ]]; then
  # if on a PR, just analyse the changed files
  echo "TRAVIS PR BRANCH: $TRAVIS_PULL_REQUEST_BRANCH"
  FILES=$(git diff --diff-filter=AM --name-only $(git merge-base HEAD master) | tr '\n' ' ' )
fi
           
./scripts/chkspelling_ag.sh $FILES

