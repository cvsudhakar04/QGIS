/***************************************************************************
  qgsmaprenderercache.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprenderercache.h"

#include "qgsmaplayer.h"
#include "qgsmaplayerlistutils.h"

QgsMapRendererCache::QgsMapRendererCache()
{
  clear();
}

void QgsMapRendererCache::clear()
{
  QMutexLocker lock( &mMutex );
  clearInternal();
}

void QgsMapRendererCache::clearInternal()
{
  mExtent.setMinimal();
  mScale = 0;

  // make sure we are disconnected from all layers
  Q_FOREACH ( const QPointer< QgsMapLayer >& layer, mConnectedLayers )
  {
    if ( layer.data() )
    {
      disconnect( layer.data(), &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
    }
  }
  mCachedImages.clear();
  mConnectedLayers.clear();
}

void QgsMapRendererCache::dropUnusedConnections()
{
  QSet< QPointer< QgsMapLayer > > stillDepends = dependentLayers();
  QSet< QPointer< QgsMapLayer > > disconnects = mConnectedLayers.subtract( stillDepends );
  Q_FOREACH ( const QPointer< QgsMapLayer >& layer, disconnects )
  {
    if ( layer.data() )
    {
      disconnect( layer.data(), &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
    }
  }

  mConnectedLayers = stillDepends;
}

QSet<QPointer<QgsMapLayer> > QgsMapRendererCache::dependentLayers() const
{
  QSet< QPointer< QgsMapLayer > > result;
  QMap<QString, CacheParameters>::const_iterator it = mCachedImages.constBegin();
  for ( ; it != mCachedImages.constEnd(); ++it )
  {
    Q_FOREACH ( const QPointer< QgsMapLayer >& l, it.value().dependentLayers )
    {
      if ( l.data() )
        result << l;
    }
  }
  return result;
}

bool QgsMapRendererCache::init( const QgsRectangle& extent, double scale )
{
  QMutexLocker lock( &mMutex );

  // check whether the params are the same
  if ( extent == mExtent &&
       qgsDoubleNear( scale, mScale ) )
    return true;

  clearInternal();

  // set new params
  mExtent = extent;
  mScale = scale;

  return false;
}

void QgsMapRendererCache::setCacheImage( const QString& cacheKey, const QImage& image, const QList<QgsMapLayer*>& dependentLayers )
{
  QMutexLocker lock( &mMutex );

  CacheParameters params;
  params.cachedImage = image;

  // connect to the layer to listen to layer's repaintRequested() signals
  Q_FOREACH ( QgsMapLayer* layer, dependentLayers )
  {
    if ( layer )
    {
      params.dependentLayers << layer;
      if ( !mConnectedLayers.contains( QPointer< QgsMapLayer >( layer ) ) )
      {
        connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
        mConnectedLayers << layer;
      }
    }
  }

  mCachedImages[cacheKey] = params;
}

bool QgsMapRendererCache::hasCacheImage( const QString& cacheKey ) const
{
  return mCachedImages.contains( cacheKey );
}

QImage QgsMapRendererCache::cacheImage( const QString& cacheKey ) const
{
  QMutexLocker lock( &mMutex );
  return mCachedImages.value( cacheKey ).cachedImage;
}

void QgsMapRendererCache::layerRequestedRepaint()
{
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( sender() );
  if ( !layer )
    return;

  QMutexLocker lock( &mMutex );

  // check through all cached images to clear any which depend on this layer
  QMap<QString, CacheParameters>::iterator it = mCachedImages.begin();
  for ( ; it != mCachedImages.end(); )
  {
    if ( !it.value().dependentLayers.contains( layer ) )
    {
      ++it;
      continue;
    }

    it = mCachedImages.erase( it );
  }
  dropUnusedConnections();
}

void QgsMapRendererCache::clearCacheImage( const QString& cacheKey )
{
  QMutexLocker lock( &mMutex );

  mCachedImages.remove( cacheKey );
  dropUnusedConnections();
}
