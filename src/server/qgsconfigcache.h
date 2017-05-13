/***************************************************************************
                              qgsconfigcache.h
                              ----------------
  begin                : July 24th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONFIGCACHE_H
#define QGSCONFIGCACHE_H

#include "qgsconfig.h"

#include <QCache>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QDomDocument>

#include "qgis_server.h"
#include "qgswmsconfigparser.h"

class QgsServerProjectParser;
class QgsAccessControl;

class SERVER_EXPORT QgsConfigCache : public QObject
{
    Q_OBJECT
  public:
    static QgsConfigCache *instance();

    QgsServerProjectParser *serverConfiguration( const QString &filePath );
    QgsWmsConfigParser *wmsConfiguration(
      const QString &filePath
      , const QgsAccessControl *accessControl
      , const QMap<QString, QString> &parameterMap = ( QMap< QString, QString >() )
    );

    void removeEntry( const QString &path );

  private:
    QgsConfigCache();

    //! Check for configuration file updates (remove entry from cache if file changes)
    QFileSystemWatcher mFileSystemWatcher;

    //! Returns xml document for project file / sld or 0 in case of errors
    QDomDocument *xmlDocument( const QString &filePath );

    QCache<QString, QDomDocument> mXmlDocumentCache;
    QCache<QString, QgsWmsConfigParser> mWMSConfigCache;

  private slots:
    //! Removes changed entry from this cache
    void removeChangedEntry( const QString &path );
};

#endif // QGSCONFIGCACHE_H
