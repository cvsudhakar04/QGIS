# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemMap.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 Nyall Dawson'
__date__ = '20/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QFileInfo, QRectF, QDir
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QPainter, QColor

from qgis.core import (QgsLayoutItemMap,
                       QgsRectangle,
                       QgsRasterLayer,
                       QgsVectorLayer,
                       QgsLayout,
                       QgsMapSettings,
                       QgsProject,
                       QgsMultiBandColorRenderer,
                       QgsCoordinateReferenceSystem
                       )

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker
from test_qgslayoutitem import LayoutItemTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()

project_instance = QgsProject()

class TestQgsComposerMap(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemMap

    def setUp(self):
        self.report = "<h1>Python QgsLayoutItemMap Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)
        myPath = os.path.join(TEST_DATA_DIR, 'rgb256x256.png')
        rasterFileInfo = QFileInfo(myPath)
        self.raster_layer = QgsRasterLayer(rasterFileInfo.filePath(),
                                           rasterFileInfo.completeBaseName())
        rasterRenderer = QgsMultiBandColorRenderer(
            self.raster_layer.dataProvider(), 1, 2, 3)
        self.raster_layer.setRenderer(rasterRenderer)

        myPath = os.path.join(TEST_DATA_DIR, 'points.shp')
        vector_file_info = QFileInfo(myPath)
        self.vector_layer = QgsVectorLayer(vector_file_info.filePath(),
                                           vector_file_info.completeBaseName(), 'ogr')
        assert self.vector_layer.isValid()

        # pipe = mRasterLayer.pipe()
        # assert pipe.set(rasterRenderer), 'Cannot set pipe renderer'
        project_instance.addMapLayers([self.raster_layer, self.vector_layer])

        # create composition with composer map
        self.layout = QgsLayout(project_instance)
        self.layout.initializeDefaults()
        self.map = QgsLayoutItemMap(self.layout)
        self.map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        self.map.setFrameEnabled(True)
        self.map.setLayers([self.raster_layer])
        self.layout.addLayoutItem(self.map)

    def testOverviewMap(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setFrameMap(self.map)
        checker = QgsLayoutChecker('composermap_overview', self.layout)
        checker.setColorTolerance(6)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testLayout()
        self.report += checker.report()
        self.layout.removeLayoutItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapBlend(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setFrameMap(self.map)
        overviewMap.overview().setBlendMode(QPainter.CompositionMode_Multiply)
        checker = QgsLayoutChecker('composermap_overview_blending', self.layout)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testLayout()
        self.report += checker.report()
        self.layout.removeLayoutItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapInvert(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setFrameMap(self.map)
        overviewMap.overview().setInverted(True)
        checker = QgsLayoutChecker('composermap_overview_invert', self.layout)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testLayout()
        self.report += checker.report()
        self.layout.removeLayoutItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapCenter(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(192, -288, 320, -224)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setFrameMap(self.map)
        overviewMap.overview().setInverted(False)
        overviewMap.overview().setCentered(True)
        checker = QgsLayoutChecker('composermap_overview_center', self.layout)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testLayout()
        self.report += checker.report()
        self.layout.removeLayoutItem(overviewMap)
        assert myTestResult, myMessage

    def testMapCrs(self):
        # create composition with composer map
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(project_instance)
        layout.initializeDefaults()

        # check that new maps inherit project CRS
        project_instance.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        rectangle = QgsRectangle(-13838977, 2369660, -8672298, 6250909)
        map.setExtent(rectangle)
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        self.assertFalse(map.presetCrs().isValid())

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(map.crs().authid(), 'EPSG:3857')
        self.assertEqual(map.presetCrs().authid(), 'EPSG:3857')

        checker = QgsLayoutChecker('composermap_crs3857', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        self.report += checker.report()
        self.assertTrue(result, message)

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(map.presetCrs().authid(), 'EPSG:4326')
        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        rectangle = QgsRectangle(-124, 17, -78, 52)
        map.zoomToExtent(rectangle)
        checker = QgsLayoutChecker('composermap_crs4326', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        self.report += checker.report()
        self.assertTrue(result, message)

        # change back to project CRS
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        self.assertFalse(map.presetCrs().isValid())

    def testuniqueId(self):
        return
        doc = QDomDocument()
        documentElement = doc.createElement('ComposerItemClipboard')
        self.layout.writeXml(documentElement, doc)
        self.layout.addItemsFromXml(documentElement, doc)

        # test if both composer maps have different ids
        newMap = QgsComposerMap(self.layout, 0, 0, 10, 10)
        mapList = self.layout.composerMapItems()

        for mapIt in mapList:
            if mapIt != self.map:
                newMap = mapIt
                break

        oldId = self.map.id()
        newId = newMap.id()

        self.layout.removeComposerItem(newMap)
        myMessage = 'old: %s new: %s' % (oldId, newId)
        assert oldId != newId, myMessage

    def testContainsAdvancedEffects(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(project_instance)
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.containsAdvancedEffects())
        self.vector_layer.setBlendMode(QPainter.CompositionMode_Darken)
        result = map.containsAdvancedEffects()
        self.vector_layer.setBlendMode(QPainter.CompositionMode_SourceOver)
        self.assertTrue(result)

    def testRasterization(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(project_instance)
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.requiresRasterization())
        self.vector_layer.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertFalse(map.requiresRasterization())
        self.assertTrue(map.containsAdvancedEffects())

        map.setBackgroundEnabled(False)
        self.assertTrue(map.requiresRasterization())
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(1, 1, 1, 1))
        self.assertTrue(map.requiresRasterization())

        self.vector_layer.setBlendMode(QPainter.CompositionMode_SourceOver)

    def testWorldFileGeneration(self):
        return
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.map.setNewExtent(myRectangle)
        self.map.setMapRotation(30.0)

        self.layout.setGenerateWorldFile(True)
        self.layout.setReferenceMap(self.map)

        p = self.layout.computeWorldFileParameters()
        pexpected = (4.180480199790922, 2.4133064516129026, 779443.7612381146,
                     2.4136013686911886, -4.179969388427311, 3342408.5663611)
        ptolerance = (0.001, 0.001, 1, 0.001, 0.001, 1e+03)
        for i in range(0, 6):
            assert abs(p[i] - pexpected[i]) < ptolerance[i]


if __name__ == '__main__':
    unittest.main()
