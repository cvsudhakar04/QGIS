###########################################################################
#    script.sh
#    ---------------------
#    Date                 : February 2017
#    Copyright            : (C) 2017 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
set -e

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

export QGISSTYLE=${DIR}/scripts/qgisstyle

pushd build
python ${TRAVIS_BUILD_DIR}/.ci/travis/scripts/ctest2travis.py \
  xvfb-run ctest -V --output-on-failure
popd
