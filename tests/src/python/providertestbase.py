# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import str
from builtins import object
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-27'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import (
    QgsRectangle,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    QgsAbstractFeatureIterator,
    QgsExpressionContextScope,
    QgsExpressionContext,
    QgsVectorDataProvider,
    QgsVectorLayerFeatureSource,
    QgsFeatureSink,
    QgsTestUtils,
    NULL
)

from utilities import compareWkt
from featuresourcetestbase import FeatureSourceTestCase


class ProviderTestCase(FeatureSourceTestCase):

    '''
        This is a collection of tests for vector data providers and kept generic.
        To make use of it, subclass it and set self.source to a provider you want to test.
        Make sure that your provider uses the default dataset by converting one of the provided datasets from the folder
        tests/testdata/provider to a dataset your provider is able to handle.

        To test expression compilation, add the methods `enableCompiler()` and `disableCompiler()` to your subclass.
        If these methods are present, the tests will ensure that the result of server side and client side expression
        evaluation are equal.
    '''

    def uncompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        cannot be compiled """
        return set()

    def partiallyCompiledFilters(self):
        """ Individual derived provider tests should override this to return a list of expressions which
        should be partially compiled """
        return set()

    def assert_query(self, source, expression, expected):
        FeatureSourceTestCase.assert_query(self, source, expression, expected)

        if self.compiled:
            # Check compilation status
            it = source.getFeatures(QgsFeatureRequest().setFilterExpression(expression))

            if expression in self.uncompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.NoCompilation)
            elif expression in self.partiallyCompiledFilters():
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.PartiallyCompiled)
            else:
                self.assertEqual(it.compileStatus(), QgsAbstractFeatureIterator.Compiled)

    def runGetFeatureTests(self, source):
        FeatureSourceTestCase.runGetFeatureTests(self, source)

        # combination of an uncompilable expression and limit
        feature = next(self.vl.getFeatures('pk=4'))
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('parent', feature)
        context.appendScope(scope)

        request = QgsFeatureRequest()
        request.setExpressionContext(context)
        request.setFilterExpression('"pk" = attribute(@parent, \'pk\')')
        request.setLimit(1)

        values = [f['pk'] for f in self.vl.getFeatures(request)]
        self.assertEqual(values, [4])

    def runPolyGetFeatureTests(self, provider):
        assert len([f for f in provider.getFeatures()]) == 4

        # geometry
        self.assert_query(provider, 'x($geometry) < -70', [1])
        self.assert_query(provider, 'y($geometry) > 79', [1, 2])
        self.assert_query(provider, 'xmin($geometry) < -70', [1, 3])
        self.assert_query(provider, 'ymin($geometry) < 76', [3])
        self.assert_query(provider, 'xmax($geometry) > -68', [2, 3])
        self.assert_query(provider, 'ymax($geometry) > 80', [1, 2])
        self.assert_query(provider, 'area($geometry) > 10', [1])
        self.assert_query(provider, 'perimeter($geometry) < 12', [2, 3])
        self.assert_query(provider, 'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'', [1, 3])
        self.assert_query(provider, 'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')', [1, 3])
        self.assert_query(provider, 'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))', [2])
        self.assert_query(provider, 'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))', [2])
        self.assert_query(provider, 'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))', [1])
        self.assert_query(provider, 'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))', [1])
        self.assert_query(provider, 'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))', [1, 3])
        self.assert_query(provider, 'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))', [2])
        self.assert_query(provider, 'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))', [1, 2])
        self.assert_query(provider, 'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7', [1, 2])

    def testGetFeaturesUncompiled(self):
        self.compiled = False
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runGetFeatureTests(self.source)
        if hasattr(self, 'poly_provider'):
            self.runPolyGetFeatureTests(self.poly_provider)

    def testGetFeaturesExp(self):
        try:
            self.enableCompiler()
            self.compiled = True
            self.runGetFeatureTests(self.source)
            if hasattr(self, 'poly_provider'):
                self.runPolyGetFeatureTests(self.poly_provider)
        except AttributeError:
            print('Provider does not support compiling')

    def testSubsetString(self):
        if not self.source.supportsSubsetString():
            print('Provider does not support subset strings')
            return

        subset = self.getSubsetString()
        self.source.setSubsetString(subset)
        self.assertEqual(self.source.subsetString(), subset)
        result = set([f['pk'] for f in self.source.getFeatures()])
        all_valid = (all(f.isValid() for f in self.source.getFeatures()))
        self.source.setSubsetString(None)

        expected = set([2, 3, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect
        self.source.setSubsetString(subset)
        extent = QgsRectangle(-70, 70, -60, 75)
        request = QgsFeatureRequest().setFilterRect(extent)
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([2])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

        # Subset string AND filter rect, version 2
        self.source.setSubsetString(subset)
        extent = QgsRectangle(-71, 65, -60, 80)
        result = set([f['pk'] for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        self.source.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)

        # Subset string AND expression
        self.source.setSubsetString(subset)
        request = QgsFeatureRequest().setFilterExpression('length("name")=5')
        result = set([f['pk'] for f in self.source.getFeatures(request)])
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        self.source.setSubsetString(None)
        expected = set([2, 4])
        assert set(expected) == result, 'Expected {} and got {} when testing subset string {}'.format(set(expected), result, subset)
        self.assertTrue(all_valid)

    def getSubsetString(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 410'

    def getSubsetString2(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"cnt" > 100 and "cnt" < 400'

    def getSubsetString3(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"name"=\'Apple\''

    def getSubsetStringNoMatching(self):
        """Individual providers may need to override this depending on their subset string formats"""
        return '"name"=\'AppleBearOrangePear\''

    def testGetFeaturesThreadSafety(self):
        # no request
        self.assertTrue(QgsTestUtils.testProviderIteratorThreadSafety(self.source))

        # filter rect request
        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        self.assertTrue(QgsTestUtils.testProviderIteratorThreadSafety(self.source, request))

    def testOrderBy(self):
        try:
            self.disableCompiler()
        except AttributeError:
            pass
        self.runOrderByTests()

    def testOrderByCompiled(self):
        try:
            self.enableCompiler()
            self.runOrderByTests()
        except AttributeError:
            print('Provider does not support compiling')

    def runOrderByTests(self):
        FeatureSourceTestCase.runOrderByTests(self)

        # Combination with subset of attributes
        request = QgsFeatureRequest().addOrderBy('num_char', False).setSubsetOfAttributes(['pk'], self.vl.fields())
        values = [f['pk'] for f in self.vl.getFeatures(request)]
        self.assertEqual(values, [5, 4, 3, 2, 1])

    def testOpenIteratorAfterLayerRemoval(self):
        """
        Test that removing layer after opening an iterator does not crash. All required
        information should be captured in the iterator's source and there MUST be no
        links between the iterators and the layer's data provider
        """
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # store the source
        source = QgsVectorLayerFeatureSource(l)

        # delete the layer
        del l

        # get the features
        pks = []
        for f in source.getFeatures():
            pks.append(f['pk'])
        self.assertEqual(set(pks), {1, 2, 3, 4, 5})

    def testGetFeaturesPolyFilterRectTests(self):
        """ Test fetching features from a polygon layer with filter rect"""
        try:
            if not self.poly_provider:
                return
        except:
            return

        extent = QgsRectangle(-73, 70, -63, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f['pk'] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        # Some providers may return the exact intersection matches (2, 3) even without the ExactIntersect flag, so we accept that too
        assert set(features) == set([2, 3]) or set(features) == set([1, 2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # Test with exact intersection
        request = QgsFeatureRequest().setFilterRect(extent).setFlags(QgsFeatureRequest.ExactIntersect)
        features = [f['pk'] for f in self.poly_provider.getFeatures(request)]
        all_valid = (all(f.isValid() for f in self.source.getFeatures(request)))
        assert set(features) == set([2, 3]), 'Got {} instead'.format(features)
        self.assertTrue(all_valid)

        # test with an empty rectangle
        extent = QgsRectangle()
        features = [f['pk'] for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))]
        assert set(features) == set([1, 2, 3, 4, 5]), 'Got {} instead'.format(features)

    def testMinValue(self):
        self.assertEqual(self.source.minimumValue(1), -200)
        self.assertEqual(self.source.minimumValue(2), 'Apple')

        subset = self.getSubsetString()
        self.source.setSubsetString(subset)
        min_value = self.source.minimumValue(1)
        self.source.setSubsetString(None)
        self.assertEqual(min_value, 200)

    def testMaxValue(self):
        self.assertEqual(self.source.maximumValue(1), 400)
        self.assertEqual(self.source.maximumValue(2), 'Pear')

        subset = self.getSubsetString2()
        self.source.setSubsetString(subset)
        max_value = self.source.maximumValue(1)
        self.source.setSubsetString(None)
        self.assertEqual(max_value, 300)

    def testExtent(self):
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        provider_extent = self.source.extent()
        self.assertAlmostEqual(provider_extent.xMinimum(), -71.123, 3)
        self.assertAlmostEqual(provider_extent.xMaximum(), -65.32, 3)
        self.assertAlmostEqual(provider_extent.yMinimum(), 66.33, 3)
        self.assertAlmostEqual(provider_extent.yMaximum(), 78.3, 3)

        # with only one point
        subset = self.getSubsetString3()
        self.source.setSubsetString(subset)
        count = self.source.featureCount()
        provider_extent = self.source.extent()
        self.source.setSubsetString(None)
        self.assertEqual(count, 1)
        self.assertAlmostEqual(provider_extent.xMinimum(), -68.2, 3)
        self.assertAlmostEqual(provider_extent.xMaximum(), -68.2, 3)
        self.assertAlmostEqual(provider_extent.yMinimum(), 70.8, 3)
        self.assertAlmostEqual(provider_extent.yMaximum(), 70.8, 3)

        # with no points
        subset = self.getSubsetStringNoMatching()
        self.source.setSubsetString(subset)
        count = self.source.featureCount()
        provider_extent = self.source.extent()
        self.source.setSubsetString(None)
        self.assertEqual(count, 0)
        self.assertTrue(provider_extent.isNull())

    def testUnique(self):
        self.assertEqual(set(self.source.uniqueValues(1)), set([-200, 100, 200, 300, 400]))
        assert set(['Apple', 'Honey', 'Orange', 'Pear', NULL]) == set(self.source.uniqueValues(2)), 'Got {}'.format(set(self.source.uniqueValues(2)))

        subset = self.getSubsetString2()
        self.source.setSubsetString(subset)
        values = self.source.uniqueValues(1)
        self.source.setSubsetString(None)
        self.assertEqual(set(values), set([200, 300]))

    def testUniqueStringsMatching(self):
        self.assertEqual(set(self.source.uniqueStringsMatching(2, 'a')), set(['Pear', 'Orange', 'Apple']))
        # test case insensitive
        self.assertEqual(set(self.source.uniqueStringsMatching(2, 'A')), set(['Pear', 'Orange', 'Apple']))
        # test string ending in substring
        self.assertEqual(set(self.source.uniqueStringsMatching(2, 'ney')), set(['Honey']))
        # test limit
        result = set(self.source.uniqueStringsMatching(2, 'a', 2))
        self.assertEqual(len(result), 2)
        self.assertTrue(result.issubset(set(['Pear', 'Orange', 'Apple'])))

        assert set([u'Apple', u'Honey', u'Orange', u'Pear', NULL]) == set(self.source.uniqueValues(2)), 'Got {}'.format(set(self.source.uniqueValues(2)))

        subset = self.getSubsetString2()
        self.source.setSubsetString(subset)
        values = self.source.uniqueStringsMatching(2, 'a')
        self.source.setSubsetString(None)
        self.assertEqual(set(values), set(['Pear', 'Apple']))

    def testFeatureCount(self):
        self.assertEqual(self.source.featureCount(), 5)

        #Add a subset string and test feature count
        subset = self.getSubsetString()
        self.source.setSubsetString(subset)
        count = self.source.featureCount()
        self.source.setSubsetString(None)
        self.assertEqual(count, 3)
        self.assertEqual(self.source.featureCount(), 5)

        # one matching records
        subset = self.getSubsetString3()
        self.source.setSubsetString(subset)
        count = self.source.featureCount()
        self.source.setSubsetString(None)
        self.assertEqual(count, 1)
        self.assertEqual(self.source.featureCount(), 5)

        # no matching records
        subset = self.getSubsetStringNoMatching()
        self.source.setSubsetString(subset)
        count = self.source.featureCount()
        self.source.setSubsetString(None)
        self.assertEqual(count, 0)
        self.assertEqual(self.source.featureCount(), 5)

    def testGetFeaturesNoGeometry(self):
        """ Test that no geometry is present when fetching features without geometry"""

        for f in self.source.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)):
            self.assertFalse(f.hasGeometry(), 'Expected no geometry, got one')
            self.assertTrue(f.isValid())

    def testAddFeature(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        f1 = QgsFeature()
        f1.setAttributes([6, -220, NULL, 'String', '15'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))

        f2 = QgsFeature()
        f2.setAttributes([7, 330, 'Coconut', 'CoCoNut', '13'])

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            # expect success
            result, added = l.dataProvider().addFeatures([f1, f2])
            self.assertTrue(result, 'Provider reported AddFeatures capability, but returned False to addFeatures')
            f1.setId(added[0].id())
            f2.setId(added[1].id())

            # check result
            self.testGetFeatures(l.dataProvider(), [f1, f2])

            # add empty list, should return true for consistency
            self.assertTrue(l.dataProvider().addFeatures([]))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().addFeatures([f1, f2]), 'Provider reported no AddFeatures capability, but returned true to addFeatures')

    def testAddFeatureFastInsert(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        f1 = QgsFeature()
        f1.setAttributes([6, -220, NULL, 'String', '15'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))

        f2 = QgsFeature()
        f2.setAttributes([7, 330, 'Coconut', 'CoCoNut', '13'])

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            # expect success
            result, added = l.dataProvider().addFeatures([f1, f2], QgsFeatureSink.FastInsert)
            self.assertTrue(result, 'Provider reported AddFeatures capability, but returned False to addFeatures')
            self.assertEqual(l.dataProvider().featureCount(), 7)

    def testAddFeaturesUpdateExtent(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -65.3,78.3')

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            f1 = QgsFeature()
            f1.setAttributes([6, -220, NULL, 'String', '15'])
            f1.setGeometry(QgsGeometry.fromWkt('Point (-50 90)'))
            l.dataProvider().addFeatures([f1])

            l.dataProvider().updateExtents()
            self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -50.0,90.0')

    def testDeleteFeatures(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        #find 2 features to delete
        features = [f for f in l.dataProvider().getFeatures()]
        to_delete = [f.id() for f in features if f.attributes()[0] in [1, 3]]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            # expect success
            result = l.dataProvider().deleteFeatures(to_delete)
            self.assertTrue(result, 'Provider reported DeleteFeatures capability, but returned False to deleteFeatures')

            # check result
            self.testGetFeatures(l.dataProvider(), skip_features=[1, 3])

            # delete empty list, should return true for consistency
            self.assertTrue(l.dataProvider().deleteFeatures([]))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().deleteFeatures(to_delete),
                             'Provider reported no DeleteFeatures capability, but returned true to deleteFeatures')

    def testDeleteFeaturesUpdateExtent(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().extent().toString(1), '-71.1,66.3 : -65.3,78.3')

        to_delete = [f.id() for f in l.dataProvider().getFeatures() if f.attributes()[0] in [5, 4]]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            l.dataProvider().deleteFeatures(to_delete)

            l.dataProvider().updateExtents()
            self.assertEqual(l.dataProvider().extent().toString(1), '-70.3,66.3 : -68.2,70.8')

    def testTruncate(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        features = [f['pk'] for f in l.dataProvider().getFeatures()]

        if l.dataProvider().capabilities() & QgsVectorDataProvider.FastTruncate or l.dataProvider().capabilities() & QgsVectorDataProvider.DeleteFeatures:
            # expect success
            result = l.dataProvider().truncate()
            self.assertTrue(result, 'Provider reported FastTruncate or DeleteFeatures capability, but returned False to truncate()')

            # check result
            features = [f['pk'] for f in l.dataProvider().getFeatures()]
            self.assertEqual(len(features), 0)
        else:
            # expect fail
            self.assertFalse(l.dataProvider().truncate(),
                             'Provider reported no FastTruncate or DeleteFeatures capability, but returned true to truncate()')

    def testChangeAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        #find 2 features to change
        features = [f for f in l.dataProvider().getFeatures()]
        # need to keep order here
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeAttributeValues call
        changes = {to_change[0].id(): {1: 501, 3: 'new string'}, to_change[1].id(): {1: 502, 4: 'NEW'}}
        # changes by pk, for testing after retrieving changed features
        new_attr_map = {1: {1: 501, 3: 'new string'}, 3: {1: 502, 4: 'NEW'}}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect success
            result = l.dataProvider().changeAttributeValues(changes)
            self.assertTrue(result, 'Provider reported ChangeAttributeValues capability, but returned False to changeAttributeValues')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_attributes=new_attr_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeAttributeValues({}))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().changeAttributeValues(changes),
                             'Provider reported no ChangeAttributeValues capability, but returned true to changeAttributeValues')

    def testChangeGeometries(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        # find 2 features to change
        features = [f for f in l.dataProvider().getFeatures()]
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeGeometryValues call
        changes = {to_change[0].id(): QgsGeometry.fromWkt('Point (10 20)'), to_change[1].id(): QgsGeometry()}
        # changes by pk, for testing after retrieving changed features
        new_geom_map = {1: QgsGeometry.fromWkt('Point ( 10 20 )'), 3: QgsGeometry()}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries:
            # expect success
            result = l.dataProvider().changeGeometryValues(changes)
            self.assertTrue(result,
                            'Provider reported ChangeGeometries capability, but returned False to changeGeometryValues')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_geometries=new_geom_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeGeometryValues({}))

        else:
            # expect fail
            self.assertFalse(l.dataProvider().changeGeometryValues(changes),
                             'Provider reported no ChangeGeometries capability, but returned true to changeGeometryValues')

    def testChangeFeatures(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        features = [f for f in l.dataProvider().getFeatures()]

        # find 2 features to change attributes for
        features = [f for f in l.dataProvider().getFeatures()]
        # need to keep order here
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 2])
        # changes by feature id, for changeAttributeValues call
        attribute_changes = {to_change[0].id(): {1: 501, 3: 'new string'}, to_change[1].id(): {1: 502, 4: 'NEW'}}
        # changes by pk, for testing after retrieving changed features
        new_attr_map = {1: {1: 501, 3: 'new string'}, 2: {1: 502, 4: 'NEW'}}

        # find 2 features to change geometries for
        to_change = [f for f in features if f.attributes()[0] == 1]
        to_change.extend([f for f in features if f.attributes()[0] == 3])
        # changes by feature id, for changeGeometryValues call
        geometry_changes = {to_change[0].id(): QgsGeometry.fromWkt('Point (10 20)'), to_change[1].id(): QgsGeometry()}
        # changes by pk, for testing after retrieving changed features
        new_geom_map = {1: QgsGeometry.fromWkt('Point ( 10 20 )'), 3: QgsGeometry()}

        if l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries and l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect success
            result = l.dataProvider().changeFeatures(attribute_changes, geometry_changes)
            self.assertTrue(result,
                            'Provider reported ChangeGeometries and ChangeAttributeValues capability, but returned False to changeFeatures')

            # check result
            self.testGetFeatures(l.dataProvider(), changed_attributes=new_attr_map, changed_geometries=new_geom_map)

            # change empty list, should return true for consistency
            self.assertTrue(l.dataProvider().changeFeatures({}, {}))

        elif not l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeGeometries:
            # expect fail
            self.assertFalse(l.dataProvider().changeFeatures(attribute_changes, geometry_changes),
                             'Provider reported no ChangeGeometries capability, but returned true to changeFeatures')
        elif not l.dataProvider().capabilities() & QgsVectorDataProvider.ChangeAttributeValues:
            # expect fail
            self.assertFalse(l.dataProvider().changeFeatures(attribute_changes, geometry_changes),
                             'Provider reported no ChangeAttributeValues capability, but returned true to changeFeatures')
