# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS.

From build dir, run: ctest -R PyQgsServerWMS -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '25/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver import QgsServerTestBase
from qgis.core import QgsProject

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetMap(QgsServerTestBase):

    """QGIS Server WMS Tests for GetMap request"""

    def test_wms_getmap_basic_mode(self):
        # 1 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=1bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_1bit", 20000)

        # 8 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=8bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_8bit", 20000)

        # 16 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=16bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_16bit", 20000)

    def test_wms_getmap_basic(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,dem",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic2")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectUseLayerIdsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "country20131022151106556",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic3")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic4")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "sERVICE": "WMS",
            "VeRSION": "1.1.1",
            "REqUEST": "GetMap",
            "LAYeRS": "Country,db_point",
            "STYLeS": "",
            "FORMAt": "image/png",
            "bBOX": "-16817707,-4710778,5696513,14587125",
            "HeIGHT": "500",
            "WIDTH": "500",
            "CRs": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic4")

    def test_wms_getmap_invalid_parameters(self):
        # height should be an int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "FOO",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HEIGHT (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        # width should be an int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "FOO",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"WIDTH (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        # bbox should be formatted like "double,double,double,double"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"BBOX (\'-16817707,-4710778,5696513\') cannot be converted into a rectangle" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,FOO",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"BBOX (\'-16817707,-4710778,5696513,FOO\') cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # test invalid bbox : xmin > xmax
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "1,0,0,1",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # test invalid bbox : ymin > ymax
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "0,1,0,0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # opacities should be a list of int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "OPACITIES": "253,FOO",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"OPACITIES (\'253,FOO\') cannot be converted into a list of int" in r
        self.assertTrue(err)

        # filters should be formatted like "layer0:filter0;layer1:filter1"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country \"name\" = 'eurasia'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"FILTER (\'Country \"name\" = \'eurasia\'\') is not properly formatted" in r
        self.assertTrue(err)

        # selections should be formatted like "layer0:id0,id1;layer1:id0"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "SELECTION": "Country=4"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"SELECTION (\'Country=4\') is not properly formatted" in r
        self.assertTrue(err)

        # invalid highlight geometries
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POLYGONN((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_GEOM (\'POLYGONN((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))\') cannot be converted into a list of geometries" in r
        self.assertTrue(err)

        # invalid highlight label colors
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELCOLOR": "%2300230000;%230023000",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELCOLOR (\'#00230000;#0023000\') cannot be converted into a list of colors" in r
        self.assertTrue(err)

        # invalid list of label sizes
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELSIZE": "16;17;FOO",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELSIZE (\'16;17;FOO\') cannot be converted into a list of int" in r
        self.assertTrue(err)

        # invalid list of label buffer size
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.5;2;FF",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELBUFFERSIZE (\'1.5;2;FF\') cannot be converted into a list of float" in r
        self.assertTrue(err)

        # invalid buffer color
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00;%232300FF0",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELBUFFERCOLOR (\'#2300FF00;#2300FF0\') cannot be converted into a list of colors" in r
        self.assertTrue(err)

    def test_wms_getmap_transparent(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TRANSPARENT": "TRUE"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Transparent")

    def test_wms_getmap_background(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "green"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Background")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "0x008000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Background_Hex")

    def test_wms_getmap_invalid_size(self):
        project = os.path.join(self.testdata_path, "test_project_with_size.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "HEIGHT": "5001",
            "WIDTH": "5000"
        }.items())])

        expected = self.strip_version_xmlns(b'<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Size error">The requested map size is too large</ServiceException>\n</ServiceExceptionReport>\n')
        r, h = self._result(self._execute_request(qs))

        self.assertEqual(self.strip_version_xmlns(r), expected)

    def test_wms_getmap_order(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello,Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_LayerOrder")

    def test_wms_getmap_srs(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SRS")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SRS")

    def test_wms_getmap_style(self):
        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleDefault")

        # custom style with STYLES parameter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLES": "custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleCustom")

        # custom style with STYLE parameter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLE": "custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleCustom")

    def test_wms_getmap_filter(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country:\"name\" = 'eurasia'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter")

        # try to display a feature yet filtered by the project
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country:\"name\" = 'africa'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter2")

        # display all features to check that initial filter is restored
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter3")

    def test_wms_getmap_filter_ogc(self):
        filter = "<Filter><PropertyIsEqualTo><PropertyName>name</PropertyName>" + \
                 "<Literal>eurasia</Literal></PropertyIsEqualTo></Filter>"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC")

    def test_wms_getmap_selection(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "SELECTION": "Country: 4"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Selection")

    def test_wms_getmap_diagrams(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Diagrams,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Diagrams")

    def test_wms_getmap_opacities(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "OPACITIES": "125, 50"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello,dem",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "OPACITIES": "125,50,150"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities2")

    def test_wms_getmap_highlight(self):
        # highlight layer with color separated from sld
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POLYGON((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))",
            "HIGHLIGHT_SYMBOL": "<StyledLayerDescriptor><UserStyle><Name>Highlight</Name><FeatureTypeStyle><Rule><Name>Symbol</Name><LineSymbolizer><Stroke><SvgParameter name=\"stroke\">%23ea1173</SvgParameter><SvgParameter name=\"stroke-opacity\">1</SvgParameter><SvgParameter name=\"stroke-width\">1.6</SvgParameter></Stroke></LineSymbolizer></Rule></FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "HIGHLIGHT_LABELSTRING": "Highlight Layer!",
            "HIGHLIGHT_LABELSIZE": "16",
            "HIGHLIGHT_LABELCOLOR": "%2300FF0000",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.5",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Highlight")

    def test_wms_getmap_annotations(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectAnnotationPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Annotations")

    def test_wms_getmap_sld(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "REQUEST": "GetMap",
            "VERSION": "1.1.1",
            "SERVICE": "WMS",
            "SLD": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>db_point</se:Name> <UserStyle> <se:Name>db_point_style</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\"> <ogc:PropertyIsEqualTo> <ogc:PropertyName>gid</ogc:PropertyName> <ogc:Literal>1</ogc:Literal> </ogc:PropertyIsEqualTo> </ogc:Filter> <se:PointSymbolizer uom=\"http://www.opengeospatial.org/se/units/metre\"> <se:Graphic> <se:Mark> <se:WellKnownName>square</se:WellKnownName> <se:Fill> <se:SvgParameter name=\"fill\">5e86a1</se:SvgParameter> </se:Fill> <se:Stroke> <se:SvgParameter name=\"stroke\">000000</se:SvgParameter> </se:Stroke> </se:Mark> <se:Size>0.007</se:Size> </se:Graphic> </se:PointSymbolizer> </se:Rule> </se:FeatureTypeStyle> </UserStyle> </NamedLayer> </StyledLayerDescriptor>",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "WIDTH": "500",
            "HEIGHT": "500",
            "LAYERS": "db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLD")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

    def test_wms_getmap_group(self):
        """A WMS shall render the requested layers by drawing the leftmost in the list 
        bottommost, the next one over that, and so on."""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Diagrams,Country_Labels,Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_individual, _ = self._result(self._execute_request(qs))

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "CountryGroup",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_group, _ = self._result(self._execute_request(qs))

        """ Debug check:
        f = open('grouped.png', 'wb+')
        f.write(r_group)
        f.close()
        f = open('individual.png', 'wb+')
        f.write(r_individual)
        f.close()
        #"""

        self.assertEqual(r_individual, r_group, 'Individual layers query and group layers query results should be identical')

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "SLD": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>CountryGroup</se:Name></NamedLayer> </StyledLayerDescriptor>",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_group_sld, _ = self._result(self._execute_request(qs))
        self.assertEqual(r_individual, r_group_sld, 'Individual layers query and SLD group layers query results should be identical')


if __name__ == '__main__':
    unittest.main()
