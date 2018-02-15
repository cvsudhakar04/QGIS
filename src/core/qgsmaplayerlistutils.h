#ifndef QGSMAPLAYERLISTUTILS_H
#define QGSMAPLAYERLISTUTILS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QPointer>

#include "qgsmaplayer.h"
#include "qgsmaplayerref.h"

/// @cond PRIVATE

inline QList<QgsMapLayer *> _qgis_listQPointerToRaw( const QgsWeakMapLayerPointerList &layers )
{
  QList<QgsMapLayer *> lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QgsWeakMapLayerPointer &layerPtr, layers )
  {
    if ( layerPtr )
      lst.append( layerPtr.data() );
  }
  return lst;
}

inline QgsWeakMapLayerPointerList _qgis_listRawToQPointer( const QList<QgsMapLayer *> &layers )
{
  QgsWeakMapLayerPointerList lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    lst.append( layer );
  }
  return lst;
}

inline QList<QgsMapLayer *> _qgis_listRefToRaw( const QList< QgsMapLayerRef > &layers )
{
  QList<QgsMapLayer *> lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QgsMapLayerRef &layer, layers )
  {
    if ( layer )
      lst.append( layer.get() );
  }
  return lst;
}

inline QList< QgsMapLayerRef > _qgis_listRawToRef( const QList<QgsMapLayer *> &layers )
{
  QList< QgsMapLayerRef > lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    lst.append( QgsMapLayerRef( layer ) );
  }
  return lst;
}

inline void _qgis_removeLayers( QList< QgsMapLayerRef > &list, const QList< QgsMapLayer *> &layersToRemove )
{
  QMutableListIterator<QgsMapLayerRef> it( list );
  while ( it.hasNext() )
  {
    QgsMapLayerRef &ref = it.next();
    if ( layersToRemove.contains( ref.get() ) )
      it.remove();
  }
}

inline QStringList _qgis_listQPointerToIDs( const QgsWeakMapLayerPointerList &layers )
{
  QStringList lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QgsWeakMapLayerPointer &layerPtr, layers )
  {
    if ( layerPtr )
      lst << layerPtr->id();
  }
  return lst;
}

inline static QgsMapLayer *_qgis_findLayer( const QList< QgsMapLayer *> &layers, const QString &identifier )
{
  QgsMapLayer *matchId = nullptr;
  QgsMapLayer *matchName = nullptr;
  QgsMapLayer *matchNameInsensitive = nullptr;

  // Look for match against layer IDs
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    if ( !matchId && layer->id() == identifier )
    {
      matchId = layer;
      break;
    }
    if ( !matchName && layer->name() == identifier )
    {
      matchName = layer;
    }
    if ( !matchNameInsensitive && QString::compare( layer->name(), identifier, Qt::CaseInsensitive ) == 0 )
    {
      matchNameInsensitive = layer;
    }
  }

  if ( matchId )
  {
    return matchId;
  }
  else if ( matchName )
  {
    return matchName;
  }
  else if ( matchNameInsensitive )
  {
    return matchNameInsensitive;
  }
  else
  {
    return nullptr;
  }
}

inline uint qHash( const QgsWeakMapLayerPointer &key )
{
  return qHash( key ? key->id() : QString() );
}

///@endcond

#endif // QGSMAPLAYERLISTUTILS_H
