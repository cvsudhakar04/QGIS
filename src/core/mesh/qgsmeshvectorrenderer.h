/***************************************************************************
                         qgstriangularmesh.h
                         -------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHVECTORRENDERER_H
#define QGSMESHVECTORRENDERER_H


#define SIP_NO_FILE

#include <QVector>
#include <QSize>

#include "qgis_core.h"
#include "qgsmeshdataprovider.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"

///@cond PRIVATE

/**
 * \ingroup core
 *
 * Helper private class for rendering vector datasets (e.g. velocity)
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshVectorRenderer
{
  public:
    //! Ctor
    QgsMeshVectorRenderer( const QgsTriangularMesh &m,
                           const QVector<double> &datasetValuesX,
                           const QVector<double> &datasetValuesY,
                           const QVector<double> &datasetValuesMag,
                           bool dataIsOnVertices,
                           const QgsMeshRendererVectorSettings &settings,
                           QgsRenderContext &context,
                           const QSize &size );
    //! Dtor
    ~QgsMeshVectorRenderer();

    /**
     * Draws vector arrows in the context's painter based on settings
     */
    void draw();

  private:
    //! Draws for data defined on vertices
    void drawVectorDataOnVertices();
    //! Draws for data defined on face centers
    void drawVectorDataOnFaces();
    //! Draws arrow from start point and vector data
    void drawVectorArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );
    //! Calculates the end point of the arrow based on start point and vector data
    bool calcVectorLineEnd( QgsPointXY &lineEnd,
                            double &vectorLength,
                            double &cosAlpha,
                            double &sinAlpha, //out
                            const QgsPointXY &lineStart,
                            double xVal,
                            double yVal,
                            double magnitude //in
                          );


    const QgsTriangularMesh &mTriangularMesh;
    const QVector<double> &mDatasetValuesX;
    const QVector<double> &mDatasetValuesY;
    const QVector<double> &mDatasetValuesMag; //magnitudes
    double mMinX = 0.0;
    double mMaxX = 0.0;
    double mMinY = 0.0;
    double mMaxY = 0.0;
    double mMinMag = 0.0;
    double mMaxMag = 0.0;
    QgsRenderContext &mContext;
    const QgsMeshRendererVectorSettings &mCfg;
    bool mDataOnVertices = true;
    QSize mOutputSize;
};

///@endcond



#endif // QGSMESHVECTORRENDERER_H
