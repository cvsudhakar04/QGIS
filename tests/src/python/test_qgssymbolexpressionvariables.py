# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbolexpressionvariables.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias at opengis dot ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthiasd Kuhn'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from PyQt4.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSingleSymbolRendererV2,
                       QgsFillSymbolV2,
                       QgsMarkerSymbolV2,
                       QgsRendererCategoryV2,
                       QgsCategorizedSymbolRendererV2,
                       QgsGraduatedSymbolRendererV2,
                       QgsRendererRangeV2,
                       QgsFeatureRequest
                       )
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
# Convenience instances in case you may need them
# not used in this test
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSymbolExpressionVariables(TestCase):

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.layer = QgsVectorLayer(myShpFile, 'Polys', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(self.layer)

        rendered_layers = [self.layer.id()]
        self.mapsettings = CANVAS.mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def testPartNum(self):
        # Create rulebased style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})

        renderer = QgsSingleSymbolRendererV2(sym1)
        renderer.symbols()[0].symbolLayers()[0].setDataDefinedProperty('color', 'color_rgb( (@geometry_part_num - 1) * 200, 0, 0 )')
        self.layer.setRendererV2(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometry_part_num')
        result = renderchecker.runTest('part_geometry_part_num')

        self.assertTrue(result)

    def testGeometryPart(self):
        # Create rulebased style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})

        renderer = QgsSingleSymbolRendererV2(sym1)
        renderer.symbols()[0].symbolLayers()[0].setDataDefinedProperty('color', 'color_rgb( if( x_min( @geometry_part) < -104, 200, 0 ), 0, 0 )')
        self.layer.setRendererV2(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometry_part')
        result = renderchecker.runTest('part_geometry_part')

        self.assertTrue(result)

    def testPartCount(self):
        # Create rulebased style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})

        renderer = QgsSingleSymbolRendererV2(sym1)
        renderer.symbols()[0].symbolLayers()[0].setDataDefinedProperty('color', 'color_rgb( (@geometry_part_count - 1) * 200, 0, 0 )')
        self.layer.setRendererV2(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometry_part_count')
        result = renderchecker.runTest('part_geometry_part_count')

        self.assertTrue(result)

if __name__ == '__main__':
    unittest.main()
