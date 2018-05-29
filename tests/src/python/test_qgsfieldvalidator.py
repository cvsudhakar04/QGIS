# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldValidator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '31/01/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QValidator
from qgis.core import QgsVectorLayer
from qgis.gui import QgsFieldValidator
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


start_app()


class TestQgsFieldValidator(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        testPath = TEST_DATA_DIR + '/' + 'bug_17878.gpkg|layername=bug_17878'
        self.vl = QgsVectorLayer(testPath, "test_data", "ogr")
        assert self.vl.isValid()

    def tearDown(self):
        """Run after each test."""
        pass

    def test_validator(self):
        # Test the double
        """ 
        Expected results from validate
        QValidator::Invalid 0 The string is clearly invalid.
        QValidator::Intermediate 1 The string is a plausible intermediate value.
        QValidator::Acceptable 2 The string is acceptable as a final result; i.e. it is valid.
        """

        double_field = self.vl.fields()[self.vl.fields().indexFromName('double_field')]
        self.assertEqual(double_field.precision(), 0) # this is what the provider reports :(
        self.assertEqual(double_field.length(), 0) # not set
        self.assertEqual(double_field.type(), QVariant.Double)

        validator = QgsFieldValidator(None, double_field, '0.0', '')

        def _test(value, expected):
            ret = validator.validate(value, 0)
            self.assertEqual(ret[0], expected, value)
            if value:
                self.assertEqual(validator.validate('-' + value, 0)[0], expected, '-' + value)
            # Check the decimal comma separator has been properly transformed
            if expected != QValidator.Invalid:
                self.assertEqual(ret[1], value.replace(',', '.'))

        # Valid
        _test('0.1234', QValidator.Acceptable)
        _test('0,1234', QValidator.Acceptable)
        _test('12345.1234e+123', QValidator.Acceptable)
        _test('12345.1234e-123', QValidator.Acceptable)
        _test('12345,1234e+123', QValidator.Acceptable)
        _test('12345,1234e-123', QValidator.Acceptable)
        _test('', QValidator.Acceptable)

        # Out of range
        _test('12345.1234e+823', QValidator.Intermediate)
        _test('12345.1234e-823', QValidator.Intermediate)
        _test('12345,1234e+823', QValidator.Intermediate)
        _test('12345,1234e-823', QValidator.Intermediate)

        # Invalid
        _test('12345-1234', QValidator.Invalid)
        _test('onetwothree', QValidator.Invalid)

        int_field = self.vl.fields()[self.vl.fields().indexFromName('int_field')]
        self.assertEqual(int_field.precision(), 0) # this is what the provider reports :(
        self.assertEqual(int_field.length(), 0) # not set
        self.assertEqual(int_field.type(), QVariant.Int)

        validator = QgsFieldValidator(None, int_field, '0', '')

        # Valid
        _test('0', QValidator.Acceptable)
        _test('1234', QValidator.Acceptable)
        _test('', QValidator.Acceptable)

        # Invalid
        _test('12345-1234', QValidator.Invalid)
        _test('12345.1234', QValidator.Invalid)
        _test('onetwothree', QValidator.Invalid)


if __name__ == '__main__':
    unittest.main()
