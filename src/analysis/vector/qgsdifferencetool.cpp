/***************************************************************************
 *  differencetool.cpp                                                 *
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
#include "qgsdifferencetool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  QgsDifferenceTool::QgsDifferenceTool( QgsFeatureSource *layerA,
                                        QgsFeatureSource *layerB,
                                        QgsFeatureSink *output,
                                        QgsCoordinateTransformContext transformContext,
                                        QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck )
    : QgsAbstractTool( output, transformContext, invalidGeometryCheck ), mLayerA( layerA ), mLayerB( layerB )
  {
  }

  void QgsDifferenceTool::prepare()
  {
    appendToJobQueue( mLayerA );
    buildSpatialIndex( mSpatialIndex, mLayerB );
  }

  void QgsDifferenceTool::processFeature( const Job *job )
  {
    QgsFeature f;
    if ( !mOutput || !mLayerA || !mLayerB )
    {
      return;
    }

    QgsFeatureList difference = QgsOverlayUtils::featureDifference( job->feature, *mLayerA, *mLayerB, mSpatialIndex, mTransformContext, mLayerA->fields().size(), mLayerB->fields().size(), QgsOverlayUtils::OutputA );
    writeFeatures( difference );
  }

} // Geoprocessing
