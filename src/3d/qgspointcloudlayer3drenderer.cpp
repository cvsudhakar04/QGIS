/***************************************************************************
  qgspointcloudlayer3drenderer.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayer3drenderer.h"

#include "qgs3dutils.h"
#include "qgschunkedentity_p.h"
#include "qgspointcloudlayerchunkloader_p.h"

#include "qgspointcloudindex.h"
#include "qgspointcloudlayer.h"
#include "qgsxmlutils.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"


QgsPointCloudLayer3DRendererMetadata::QgsPointCloudLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( QStringLiteral( "pointcloud" ) )
{
}

QgsAbstract3DRenderer *QgsPointCloudLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsPointCloudLayer3DRenderer *r = new QgsPointCloudLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


QgsPointCloudLayer3DRenderer::QgsPointCloudLayer3DRenderer( )
{
}

void QgsPointCloudLayer3DRenderer::setLayer( QgsPointCloudLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsPointCloudLayer *QgsPointCloudLayer3DRenderer::layer() const
{
  return qobject_cast<QgsPointCloudLayer *>( mLayerRef.layer );
}

QString QgsPointCloudLayer3DRenderer::type() const
{
  return QStringLiteral( "pointcloud" );
}

QgsPointCloudLayer3DRenderer *QgsPointCloudLayer3DRenderer::clone() const
{
  QgsPointCloudLayer3DRenderer *r = new QgsPointCloudLayer3DRenderer;
  return r;
}

Qt3DCore::QEntity *QgsPointCloudLayer3DRenderer::createEntity( const Qgs3DMapSettings &map ) const
{
  QgsPointCloudLayer *pcl = layer();
  if ( !pcl || !pcl->dataProvider() || !pcl->dataProvider()->index() )
    return nullptr;

  return new QgsPointCloudLayerChunkedEntity( pcl->dataProvider()->index(), map );
}

void QgsPointCloudLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "layer" ), mLayerRef.layerId );
}

void QgsPointCloudLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mLayerRef = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
}

void QgsPointCloudLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}
