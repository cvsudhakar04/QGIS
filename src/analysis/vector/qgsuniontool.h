/***************************************************************************
 *  unionaytool.h                                                      *
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

#ifndef VECTORANALYSIS_QGSUNION_TOOL_H
#define VECTORANALYSIS_QGSUNION_TOOL_H

#define SIP_NO_FILE

#include "qgsabstracttool.h"

namespace Vectoranalysis
{

  /**
   * \ingroup analysis
   * Multithreaded vector union tool
   * \since QGIS 3.14
  */
  class ANALYSIS_EXPORT QgsUnionTool : public QgsAbstractTool
  {
    public:

      /**
       * QgsUnionTool constructor
       */
      QgsUnionTool( QgsFeatureSource *layerA,
                    QgsFeatureSource *layerB,
                    const QgsAttributeList &fieldIndicesA,
                    const QgsAttributeList &fieldIndicesB,
                    QgsFeatureSink *output,
                    QgsCoordinateTransformContext transformContext,
                    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );

    protected:

      /**
       * Prepare jobs
       */
      void prepare() override;

      /**
       * Process feature
       */
      void processFeature( const Job *job ) override;

      /**
       * @brief Create job queue from next feature chunk
       * @return  true if there are more features. False if all features have been processed
       */
      bool prepareNextChunk() override;

    private:
      enum Task
      {
        ProcessLayerAFeature,
        ProcessLayerBFeature
      };

      QgsSpatialIndex mSpatialIndexA;
      QgsSpatialIndex mSpatialIndexB;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;
      QgsAttributeList mFieldIndicesA;
      QgsAttributeList mFieldIndicesB;
      bool mLayerAFinished = false;
  };

} // Geoprocessing

#endif // VECTORANALYSIS_QGSUNION_TOOL_H
