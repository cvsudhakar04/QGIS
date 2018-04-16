/***************************************************************************
                         qgsnativealgorithms.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnativealgorithms.h"
#include "qgsalgorithmaddincrementalfield.h"
#include "qgsalgorithmassignprojection.h"
#include "qgsalgorithmboundary.h"
#include "qgsalgorithmboundingbox.h"
#include "qgsalgorithmbuffer.h"
#include "qgsalgorithmcentroid.h"
#include "qgsalgorithmclip.h"
#include "qgsalgorithmconvexhull.h"
#include "qgsalgorithmdissolve.h"
#include "qgsalgorithmdropgeometry.h"
#include "qgsalgorithmdropmzvalues.h"
#include "qgsalgorithmexplode.h"
#include "qgsalgorithmextenttolayer.h"
#include "qgsalgorithmextractbyattribute.h"
#include "qgsalgorithmextractbyexpression.h"
#include "qgsalgorithmextractbyextent.h"
#include "qgsalgorithmextractbylocation.h"
#include "qgsalgorithmextractvertices.h"
#include "qgsalgorithmfiledownloader.h"
#include "qgsalgorithmfilter.h"
#include "qgsalgorithmfixgeometries.h"
#include "qgsalgorithmjoinbyattribute.h"
#include "qgsalgorithmjoinwithlines.h"
#include "qgsalgorithmimportphotos.h"
#include "qgsalgorithmlineintersection.h"
#include "qgsalgorithmloadlayer.h"
#include "qgsalgorithmmeancoordinates.h"
#include "qgsalgorithmmergelines.h"
#include "qgsalgorithmmergevector.h"
#include "qgsalgorithmminimumenclosingcircle.h"
#include "qgsalgorithmmultiparttosinglepart.h"
#include "qgsalgorithmmultiringconstantbuffer.h"
#include "qgsalgorithmorderbyexpression.h"
#include "qgsalgorithmorientedminimumboundingbox.h"
#include "qgsalgorithmpackage.h"
#include "qgsalgorithmpointonsurface.h"
#include "qgsalgorithmprojectpointcartesian.h"
#include "qgsalgorithmpromotetomultipart.h"
#include "qgsalgorithmrasterlayeruniquevalues.h"
#include "qgsalgorithmremoveduplicatevertices.h"
#include "qgsalgorithmremoveholes.h"
#include "qgsalgorithmremovenullgeometry.h"
#include "qgsalgorithmrenamelayer.h"
#include "qgsalgorithmrotate.h"
#include "qgsalgorithmsaveselectedfeatures.h"
#include "qgsalgorithmsegmentize.h"
#include "qgsalgorithmsimplify.h"
#include "qgsalgorithmsmooth.h"
#include "qgsalgorithmsnaptogrid.h"
#include "qgsalgorithmsplitwithlines.h"
#include "qgsalgorithmstringconcatenation.h"
#include "qgsalgorithmsubdivide.h"
#include "qgsalgorithmswapxy.h"
#include "qgsalgorithmtransect.h"
#include "qgsalgorithmtransform.h"
#include "qgsalgorithmtranslate.h"
#include "qgsalgorithmuniquevalueindex.h"


///@cond PRIVATE

QgsNativeAlgorithms::QgsNativeAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon QgsNativeAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::id() const
{
  return QStringLiteral( "native" );
}

QString QgsNativeAlgorithms::helpId() const
{
  return QStringLiteral( "qgis" );
}

QString QgsNativeAlgorithms::name() const
{
  return tr( "QGIS (native c++)" );
}

bool QgsNativeAlgorithms::supportsNonFileBasedOutput() const
{
  return true;
}

void QgsNativeAlgorithms::loadAlgorithms()
{
  addAlgorithm( new QgsAddIncrementalFieldAlgorithm() );
  addAlgorithm( new QgsAddUniqueValueIndexAlgorithm() );
  addAlgorithm( new QgsAssignProjectionAlgorithm() );
  addAlgorithm( new QgsBoundaryAlgorithm() );
  addAlgorithm( new QgsBoundingBoxAlgorithm() );
  addAlgorithm( new QgsBufferAlgorithm() );
  addAlgorithm( new QgsCentroidAlgorithm() );
  addAlgorithm( new QgsClipAlgorithm() );
  addAlgorithm( new QgsCollectAlgorithm() );
  addAlgorithm( new QgsConvexHullAlgorithm() );
  addAlgorithm( new QgsDissolveAlgorithm() );
  addAlgorithm( new QgsDropGeometryAlgorithm() );
  addAlgorithm( new QgsDropMZValuesAlgorithm() );
  addAlgorithm( new QgsExplodeAlgorithm() );
  addAlgorithm( new QgsExtentToLayerAlgorithm() );
  addAlgorithm( new QgsExtractByAttributeAlgorithm() );
  addAlgorithm( new QgsExtractByExpressionAlgorithm() );
  addAlgorithm( new QgsExtractByExtentAlgorithm() );
  addAlgorithm( new QgsExtractByLocationAlgorithm() );
  addAlgorithm( new QgsExtractVerticesAlgorithm() );
  addAlgorithm( new QgsFileDownloaderAlgorithm() );
  addAlgorithm( new QgsFilterAlgorithm() );
  addAlgorithm( new QgsFixGeometriesAlgorithm() );
  addAlgorithm( new QgsImportPhotosAlgorithm() );
  addAlgorithm( new QgsJoinByAttributeAlgorithm() );
  addAlgorithm( new QgsJoinWithLinesAlgorithm() );
  addAlgorithm( new QgsLineIntersectionAlgorithm() );
  addAlgorithm( new QgsLoadLayerAlgorithm() );
  addAlgorithm( new QgsMeanCoordinatesAlgorithm() );
  addAlgorithm( new QgsMergeLinesAlgorithm() );
  addAlgorithm( new QgsMergeVectorAlgorithm() );
  addAlgorithm( new QgsMinimumEnclosingCircleAlgorithm() );
  addAlgorithm( new QgsMultipartToSinglepartAlgorithm() );
  addAlgorithm( new QgsMultiRingConstantBufferAlgorithm() );
  addAlgorithm( new QgsOrderByExpressionAlgorithm() );
  addAlgorithm( new QgsOrientedMinimumBoundingBoxAlgorithm() );
  addAlgorithm( new QgsPackageAlgorithm() );
  addAlgorithm( new QgsPointOnSurfaceAlgorithm() );
  addAlgorithm( new QgsProjectPointCartesianAlgorithm() );
  addAlgorithm( new QgsPromoteToMultipartAlgorithm() );
  addAlgorithm( new QgsRasterLayerUniqueValuesReportAlgorithm() );
  addAlgorithm( new QgsAlgorithmRemoveDuplicateVertices() );
  addAlgorithm( new QgsRemoveHolesAlgorithm() );
  addAlgorithm( new QgsRemoveNullGeometryAlgorithm() );
  addAlgorithm( new QgsRenameLayerAlgorithm() );
  addAlgorithm( new QgsRotateFeaturesAlgorithm() );
  addAlgorithm( new QgsSaveSelectedFeatures() );
  addAlgorithm( new QgsSegmentizeByMaximumAngleAlgorithm() );
  addAlgorithm( new QgsSegmentizeByMaximumDistanceAlgorithm() );
  addAlgorithm( new QgsSelectByLocationAlgorithm() );
  addAlgorithm( new QgsSimplifyAlgorithm() );
  addAlgorithm( new QgsSmoothAlgorithm() );
  addAlgorithm( new QgsSnapToGridAlgorithm() );
  addAlgorithm( new QgsSplitWithLinesAlgorithm() );
  addAlgorithm( new QgsStringConcatenationAlgorithm() );
  addAlgorithm( new QgsSubdivideAlgorithm() );
  addAlgorithm( new QgsSwapXYAlgorithm() );
  addAlgorithm( new QgsTransectAlgorithm() );
  addAlgorithm( new QgsTransformAlgorithm() );
  addAlgorithm( new QgsTranslateAlgorithm() );
}


///@endcond



