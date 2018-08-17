/***************************************************************************
                         qgsmeshlayerrenderer.cpp
                         ------------------------
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

#include <memory>

#include "qgsmeshlayerrenderer.h"

#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgssettings.h"
#include "qgsstyle.h"


QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id() )
  , mFeedback( new QgsMeshLayerRendererFeedback )
  , mContext( context )
  , mRendererSettings( layer->rendererSettings() )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  createMeshSymbol( mNativeMeshSymbol, mRendererSettings.nativeMeshSettings() );
  createMeshSymbol( mTriangularMeshSymbol, mRendererSettings.triangularMeshSettings() );

  copyScalarDatasetValues( layer );
  copyVectorDatasetValues( layer );

  calculateOutputSize();
}

QgsFeedback *QgsMeshLayerRenderer::feedback() const
{
  return mFeedback.get();
}

void QgsMeshLayerRenderer::calculateOutputSize()
{
  // figure out image size
  QgsRectangle extent = mContext.extent();  // this is extent in layer's coordinate system - but we need it in map coordinate system
  QgsMapToPixel mapToPixel = mContext.mapToPixel();
  QgsPointXY topleft = mapToPixel.transform( extent.xMinimum(), extent.yMaximum() );
  QgsPointXY bottomright = mapToPixel.transform( extent.xMaximum(), extent.yMinimum() );
  int width = int( bottomright.x() - topleft.x() );
  int height = int( bottomright.y() - topleft.y() );
  mOutputSize = QSize( width, height );
}

void QgsMeshLayerRenderer::createMeshSymbol( std::unique_ptr<QgsSymbol> &symbol,
    const QgsMeshRendererMeshSettings &settings )
{
  if ( settings.isEnabled() )
  {
    QgsSymbolLayerList l1;
    l1 << new QgsSimpleFillSymbolLayer( Qt::white,
                                        Qt::NoBrush,
                                        settings.color(),
                                        Qt::SolidLine,
                                        settings.lineWidth() );
    symbol.reset( new QgsFillSymbol( l1 ) );
  }
}


void QgsMeshLayerRenderer::copyScalarDatasetValues( QgsMeshLayer *layer )
{
  const QgsMeshDatasetIndex datasetIndex = mRendererSettings.activeScalarDataset();
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->dataProvider()->datasetGroupMetadata( datasetIndex );
    mScalarDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
    int count;
    if ( mScalarDataOnVertices )
      count = mNativeMesh.vertices.count();
    else
      count = mNativeMesh.faces.count();

    mScalarDatasetValues.resize( count );
    for ( int i = 0; i < count; ++i )
    {
      double v = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
      mScalarDatasetValues[i] = v;
    }

    QgsMeshLayerUtils::calculateMinimumMaximum( mScalarDatasetMinimum, mScalarDatasetMaximum, mScalarDatasetValues );
  }
}

void QgsMeshLayerRenderer::copyVectorDatasetValues( QgsMeshLayer *layer )
{
  const QgsMeshDatasetIndex datasetIndex = mRendererSettings.activeVectorDataset();
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->dataProvider()->datasetGroupMetadata( datasetIndex );

    bool isScalar = metadata.isScalar();
    if ( isScalar )
    {
      QgsDebugMsg( "Dataset has no vector values" );
    }
    else
    {
      mVectorDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
      int count;
      if ( mVectorDataOnVertices )
        count = mNativeMesh.vertices.count();
      else
        count = mNativeMesh.faces.count();

      mVectorDatasetValuesX.resize( count );
      mVectorDatasetValuesY.resize( count );
      mVectorDatasetValuesMag.resize( count );
      for ( int i = 0; i < count; ++i )
      {
        double x = layer->dataProvider()->datasetValue( datasetIndex, i ).x();
        mVectorDatasetValuesX[i] = x;

        double y = layer->dataProvider()->datasetValue( datasetIndex, i ).y();
        mVectorDatasetValuesY[i] = y;

        double mag = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
        mVectorDatasetValuesMag[i] = mag;
      }
    }

    QgsMeshLayerUtils::calculateMinimumMaximum( mVectorDatasetMagMinimum, mVectorDatasetMagMaximum, mVectorDatasetValuesMag );
  }
}

bool QgsMeshLayerRenderer::render()
{

  renderScalarDataset();

  renderMesh( mNativeMeshSymbol, mNativeMesh.faces ); // native mesh
  renderMesh( mTriangularMeshSymbol, mTriangularMesh.triangles() ); // triangular mesh

  renderVectorDataset();

  return true;
}

void QgsMeshLayerRenderer::renderMesh( const std::unique_ptr<QgsSymbol> &symbol, const QVector<QgsMeshFace> &faces )
{
  if ( !symbol )
    return;

  QgsFields fields;
  QgsSingleSymbolRenderer renderer( symbol->clone() );
  renderer.startRender( mContext, fields );

  for ( int i = 0; i < faces.size(); ++i )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsMeshFace &face = faces[i];
    QgsFeature feat;
    feat.setFields( fields );
    QgsGeometry geom = QgsMeshUtils::toGeometry( face, mTriangularMesh.vertices() ); //Triangular mesh vertices contains also native mesh vertices
    feat.setGeometry( geom );
    renderer.renderFeature( feat, mContext );
  }

  renderer.stopRender( mContext );
}

void QgsMeshLayerRenderer::renderScalarDataset()
{
  if ( mScalarDatasetValues.isEmpty() )
    return; // activeScalarDataset == NO_ACTIVE_MESH_DATASET

  if ( std::isnan( mScalarDatasetMinimum ) || std::isnan( mScalarDatasetMaximum ) )
    return; // only NODATA values

  QgsMeshDatasetIndex index = mRendererSettings.activeScalarDataset();
  if ( !index.isValid() )
    return; // no shader

  const QgsMeshRendererScalarSettings scalarSettings = mRendererSettings.scalarSettings( index.group() );
  QgsColorRampShader *fcn = new QgsColorRampShader( scalarSettings.colorRampShader() );
  QgsRasterShader *sh = new QgsRasterShader();
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh, mScalarDatasetValues, mScalarDataOnVertices, mContext, mOutputSize );
  QgsSingleBandPseudoColorRenderer renderer( &interpolator, 0, sh );  // takes ownership of sh
  renderer.setClassificationMin( scalarSettings.classificationMinimum() );
  renderer.setClassificationMax( scalarSettings.classificationMaximum() );

  std::unique_ptr<QgsRasterBlock> bl( renderer.block( 0, mContext.extent(), mOutputSize.width(), mOutputSize.height(), mFeedback.get() ) );
  QImage img = bl->image();

  mContext.painter()->drawImage( 0, 0, img );
}

void QgsMeshLayerRenderer::renderVectorDataset()
{
  QgsMeshDatasetIndex index = mRendererSettings.activeVectorDataset();
  if ( !index.isValid() )
    return;

  if ( mVectorDatasetValuesX.isEmpty() )
    return;

  if ( std::isnan( mVectorDatasetMagMinimum ) || std::isnan( mVectorDatasetMagMaximum ) )
    return; // only NODATA values

  if ( mVectorDatasetValuesX.size() != mVectorDatasetValuesY.size() )
    return;

  QgsMeshVectorRenderer renderer( mTriangularMesh,
                                  mVectorDatasetValuesX, mVectorDatasetValuesY, mVectorDatasetValuesMag,
                                  mVectorDatasetMagMinimum, mVectorDatasetMagMaximum,
                                  mVectorDataOnVertices, mRendererSettings.vectorSettings( index.group() ), mContext, mOutputSize );

  renderer.draw();
}
