/***************************************************************************
  qgsvectortilelayer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelayer.h"

#include "qgslogger.h"
#include "qgsvectortilelayerrenderer.h"
#include "qgsmbtilesreader.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortileloader.h"

#include "qgsdatasourceuri.h"


QgsVectorTileLayer::QgsVectorTileLayer( const QString &uri, const QString &baseName )
  : QgsMapLayer( QgsMapLayerType::VectorTileLayer, baseName )
{
  mDataSource = uri;

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  mSourceType = dsUri.param( QStringLiteral( "type" ) );
  mSourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( mSourceType == QStringLiteral( "xyz" ) )
  {
    // online tiles
    mSourceMinZoom = 0;
    mSourceMaxZoom = 14;

    if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
      mSourceMinZoom = dsUri.param( QStringLiteral( "zmin" ) ).toInt();
    if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
      mSourceMaxZoom = dsUri.param( QStringLiteral( "zmax" ) ).toInt();

    setExtent( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) );
  }
  else if ( mSourceType == QStringLiteral( "mbtiles" ) )
  {
    QgsMBTilesReader reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open MBTiles file: " ) + mSourcePath );
      return;
    }

    QgsDebugMsgLevel( QStringLiteral( "name: " ) + reader.metadataValue( QStringLiteral( "name" ) ), 2 );
    bool minZoomOk, maxZoomOk;
    int minZoom = reader.metadataValue( QStringLiteral( "minzoom" ) ).toInt( &minZoomOk );
    int maxZoom = reader.metadataValue( QStringLiteral( "maxzoom" ) ).toInt( &maxZoomOk );
    if ( minZoomOk )
      mSourceMinZoom = minZoom;
    if ( maxZoomOk )
      mSourceMaxZoom = maxZoom;
    QgsDebugMsgLevel( QStringLiteral( "zoom range: %1 - %2" ).arg( mSourceMinZoom ).arg( mSourceMaxZoom ), 2 );

    QgsRectangle r = reader.extent();
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                               QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext() );
    r = ct.transformBoundingBox( r );
    setExtent( r );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Unknown source type: " ) + mSourceType );
    return;
  }

  setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  setValid( true );

  // set a default renderer
  QgsVectorTileBasicRenderer *renderer = new QgsVectorTileBasicRenderer;
  renderer->setStyles( QgsVectorTileBasicRenderer::simpleStyleWithRandomColors() );
  setRenderer( renderer );
}

QgsVectorTileLayer::~QgsVectorTileLayer() = default;


QgsVectorTileLayer *QgsVectorTileLayer::clone() const
{
  QgsVectorTileLayer *layer = new QgsVectorTileLayer( source(), name() );
  layer->setRenderer( renderer() ? renderer()->clone() : nullptr );
  return layer;
}

QgsMapLayerRenderer *QgsVectorTileLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsVectorTileLayerRenderer( this, rendererContext );
}

bool QgsVectorTileLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QString errorMsg;
  return readSymbology( layerNode, errorMsg, context );
}

bool QgsVectorTileLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "vector-tile" ) );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsVectorTileLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  QDomElement elemRenderer = elem.firstChildElement( QStringLiteral( "renderer" ) );
  if ( elemRenderer.isNull() )
  {
    errorMessage = tr( "Missing <renderer> tag" );
    return false;
  }
  QString rendererType = elemRenderer.attribute( QStringLiteral( "type" ) );
  QgsVectorTileRenderer *r = nullptr;
  if ( rendererType == QStringLiteral( "basic" ) )
    r = new QgsVectorTileBasicRenderer;
  else
  {
    errorMessage = tr( "Unknown renderer type: " ) + rendererType;
    return false;
  }

  r->readXml( elemRenderer, context );
  return true;
}

bool QgsVectorTileLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage )
  QDomElement elem = node.toElement();

  writeCommonStyle( elem, doc, context, categories );

  if ( mRenderer )
  {
    QDomElement elemRenderer = doc.createElement( QStringLiteral( "renderer" ) );
    elemRenderer.setAttribute( QStringLiteral( "type" ), mRenderer->type() );
    mRenderer->writeXml( elemRenderer, context );
    elem.appendChild( elemRenderer );
  }
  return true;
}

void QgsVectorTileLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  Q_UNUSED( transformContext )
}

QByteArray QgsVectorTileLayer::getRawTile( QgsTileXYZ tileID )
{
  QgsTileRange tileRange( tileID.column(), tileID.column(), tileID.row(), tileID.row() );
  QList<QgsVectorTileRawData> rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mSourceType, mSourcePath, tileID.zoomLevel(), QPointF(), tileRange );
  if ( rawTiles.isEmpty() )
    return QByteArray();
  return rawTiles.first().data;
}

void QgsVectorTileLayer::setRenderer( QgsVectorTileRenderer *r )
{
  mRenderer.reset( r );
}

QgsVectorTileRenderer *QgsVectorTileLayer::renderer() const
{
  return mRenderer.get();
}
