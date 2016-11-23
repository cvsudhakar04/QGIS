# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitWithLines.py
    ---------------------
    Date                 : November 2014
    Revised              : November 2016
    Copyright            : (C) 2014 by Bernhard Ströbl
    Email                : bernhard dot stroebl at jena dot de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Bernhard Ströbl'
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Bernhard Ströbl'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import Qgis, QgsFeatureRequest, QgsFeature, QgsGeometry, QgsWkbTypes, QgsSpatialIndex
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.ProcessingLog import ProcessingLog
from processing.tools import dataobjects
from processing.tools import vector


class SplitWithLines(GeoAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'

    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Split with lines')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(self.INPUT_A,
                                          self.tr('Input layer, single geometries only'), [dataobjects.TYPE_VECTOR_POLYGON,
                                          dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterVector(self.INPUT_B,
                                          self.tr('Split layer'), [dataobjects.TYPE_VECTOR_LINE]))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Split')))

    def processAlgorithm(self, progress):
        layerA = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_A))
        splitLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_B))

        sameLayer = self.getParameterValue(self.INPUT_A) == self.getParameterValue(self.INPUT_B)
        fieldList = layerA.fields()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                                                                     layerA.wkbType(), layerA.crs())

        spatialIndex = QgsSpatialIndex()
        splitGeoms = {}
        request = QgsFeatureRequest()
        request.setSubsetOfAttributes([])

        for aSplitFeature in vector.features(splitLayer, request):
            splitGeoms[aSplitFeature.id()] = aSplitFeature.geometry()
            spatialIndex.insertFeature(aSplitFeature)
            # honor the case that user has selection on split layer and has setting "use selection"

        outFeat = QgsFeature()
        features = vector.features(layerA)

        if len(features) == 0:
            total = 100
        else:
            total = 100.0 / float(len(features))

        multiGeoms = 0 # how many multi geometries were encountered

        for current, inFeatA in enumerate(features):
            inGeom = inFeatA.geometry()

            if inGeom.isMultipart():
                multiGeoms += 1
                # MultiGeometries are not allowed because the result of a splitted part cannot be clearly defined:
                # 1) add both new parts as new features
                # 2) store one part as a new feature and the other one as part of the multi geometry
                # 2a) which part should be which, seems arbitrary
            else:
                attrsA = inFeatA.attributes()
                outFeat.setAttributes(attrsA)
                inGeoms = [inGeom]
                lines = spatialIndex.intersects(inGeom.boundingBox())

                if len(lines) > 0:  # has intersection of bounding boxes
                    splittingLines = []

                    for i in lines:
                        try:
                            splitGeom = splitGeoms[i]
                        except:
                            continue

                        # check if trying to self-intersect
                        if sameLayer:
                            if inFeatA.id() == i:
                                continue

                        engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                        engine.prepareGeometry()

                        if engine.intersects(splitGeom.geometry()):
                            splittingLines.append(splitGeom)

                    if len(splittingLines) > 0:
                        for splitGeom in splittingLines:
                            splitterPList = None
                            outGeoms = []

                            while len(inGeoms) > 0:
                                inGeom = inGeoms.pop()
                                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                                engine.prepareGeometry()
                                inPoints = vector.extractPoints(inGeom)

                                if engine.intersects(splitGeom.geometry()):
                                    if splitterPList == None:
                                        splitterPList = vector.extractPoints(splitGeom)

                                    try:
                                        result, newGeometries, topoTestPoints = inGeom.splitGeometry(splitterPList, False)
                                    except:
                                        ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                                               self.tr('Geometry exception while splitting'))
                                        result = 1

                                    # splitGeometry: If there are several intersections
                                    # between geometry and splitLine, only the first one is considered.
                                    if result == 0:  # split occurred

                                        if inPoints == vector.extractPoints(inGeom):
                                            # bug in splitGeometry: sometimes it returns 0 but
                                            # the geometry is unchanged
                                            outGeoms.append(inGeom)
                                        else:
                                            inGeoms.append(inGeom)

                                            for aNewGeom in newGeometries:
                                                inGeoms.append(aNewGeom)
                                    else:
                                        outGeoms.append(inGeom)
                                else:
                                    outGeoms.append(inGeom)

                            inGeoms = outGeoms

                for aGeom in inGeoms:
                    passed = True

                    if aGeom.wkbType == 2 or aGeom.wkbType == -2147483646:
                        passed = len(aGeom.asPolyline()) > 2

                        if not passed:
                            passed = (len(aGeom.asPolyline()) == 2 and
                                 aGeom.asPolyline()[0] != aGeom.asPolyline()[1])
                            # sometimes splitting results in lines of zero length

                    if passed:
                        outFeat.setGeometry(aGeom)
                        writer.addFeature(outFeat)

            progress.setPercentage(int(current * total))

        if multiGeoms > 0:
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                self.tr('Feature geometry error: %s input features ignored due to multi-geometry.') % str(multiGeoms))

        del writer
