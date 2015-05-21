/***************************************************************************

               ----------------------------------------------------
              date                 : 19.5.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBPAGE_H
#define QGSWEBPAGE_H

#include <QObject>

#ifdef WITH_QTWEBKIT
#include <QWebPage>
#else

#include "qgswebframe.h"

#include <QMenu>
#include <QNetworkAccessManager>
#include <QPalette>

class QWebSettings : public QObject
{
    Q_OBJECT

  public:

    enum WebAttribute {
        AutoLoadImages,
        JavascriptEnabled,
        JavaEnabled,
        PluginsEnabled,
        PrivateBrowsingEnabled,
        JavascriptCanOpenWindows,
        JavascriptCanAccessClipboard,
        DeveloperExtrasEnabled,
        LinksIncludedInFocusChain,
        ZoomTextOnly,
        PrintElementBackgrounds,
        OfflineStorageDatabaseEnabled,
        OfflineWebApplicationCacheEnabled,
        LocalStorageEnabled,
        LocalContentCanAccessRemoteUrls,
        DnsPrefetchEnabled,
        XSSAuditingEnabled,
        AcceleratedCompositingEnabled,
        SpatialNavigationEnabled,
        LocalContentCanAccessFileUrls,
        TiledBackingStoreEnabled,
        FrameFlatteningEnabled,
        SiteSpecificQuirksEnabled,
        JavascriptCanCloseWindows,
        WebGLEnabled,
        CSSRegionsEnabled,
        HyperlinkAuditingEnabled,
        CSSGridLayoutEnabled,
        ScrollAnimatorEnabled,
        CaretBrowsingEnabled,
        NotificationsEnabled
    };
    explicit QWebSettings( QObject* parent = 0 )
      :QObject( parent )
    {

    }

    void setUserStyleSheetUrl( const QUrl& )
    {

    }

    void setAttribute( WebAttribute, bool on )
    {
      Q_UNUSED( on );
    }
};

class QWebPage : public QObject
{
    Q_OBJECT

  public:

    enum LinkDelegationPolicy {
        DontDelegateLinks,
        DelegateExternalLinks,
        DelegateAllLinks
    };

    enum WebWindowType {
        WebBrowserWindow,
        WebModalDialog
    };

    explicit QWebPage( QObject* parent = 0 )
      : QObject( parent )
      , mSettings( new QWebSettings() )
      , mFrame( new QWebFrame() )
    {
    }

    ~QWebPage()
    {
      delete mFrame;
      delete mSettings;
    }

    QPalette palette() const
    {
      return QPalette();
    }

    void setPalette( const QPalette& palette )
    {
      Q_UNUSED( palette );
    }

    void setViewportSize(const QSize & size) const
    {
      Q_UNUSED( size );
    }

    void setLinkDelegationPolicy( LinkDelegationPolicy );

    void setNetworkAccessManager( QNetworkAccessManager* networkAccessManager )
    {
      Q_UNUSED( networkAccessManager );
    }

    QWebFrame* mainFrame() const
    {
      return mFrame;
    }

    QWebSettings* settings() const
    {
      return mSettings;
    }

    QSize viewportSize() const
    {
      return QSize();
    }

    QMenu* createStandardContextMenu()
    {
      return new QMenu();
    }

  signals:

  public slots:

  private:
    QWebSettings* mSettings;
    QWebFrame* mFrame;
};
#endif

#endif // QGSWEBPAGE_H
