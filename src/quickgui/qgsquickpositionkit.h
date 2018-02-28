/***************************************************************************
 qgsquickpositionkit.h
  --------------------------------------
  Date                 : Dec. 2017
  Copyright            : (C) 2017 Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKPOSITIONKIT_H
#define QGSQUICKPOSITIONKIT_H

#include <QObject>
#include <QtPositioning>

#include "qgspoint.h"

#include "qgis_quick.h"

/**
 * \ingroup quick
 * Convinient set of tools to read GPS position and accuracy.
 *
 * Also, if one can use use_simulated_location to specify simulated position.
 * Simulated position source generates random points in circles around the selected
 * point and radius. Real GPS position is not used in this mode.
 *
 * \note QML Type: PositionKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickPositionKit : public QObject
{
    Q_OBJECT

    /**
     * GPS position in WGS84 coords
     */
    Q_PROPERTY( QgsPoint position READ position NOTIFY positionChanged )

    /**
     * GPS position is available (position property is a valid number)
     */
    Q_PROPERTY( bool hasPosition READ hasPosition NOTIFY hasPositionChanged )

    /**
     * GPS horizontal accuracy in meters, -1 if not available
     */
    Q_PROPERTY( qreal accuracy READ accuracy NOTIFY positionChanged )

    /**
     * GPS direction, bearing in degrees clockwise from north to direction of travel. -1 if not available
     */
    Q_PROPERTY( qreal direction READ direction NOTIFY positionChanged )

    /**
     * GPS position and accuracy is simulated (not real from GPS sensor). Default false (use real GPS)
     */
    Q_PROPERTY( bool isSimulated READ simulated NOTIFY isSimulatedChanged )

  public:
    explicit QgsQuickPositionKit( QObject *parent = 0 );

    bool hasPosition() const { return mHasPosition; }
    QgsPoint position() const { return mPosition; }
    qreal accuracy() const { return mAccuracy; }
    qreal direction() const { return mDirection; }
    bool simulated() const { return mIsSimulated; }

    /**
     * Use simulated GPS source.
     *
     * We do not want to have the origin point as property
     * We basically want to set it once based on project/map cente and keep
     * it that way regardless of mapsettings change (e.g. zoom etc)
     */
    Q_INVOKABLE void use_simulated_location( double longitude, double latitude, double radius );

    /**
     * Use real GPS source (not simulated)
     */
    Q_INVOKABLE void use_gps_location();

  signals:
    void positionChanged();
    void hasPositionChanged();
    void isSimulatedChanged();
    void statusChanged();

  public slots:

  private slots:
    void positionUpdated( const QGeoPositionInfo &info );
    void onUpdateTimeout();

  protected:

  protected:
    QgsPoint mPosition;
    qreal mAccuracy;
    qreal mDirection;
    bool mHasPosition;

    // Simulated source
    bool mIsSimulated;

    QGeoPositionInfoSource *mSource;

  private:
    void replacePositionSource( QGeoPositionInfoSource *source );
    QString calculateStatusLabel();

    QGeoPositionInfoSource *gpsSource();
    QGeoPositionInfoSource *simulatedSource( double longitude, double latitude, double radius );

};

#endif // QGSQUICKPOSITIONKIT_H
