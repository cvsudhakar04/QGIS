# -*- coding: utf-8 -*-

"""
***************************************************************************
    ZonalStatisticsQgis.py
    ---------------------
    Date                 : September 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.analysis import QgsZonalStatistics

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects


class ZonalStatisticsQgis(GeoAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    RASTER_BAND = 'RASTER_BAND'
    INPUT_VECTOR = 'INPUT_VECTOR'
    COLUMN_PREFIX = 'COLUMN_PREFIX'
    STATISTICS = 'STATS'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.STATS = {self.tr('Count'): QgsZonalStatistics.Count,
                      self.tr('Sum'): QgsZonalStatistics.Count,
                      self.tr('Mean'): QgsZonalStatistics.Count,
                      self.tr('Median'): QgsZonalStatistics.Count,
                      self.tr('Std. dev.'): QgsZonalStatistics.Count,
                      self.tr('Min'): QgsZonalStatistics.Count,
                      self.tr('Max'): QgsZonalStatistics.Count,
                      self.tr('Range'): QgsZonalStatistics.Count,
                      self.tr('Minority'): QgsZonalStatistics.Count,
                      self.tr('Majority'): QgsZonalStatistics.Count,
                      self.tr('Variety'): QgsZonalStatistics.Count,
                      self.tr('All'): QgsZonalStatistics.All
                     }

        self.name, self.i18n_name = self.trAlgorithm('Zonal Statistics (QGIS)')
        self.group, self.i18n_group = self.trAlgorithm('Raster tools')

        self.addParameter(ParameterRaster(self.INPUT_RASTER,
                                          self.tr('Raster layer')))
        self.addParameter(ParameterNumber(self.RASTER_BAND,
                                          self.tr('Raster band'), 1, 999, 1))
        self.addParameter(ParameterVector(self.INPUT_VECTOR,
                                          self.tr('Vector layer containing zones'),
                                          [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterString(self.COLUMN_PREFIX,
                                          self.tr('Output column prefix'), '_'))
        self.addParameter(ParameterSelection(self.STATISTICS,
                                          self.tr('Statistics to calculate'),
                                          list(self.STATS.keys()),
                                          multiple=True))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Zonal statistics'),
                                    True,
                                    datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, progress):
        rasterPath = self.getParameterValue(self.INPUT_RASTER)
        vectorPath = self.getParameterValue(self.INPUT_VECTOR)
        bandNumber = self.getParameterValue(self.RASTER_BAND)
        columnPrefix = self.getParameterValue(self.COLUMN_PREFIX)
        st = self.getParameterValue(self.STATISTICS)

        vectorLayer = dataobjects.getObjectFromUri(vectorPath)

        keys = list(self.STATS.keys())
        selectedStats = 0
        for i in st:
           selectedStats |= self.STATS[keys[i]]

        zs = QgsZonalStatistics(vectorLayer, rasterPath, columnPrefix, bandNumber, selectedStats)
        zs.calculateStatistics(None)

        self.setOutputValue(self.OUTPUT_LAYER, vectorPath)
