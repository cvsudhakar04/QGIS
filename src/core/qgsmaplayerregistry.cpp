/***************************************************************************
 *  QgsMapLayerRegistry.cpp  -  Singleton class for tracking mMapLayers.
 *                         -------------------
 * begin                : Sun June 02 2004
 * copyright            : (C) 2004 by Tim Sutton
 * email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"

//
// Static calls to enforce singleton behaviour
//
QgsMapLayerRegistry *QgsMapLayerRegistry::instance()
{
  static QgsMapLayerRegistry sInstance;
  return &sInstance;
}

//
// Main class begins now...
//

QgsMapLayerRegistry::QgsMapLayerRegistry( QObject *parent )
    : QObject( parent )
    , mRegistryLock( QReadWriteLock::Recursive )
{
  // constructor does nothing
}

QgsMapLayerRegistry::~QgsMapLayerRegistry()
{
  removeAllMapLayers();
}

// get the layer count (number of registered layers)
int QgsMapLayerRegistry::count()
{
  QReadLocker lock( &mRegistryLock );
  return mMapLayers.size();
}

QgsMapLayer * QgsMapLayerRegistry::mapLayer( const QString& theLayerId )
{
  QReadLocker lock( &mRegistryLock );
  return mMapLayers.value( theLayerId );
}

QList<QgsMapLayer *> QgsMapLayerRegistry::mapLayersByName( const QString& layerName )
{
  QReadLocker lock( &mRegistryLock );
  QList<QgsMapLayer *> myResultList;
  Q_FOREACH ( QgsMapLayer* layer, mMapLayers )
  {
    if ( layer->name() == layerName )
    {
      myResultList << layer;
    }
  }
  return myResultList;
}

//introduced in 1.8
QList<QgsMapLayer *> QgsMapLayerRegistry::addMapLayers(
  const QList<QgsMapLayer *>& theMapLayers,
  bool addToLegend,
  bool takeOwnership )
{
  QList<QgsMapLayer *> myResultList;
  Q_FOREACH ( QgsMapLayer* myLayer, theMapLayers )
  {
    if ( !myLayer || !myLayer->isValid() )
    {
      QgsDebugMsg( "Cannot add invalid layers" );
      continue;
    }
    //check the layer is not already registered!
    mRegistryLock.lockForRead();
    if ( !mMapLayers.contains( myLayer->id() ) )
    {
      mRegistryLock.unlock();
      mRegistryLock.lockForWrite();
      mMapLayers[myLayer->id()] = myLayer;
      mRegistryLock.unlock();
      myResultList << myLayer;
      if ( takeOwnership )
      {
        myLayer->setParent( this );
      }
      connect( myLayer, SIGNAL( destroyed( QObject* ) ), this, SLOT( onMapLayerDeleted( QObject* ) ) );
      emit layerWasAdded( myLayer );
    }
    else
      mRegistryLock.unlock();
  }
  if ( !myResultList.isEmpty() )
  {
    emit layersAdded( myResultList );

    if ( addToLegend )
      emit legendLayersAdded( myResultList );
  }
  return myResultList;
} // QgsMapLayerRegistry::addMapLayers

//this is just a thin wrapper for addMapLayers
QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer* theMapLayer,
                                  bool addToLegend,
                                  bool takeOwnership )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer*>() << theMapLayer, addToLegend, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}


//introduced in 1.8
void QgsMapLayerRegistry::removeMapLayers( const QStringList& theLayerIds )
{
  QList<QgsMapLayer*> layers;
  mRegistryLock.lockForRead();
  Q_FOREACH ( const QString &myId, theLayerIds )
  {
    layers << mMapLayers.value( myId );
  }
  mRegistryLock.unlock();

  removeMapLayers( layers );
}

void QgsMapLayerRegistry::removeMapLayers( const QList<QgsMapLayer*>& layers )
{
  if ( layers.isEmpty() )
    return;

  QStringList layerIds;

  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    if ( layer )
      layerIds << layer->id();
  }

  emit layersWillBeRemoved( layerIds );
  emit layersWillBeRemoved( layers );

  Q_FOREACH ( QgsMapLayer* lyr, layers )
  {
    if ( !lyr )
      continue;

    QString myId( lyr->id() );
    emit layerWillBeRemoved( myId );
    emit layerWillBeRemoved( lyr );
    mRegistryLock.lockForWrite();
    mMapLayers.remove( myId );
    mRegistryLock.unlock();
    if ( lyr->parent() == this )
    {
      delete lyr;
    }
    emit layerRemoved( myId );
  }

  emit layersRemoved( layerIds );
}

void QgsMapLayerRegistry::removeMapLayer( const QString& theLayerId )
{
  removeMapLayers( QList<QgsMapLayer*>() << mMapLayers.value( theLayerId ) );
}

void QgsMapLayerRegistry::removeMapLayer( QgsMapLayer* layer )
{
  if ( layer )
    removeMapLayers( QList<QgsMapLayer*>() << layer );
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  emit removeAll();
  // now let all canvas observers know to clear themselves,
  // and then consequently any of their map legends
  removeMapLayers( mMapLayers.keys() );
  QWriteLocker lock( &mRegistryLock );
  mMapLayers.clear();
} // QgsMapLayerRegistry::removeAllMapLayers()

void QgsMapLayerRegistry::reloadAllLayers()
{
  QReadLocker lock( &mRegistryLock );
  Q_FOREACH ( QgsMapLayer* layer, mMapLayers )
  {
    layer->reload();
  }
}

void QgsMapLayerRegistry::onMapLayerDeleted( QObject* obj )
{
  QString id = mMapLayers.key( static_cast<QgsMapLayer*>( obj ) );

  if ( !id.isNull() )
  {
    QgsDebugMsg( QString( "Map layer deleted without unregistering! %1" ).arg( id ) );
    mMapLayers.remove( id );
  }
}

QMap<QString, QgsMapLayer*> QgsMapLayerRegistry::mapLayers()
{
  QReadLocker lock( &mRegistryLock );

  QMap<QString, QgsMapLayer*> mapLayers = mMapLayers;
  mapLayers.detach();
  return mapLayers;
}


#if 0
void QgsMapLayerRegistry::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
} //  QgsMapLayerRegistry::connectNotify
#endif
