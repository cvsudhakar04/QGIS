###########################################################################
#    script.sh
#    ---------------------
#    Date                 : March 2016
#    Copyright            : (C) 2016 by Matthias Kuhn
#    Email                : matthias at opengis dot ch
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

export PYTHONPATH=${HOME}/osgeo4travis/lib/python3.3/site-packages/
export PATH=${HOME}/osgeo4travis/bin:${HOME}/osgeo4travis/sbin:${HOME}/OTB-5.6.0-Linux64/bin:${PATH}
export LD_LIBRARY_PATH=${HOME}/osgeo4travis/lib
export CTEST_PARALLEL_LEVEL=1
export CCACHE_TEMPDIR=/tmp
ccache -M 2G

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Set OTB application path (installed in before_install.sh script)
export OTB_APPLICATION_PATH=${HOME}/OTB-5.6.0-Linux64/lib/otb/applications
export LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so

# xvfb-run ctest -V -E "qgis_openstreetmaptest|qgis_wcsprovidertest" -S ./qgis-test-travis.ctest --output-on-failure
if [ "$CACHE_WARMING" = true ] ; then
  echo "WARNING: CACHE WARMING IS ACTIVE. SET CACHE_WARMING=false TO GET MEANINGFUL RESULTS."
  xvfb-run ctest -V -R NOTESTS -S ./qgis-test-travis.ctest --output-on-failure
  false
else
  xvfb-run ctest -V -E "qgis_filedownloader|qgis_openstreetmaptest|qgis_wcsprovidertest|PyQgsWFSProviderGUI|qgis_ziplayertest|$(cat ${DIR}/blacklist.txt | paste -sd '|' -)" -S ./qgis-test-travis.ctest --output-on-failure
fi
