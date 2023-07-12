"""QGIS Unit tests for QgsCesiumUtils

From build dir, run: ctest -R QgsCesiumUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '10/07/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA
from qgis.core import (
    QgsBox3d,
    QgsCesiumUtils
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsCesiumUtils(QgisTestCase):

    def test_parse_region(self):
        self.assertTrue(QgsCesiumUtils.parseRegion([]).isNull())
        # invalid length (needs 6 elements)
        self.assertTrue(QgsCesiumUtils.parseRegion([1, 2, 3, 4]).isNull())
        self.assertTrue(QgsCesiumUtils.parseRegion([1, 2, 3, 4, 5, 6, 7]).isNull())
        # not doubles
        self.assertTrue(
            QgsCesiumUtils.parseRegion([1, 'a', 3, 4, 5, 6]).isNull())

        # valid
        box = QgsCesiumUtils.parseRegion([1.2, 2, 3, 4.6, 5.5, 6])
        self.assertEqual(box.xMinimum(), 1.2)
        self.assertEqual(box.xMaximum(), 3.0)
        self.assertEqual(box.yMinimum(), 2.0)
        self.assertEqual(box.yMaximum(), 4.6)
        self.assertEqual(box.zMinimum(), 5.5)
        self.assertEqual(box.zMaximum(), 6.0)


if __name__ == '__main__':
    unittest.main()
