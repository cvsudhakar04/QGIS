DIR=$(git rev-parse --show-toplevel)
cd $1
FOLDER=linux
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=linux
fi
ctest -E "$(cat ${DIR}/.ci/travis/${FOLDER}/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" --output-on-failure
cd $DIR
