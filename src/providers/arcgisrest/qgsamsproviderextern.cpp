/***************************************************************************
    qgsamsproviderextern.cpp
    ------------------------
  begin                : Nov 26, 2015
  copyright            : (C) 2015 Sandro Mani
  email                : manisandro@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsamsdataitems.h"
#include "qgsamsprovider.h"
#include "qgsamssourceselect.h"
#include "qgsowsconnection.h"

const QString AMS_KEY = QStringLiteral( "arcgismapserver" );
const QString AMS_DESCRIPTION = QStringLiteral( "ArcGIS Map Server data provider" );


QGISEXTERN QgsAmsProvider * classFactory( const QString *uri )
{
  return new QgsAmsProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return AMS_KEY;
}

QGISEXTERN QString description()
{
  return AMS_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QgsAmsSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsAmsSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  if ( thePath.isEmpty() )
  {
    return new QgsAmsRootItem( parentItem, QStringLiteral( "ArcGisMapServer" ), QStringLiteral( "arcgismapserver:" ) );
  }

  // path schema: ams:/connection name (used by OWS)
  if ( thePath.startsWith( QLatin1String( "ams:/" ) ) )
  {
    QString connectionName = thePath.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "ArcGisMapServer" ) ).contains( connectionName ) )
    {
      QgsOwsConnection connection( QStringLiteral( "ArcGisMapServer" ), connectionName );
      return new QgsAmsConnectionItem( parentItem, QStringLiteral( "ArcGisMapServer" ), thePath, connection.uri().param( QStringLiteral( "url" ) ) );
    }
  }

  return 0;
}


/*
QGISEXTERN bool saveStyle( const QString& uri, const QString& qmlStyle, const QString& sldStyle,
                           const QString& styleName, const QString& styleDescription,
                          const QString& uiFileContent, bool useAsDefault, QString& errCause )
{

}

QGISEXTERN QString loadStyle( const QString& uri, QString& errCause )
{

}

QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString& errCause )
{

}

QGISEXTERN QString getStyleById( const QString& uri, QString styleId, QString& errCause )
{

}
*/
