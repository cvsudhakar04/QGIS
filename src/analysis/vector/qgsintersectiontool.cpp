/***************************************************************************
 *  qgsintersectiontool.cpp                                               *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include "qgsintersectiontool.h"
#include "qgsfeaturesource.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  QgsIntersectionTool::QgsIntersectionTool(
    QgsFeatureSource *layerA,
    QgsFeatureSource *layerB,
    const QgsAttributeList &fieldIndicesA,
    const QgsAttributeList &fieldIndicesB,
    QgsFeatureSink *output,
    QgsCoordinateTransformContext transformContext,
    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck )
    : QgsAbstractTool( output, transformContext, invalidGeometryCheck ), mLayerA( layerA ), mLayerB( layerB ), mFieldIndicesA( fieldIndicesA ), mFieldIndicesB( fieldIndicesB )
  {
  }

  void QgsIntersectionTool::prepare()
  {
    QgsFeatureRequest request;
    prepareLayer( mLayerA, mLayerA->sourceCrs(), &mFieldIndicesA );
    buildSpatialIndex( mSpatialIndex, mLayerB, mLayerA->sourceCrs() );
    mFeatureCount = mLayerA->featureCount();
  }

  void QgsIntersectionTool::processFeature( const Job *job )
  {
    if ( !mOutput || !mLayerA || !mLayerB )
    {
      return;
    }

    QgsFeatureList intersection = QgsOverlayUtils::featureIntersection( job->feature, *mLayerA, *mLayerB, mSpatialIndex, mTransformContext, mFieldIndicesA, mFieldIndicesB );
    writeFeatures( intersection );
  }

  bool QgsIntersectionTool::prepareNextChunk()
  {
    if ( !mLayerA )
    {
      return false;
    }

    return appendNextChunkToJobQueue( mLayerA );
  }
} // Geoprocessing
