/***************************************************************************
    qgsvectortileconnection.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileconnection.h"

#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgssettings.h"
#include "qgshttpheaders.h"

#include <QFileInfo>

///@cond PRIVATE

QString QgsVectorTileProviderConnection::encodedUri( const QgsVectorTileProviderConnection::Data &conn )
{
  QgsDataSourceUri uri;

  const QFileInfo info( conn.url );
  QString suffix = info.suffix().toLower();
  if ( suffix.startsWith( QLatin1String( "mbtiles" ) ) )
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  }
  else
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  }

  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( QStringLiteral( "styleUrl" ),  conn.styleUrl );

  uri.setHttpHeaders( conn.httpHeaders );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
      break;
  }

  return uri.encodedUri();
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsVectorTileProviderConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.zMin = dsUri.hasParam( QStringLiteral( "zmin" ) ) ? dsUri.param( QStringLiteral( "zmin" ) ).toInt() : -1;
  conn.zMax = dsUri.hasParam( QStringLiteral( "zmax" ) ) ? dsUri.param( QStringLiteral( "zmax" ) ).toInt() : -1;
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.styleUrl = dsUri.param( QStringLiteral( "styleUrl" ) );

  conn.httpHeaders = dsUri.httpHeaders();

  if ( dsUri.hasParam( QStringLiteral( "serviceType" ) ) )
  {
    if ( dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
      conn.serviceType = ArcgisVectorTileService;
  }
  return conn;
}

QString QgsVectorTileProviderConnection::encodedLayerUri( const QgsVectorTileProviderConnection::Data &conn )
{
  // compared to encodedUri() this one also adds type=xyz to the URI
  QgsDataSourceUri uri;

  const QFileInfo info( conn.url );
  QString suffix = info.suffix().toLower();
  if ( suffix.startsWith( QLatin1String( "mbtiles" ) ) )
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  }
  else
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  }

  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( QStringLiteral( "styleUrl" ),  conn.styleUrl );

  uri.setHttpHeaders( conn.httpHeaders );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
      break;
  }

  return uri.encodedUri();
}

QStringList QgsVectorTileProviderConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );
  QStringList connList = settings.childGroups();

  return connList;
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::connection( const QString &name )
{
  if ( !settingsUrl.exists( name ) )
    return QgsVectorTileProviderConnection::Data();

  QgsVectorTileProviderConnection::Data conn;
  conn.url = settingsUrl.value( name );
  conn.zMin = settingsZzmin.value( name );
  conn.zMax = settingsZmax.value( name );
  conn.authCfg = settingsAuthcfg.value( name );
  conn.username = settingsUsername.value( name );
  conn.password = settingsPassword.value( name );
  conn.styleUrl = settingsStyleUrl.value( name );

  if ( settingsHeaders.exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders.value( name ) );
  else
  {
    // TODO QGIS 4 (or before) remove compatibility import
    QgsSettings settings;
    settings.beginGroup( "qgis/connections-vector-tile/" + name );
    Q_NOWARN_DEPRECATED_PUSH
    conn.httpHeaders = QgsHttpHeaders( settings );
    Q_NOWARN_DEPRECATED_POP
  }

  if ( settingsServiceType.exists( name ) &&  settingsServiceType.value( name ) == QLatin1String( "arcgis" ) )
    conn.serviceType = ArcgisVectorTileService;

  return conn;
}

void QgsVectorTileProviderConnection::deleteConnection( const QString &name )
{
  settingsConnections.removeAllChildrenSettings( name );
}

void QgsVectorTileProviderConnection::addConnection( const QString &name, QgsVectorTileProviderConnection::Data conn )
{
  settingsUrl.setValue( conn.url, name );
  settingsZzmin.setValue( conn.zMin, name );
  settingsZmax.setValue( conn.zMax, name );
  settingsAuthcfg.setValue( conn.authCfg, name );
  settingsUsername.setValue( conn.username, name );
  settingsPassword.setValue( conn.password, name );
  settingsStyleUrl.setValue( conn.styleUrl, name );

  settingsHeaders.setValue( conn.httpHeaders.headers(), name );


  // TODO QGIS 4 (or before) remove compatibility import
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-vector-tile/" + name );
  Q_NOWARN_DEPRECATED_PUSH
  conn.httpHeaders.updateSettings( settings );
  Q_NOWARN_DEPRECATED_POP
  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      settingsServiceType.setValue( QStringLiteral( "arcgis" ), name );
      break;
  }
}

QString QgsVectorTileProviderConnection::selectedConnection()
{
  return settingsConnectionSelected.value();
}

void QgsVectorTileProviderConnection::setSelectedConnection( const QString &name )
{
  settingsConnectionSelected.setValue( name );
}


QgsVectorTileProviderConnection::QgsVectorTileProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  setUri( encodedUri( connection( name ) ) );
}

QgsVectorTileProviderConnection::QgsVectorTileProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
{
}

void QgsVectorTileProviderConnection::store( const QString &name ) const
{
  addConnection( name, decodedUri( uri() ) );
}

void QgsVectorTileProviderConnection::remove( const QString &name ) const
{
  deleteConnection( name );
}

///@endcond
