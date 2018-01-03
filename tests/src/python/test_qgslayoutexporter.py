# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutExporter

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/12/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sip
import tempfile
import shutil
import os
import subprocess

from qgis.core import (QgsMultiRenderChecker,
                       QgsLayoutExporter,
                       QgsLayout,
                       QgsProject,
                       QgsMargins,
                       QgsLayoutItemShape,
                       QgsLayoutGuide,
                       QgsRectangle,
                       QgsLayoutItemPage,
                       QgsLayoutItemMap,
                       QgsLayoutPoint,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsSimpleFillSymbolLayer,
                       QgsFillSymbol)
from qgis.PyQt.QtCore import QSize, QSizeF, QDir, QRectF, Qt
from qgis.PyQt.QtGui import QImage, QPainter
from qgis.PyQt.QtSvg import QSvgRenderer, QSvgGenerator

from qgis.testing import start_app, unittest

from utilities import getExecutablePath

# PDF-to-image utility
# look for Poppler w/ Cairo, then muPDF
# * Poppler w/ Cairo renders correctly
# * Poppler w/o Cairo does not always correctly render vectors in PDF to image
# * muPDF renders correctly, but sightly shifts colors
for util in [
    'pdftocairo',
    # 'mudraw',
]:
    PDFUTIL = getExecutablePath(util)
    if PDFUTIL:
        break

# noinspection PyUnboundLocalVariable
if not PDFUTIL:
    raise Exception('PDF-to-image utility not found on PATH: '
                    'install Poppler (with Cairo)')


def pdfToPng(pdf_file_path, rendered_file_path, page, dpi=96):
    if PDFUTIL.strip().endswith('pdftocairo'):
        filebase = os.path.join(
            os.path.dirname(rendered_file_path),
            os.path.splitext(os.path.basename(rendered_file_path))[0]
        )
        call = [
            PDFUTIL, '-png', '-singlefile', '-r', str(dpi),
            '-x', '0', '-y', '0', '-f', str(page), '-l', str(page),
            pdf_file_path, filebase
        ]
    elif PDFUTIL.strip().endswith('mudraw'):
        call = [
            PDFUTIL, '-c', 'rgba',
            '-r', str(dpi), '-f', str(page), '-l', str(page),
            # '-b', '8',
            '-o', rendered_file_path, pdf_file_path
        ]
    else:
        return False, ''

    print("exportToPdf call: {0}".format(' '.join(call)))
    try:
        subprocess.check_call(call)
    except subprocess.CalledProcessError as e:
        assert False, ("exportToPdf failed!\n"
                       "cmd: {0}\n"
                       "returncode: {1}\n"
                       "message: {2}".format(e.cmd, e.returncode, e.message))


def svgToPng(svg_file_path, rendered_file_path, width):
    svgr = QSvgRenderer(svg_file_path)

    height = width / svgr.viewBoxF().width() * svgr.viewBoxF().height()

    image = QImage(width, height, QImage.Format_ARGB32)
    image.fill(Qt.transparent)

    p = QPainter(image)
    p.setRenderHint(QPainter.Antialiasing, False)
    svgr.render(p)
    p.end()

    res = image.save(rendered_file_path, 'png')
    if not res:
        os.unlink(rendered_file_path)


start_app()

project_instance = QgsProject()

class TestQgsLayoutExporter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.basetestpath = tempfile.mkdtemp()
        cls.dots_per_meter = 96 / 25.4 * 1000

    def setUp(self):
        self.report = "<h1>Python QgsLayoutExporter Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def checkImage(self, name, reference_image, rendered_image, size_tolerance=0):
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("layout_exporter")
        checker.setControlName("expected_layoutexporter_" + reference_image)
        checker.setRenderedImage(rendered_image)
        checker.setColorTolerance(2)
        checker.setSizeTolerance(size_tolerance, size_tolerance)
        result = checker.runTest(name, 20)
        self.report += checker.report()
        print((self.report))
        return result

    def testRenderPage(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        # get width/height, create image and render the composition to it
        size = QSize(1122, 794)
        output_image = QImage(size, QImage.Format_RGB32)

        output_image.setDotsPerMeterX(self.dots_per_meter)
        output_image.setDotsPerMeterY(self.dots_per_meter)
        QgsMultiRenderChecker.drawBackground(output_image)
        painter = QPainter(output_image)
        exporter = QgsLayoutExporter(l)

        # valid page
        exporter.renderPage(painter, 0)
        painter.end()

        rendered_file_path = os.path.join(self.basetestpath, 'test_renderpage.png')
        output_image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('renderpage', 'renderpage', rendered_file_path))

    def testRenderPageToImage(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        exporter = QgsLayoutExporter(l)
        size = QSize(1122, 794)

        # bad page numbers
        image = exporter.renderPageToImage(-1, size)
        self.assertTrue(image.isNull())
        image = exporter.renderPageToImage(1, size)
        self.assertTrue(image.isNull())

        # good page
        image = exporter.renderPageToImage(0, size)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimagepage.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimagepage', 'rendertoimagepage', rendered_file_path))

    def testRenderRegion(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add a guide, to ensure it is not included in export
        g1 = QgsLayoutGuide(Qt.Horizontal, QgsLayoutMeasurement(15, QgsUnitTypes.LayoutMillimeters), l.pageCollection().page(0))
        l.guides().addGuide(g1)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        # get width/height, create image and render the composition to it
        size = QSize(560, 509)
        output_image = QImage(size, QImage.Format_RGB32)

        output_image.setDotsPerMeterX(self.dots_per_meter)
        output_image.setDotsPerMeterY(self.dots_per_meter)
        QgsMultiRenderChecker.drawBackground(output_image)
        painter = QPainter(output_image)
        exporter = QgsLayoutExporter(l)

        exporter.renderRegion(painter, QRectF(5, 10, 110, 100))
        painter.end()

        rendered_file_path = os.path.join(self.basetestpath, 'test_renderregion.png')
        output_image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('renderregion', 'renderregion', rendered_file_path))

    def testRenderRegionToImage(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        exporter = QgsLayoutExporter(l)
        size = QSize(560, 509)

        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100), size)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregionsize.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregionsize', 'rendertoimageregionsize', rendered_file_path))

        # using layout dpi
        l.context().setDpi(40)
        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100))
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregiondpi.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregiondpi', 'rendertoimageregiondpi', rendered_file_path))

        # overriding dpi
        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100), QSize(), 80)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregionoverridedpi.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregionoverridedpi', 'rendertoimageregionoverridedpi', rendered_file_path))

    def testExportToImage(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagedpi.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertTrue(self.checkImage('exporttoimagedpi_page1', 'exporttoimagedpi_page1', rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagedpi_2.png')
        self.assertTrue(self.checkImage('exporttoimagedpi_page2', 'exporttoimagedpi_page2', page2_path))

        # crop to contents
        settings.cropToContents = True
        settings.cropMargins = QgsMargins(10, 20, 30, 40)

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagecropped.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertTrue(self.checkImage('exporttoimagecropped_page1', 'exporttoimagecropped_page1', rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagecropped_2.png')
        self.assertTrue(self.checkImage('exporttoimagecropped_page2', 'exporttoimagecropped_page2', page2_path))

        # specific pages
        settings.cropToContents = False
        settings.pages = [1]

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagepages.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertFalse(os.path.exists(rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagepages_2.png')
        self.assertTrue(self.checkImage('exporttoimagedpi_page2', 'exporttoimagedpi_page2', page2_path))

        # image size
        settings.imageSize = QSize(600, 851)

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagesize.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagesize_2.png')
        self.assertTrue(self.checkImage('exporttoimagesize_page2', 'exporttoimagesize_page2', page2_path))

    def testExportToPdf(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False

        pdf_file_path = os.path.join(self.basetestpath, 'test_exporttopdfdpi.pdf')
        self.assertEqual(exporter.exportToPdf(pdf_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttopdfdpi.png')
        dpi = 80
        pdfToPng(pdf_file_path, rendered_page_1, dpi=dpi, page=1)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttopdfdpi2.png')
        pdfToPng(pdf_file_path, rendered_page_2, dpi=dpi, page=2)

        self.assertTrue(self.checkImage('exporttopdfdpi_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttopdfdpi_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

    def testExportToSvg(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.SvgExportSettings()
        settings.dpi = 80
        settings.forceVectorOutput = False

        svg_file_path = os.path.join(self.basetestpath, 'test_exporttosvgdpi.svg')
        svg_file_path_2 = os.path.join(self.basetestpath, 'test_exporttosvgdpi_2.svg')
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))
        self.assertTrue(os.path.exists(svg_file_path_2))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttosvgdpi.png')
        svgToPng(svg_file_path, rendered_page_1, width=936)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttosvgdpi2.png')
        svgToPng(svg_file_path_2, rendered_page_2, width=467)

        self.assertTrue(self.checkImage('exporttosvgdpi_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttosvgdpi_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

        # layered
        settings.exportAsLayers = True

        svg_file_path = os.path.join(self.basetestpath, 'test_exporttosvglayered.svg')
        svg_file_path_2 = os.path.join(self.basetestpath, 'test_exporttosvglayered_2.svg')
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))
        self.assertTrue(os.path.exists(svg_file_path_2))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttosvglayered.png')
        svgToPng(svg_file_path, rendered_page_1, width=936)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttosvglayered2.png')
        svgToPng(svg_file_path_2, rendered_page_2, width=467)

        self.assertTrue(self.checkImage('exporttosvglayered_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttosvglayered_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

    def testExportWorldFile(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()

        # add some items
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(30, 60, 200, 100))
        extent = QgsRectangle(2000, 2800, 2500, 2900)
        map.setExtent(extent)
        l.addLayoutItem(map)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80
        settings.generateWorldFile = False

        rendered_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.png')
        world_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.pgw')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(rendered_file_path))
        self.assertFalse(os.path.exists(world_file_path))

        # with world file
        settings.generateWorldFile = True
        rendered_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(rendered_file_path))
        self.assertTrue(os.path.exists(world_file_path))

        lines = tuple(open(world_file_path, 'r'))
        values = [float(f) for f in lines]
        self.assertAlmostEqual(values[0], 0.794117647059, 2)
        self.assertAlmostEqual(values[1], 0.0, 2)
        self.assertAlmostEqual(values[2], 0.0, 2)
        self.assertAlmostEqual(values[3], -0.794251134644, 2)
        self.assertAlmostEqual(values[4], 1925.000000000000, 2)
        self.assertAlmostEqual(values[5], 3050.000000000000, 2)

    def testExcludePagesImage(self):
        l = QgsLayout(project_instance)
        l.initializeDefaults()
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80
        settings.generateWorldFile = False

        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export.png')
        details = QgsLayoutExporter.PageExportDetails()
        details.directory = self.basetestpath
        details.baseName = 'test_exclude_export'
        details.extension = 'png'
        details.page = 0

        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))

        # exclude a page
        l.pageCollection().page(0).setExcludeFromExports(True)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export_excluded.png')
        details.baseName = 'test_exclude_export_excluded'
        details.page = 0
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))

        # exclude second page
        l.pageCollection().page(1).setExcludeFromExports(True)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export_excluded_all.png')
        details.baseName = 'test_exclude_export_excluded_all'
        details.page = 0
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))

    def testPageFileName(self):
        l = QgsLayout(project_instance)
        exporter = QgsLayoutExporter(l)
        details = QgsLayoutExporter.PageExportDetails()
        details.directory = '/tmp/output'
        details.baseName = 'my_maps'
        details.extension = 'png'
        details.page = 0
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps.png')
        details.page = 1
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps_2.png')
        details.page = 2
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps_3.png')


if __name__ == '__main__':
    unittest.main()
