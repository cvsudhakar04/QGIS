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
#include "qgsalgorithmextenttolayer.h"
#include "qgsalgorithmextractbyattribute.h"
#include "qgsalgorithmextractbyexpression.h"
#include "qgsalgorithmextractbyextent.h"
#include "qgsalgorithmextractbylocation.h"
#include "qgsalgorithmextractvertices.h"
#include "qgsalgorithmfiledownloader.h"
#include "qgsalgorithmfixgeometries.h"
#include "qgsalgorithmjoinbyattribute.h"
#include "qgsalgorithmjoinwithlines.h"
#include "qgsalgorithmlineintersection.h"
#include "qgsalgorithmloadlayer.h"
#include "qgsalgorithmmeancoordinates.h"
#include "qgsalgorithmmergelines.h"
#include "qgsalgorithmmergevector.h"
#include "qgsalgorithmminimumenclosingcircle.h"
#include "qgsalgorithmmultiparttosinglepart.h"
#include "qgsalgorithmorderbyexpression.h"
#include "qgsalgorithmorientedminimumboundingbox.h"
#include "qgsalgorithmpackage.h"
#include "qgsalgorithmpromotetomultipart.h"
#include "qgsalgorithmrasterlayeruniquevalues.h"
#include "qgsalgorithmremoveduplicatevertices.h"
#include "qgsalgorithmremovenullgeometry.h"
#include "qgsalgorithmrenamelayer.h"
#include "qgsalgorithmsaveselectedfeatures.h"
#include "qgsalgorithmsimplify.h"
#include "qgsalgorithmsmooth.h"
#include "qgsalgorithmsnaptogrid.h"
#include "qgsalgorithmsplitwithlines.h"
#include "qgsalgorithmstringconcatenation.h"
#include "qgsalgorithmsubdivide.h"
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
  addAlgorithm( new QgsExtentToLayerAlgorithm() );
  addAlgorithm( new QgsExtractByAttributeAlgorithm() );
  addAlgorithm( new QgsExtractByExpressionAlgorithm() );
  addAlgorithm( new QgsExtractByExtentAlgorithm() );
  addAlgorithm( new QgsExtractByLocationAlgorithm() );
  addAlgorithm( new QgsExtractVerticesAlgorithm() );
  addAlgorithm( new QgsFileDownloaderAlgorithm() );
  addAlgorithm( new QgsFixGeometriesAlgorithm() );
  addAlgorithm( new QgsJoinByAttributeAlgorithm() );
  addAlgorithm( new QgsJoinWithLinesAlgorithm() );
  addAlgorithm( new QgsLineIntersectionAlgorithm() );
  addAlgorithm( new QgsLoadLayerAlgorithm() );
  addAlgorithm( new QgsMeanCoordinatesAlgorithm() );
  addAlgorithm( new QgsMergeLinesAlgorithm() );
  addAlgorithm( new QgsMergeVectorAlgorithm() );
  addAlgorithm( new QgsMinimumEnclosingCircleAlgorithm() );
  addAlgorithm( new QgsMultipartToSinglepartAlgorithm() );
  addAlgorithm( new QgsOrderByExpressionAlgorithm() );
  addAlgorithm( new QgsOrientedMinimumBoundingBoxAlgorithm() );
  addAlgorithm( new QgsPackageAlgorithm() );
  addAlgorithm( new QgsPromoteToMultipartAlgorithm() );
  addAlgorithm( new QgsRasterLayerUniqueValuesReportAlgorithm() );
  addAlgorithm( new QgsAlgorithmRemoveDuplicateVertices() );
  addAlgorithm( new QgsRemoveNullGeometryAlgorithm() );
  addAlgorithm( new QgsRenameLayerAlgorithm() );
  addAlgorithm( new QgsSaveSelectedFeatures() );
  addAlgorithm( new QgsSelectByLocationAlgorithm() );
  addAlgorithm( new QgsSimplifyAlgorithm() );
  addAlgorithm( new QgsSmoothAlgorithm() );
  addAlgorithm( new QgsSnapToGridAlgorithm() );
  addAlgorithm( new QgsSplitWithLinesAlgorithm() );
  addAlgorithm( new QgsStringConcatenationAlgorithm() );
  addAlgorithm( new QgsSubdivideAlgorithm() );
  addAlgorithm( new QgsTransectAlgorithm() );
  addAlgorithm( new QgsTransformAlgorithm() );
  addAlgorithm( new QgsTranslateAlgorithm() );
}


///@endcond



