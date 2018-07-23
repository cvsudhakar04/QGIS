/***************************************************************************
    begin                : August 1, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSO2_H
#define QGSO2_H

#include "o2.h"

class QgsAuthOAuth2Config;

/**
 * QGIS-specific subclass of O2 lib's base OAuth 2.0 authenticator.
 * Adds support for QGIS authentication system.
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsO2: public O2
{

    Q_OBJECT

  public:

    /**
     * Construct QgsO2
     * \param authcfg authentication configuration id
     * \param oauth2config OAuth2 configuration
     * \param parent
     * \param manager QGIS network access manager instance
     */
    explicit QgsO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config = nullptr,
                    QObject *parent = nullptr, QNetworkAccessManager *manager = nullptr );

    ~QgsO2() override;

    //! Authentication configuration id
    QString authcfg() const { return mAuthcfg; }
    //! OAuth2 configuration
    QgsAuthOAuth2Config *oauth2config() { return mOAuth2Config; }

  public slots:

    //! Clear all properties
    void clearProperties();

  private:
    void initOAuthConfig();

    void setSettingsStore( bool persist = false );

    void setVerificationResponseContent();

    QString mTokenCacheFile;
    QString mAuthcfg;
    QgsAuthOAuth2Config *mOAuth2Config;
};

#endif // QGSO2_H
