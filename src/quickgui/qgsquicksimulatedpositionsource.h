/***************************************************************************
 qgsquicksimulatedpositionsource.h
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

#ifndef QGSQUICKSIMULATEDPOSITIONSOURCE_H
#define QGSQUICKSIMULATEDPOSITIONSOURCE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis_quick.h"
#include <QObject>
#include <QtPositioning>
#include <qgspoint.h>

/**
 * \ingroup quick
 * This is an internal (implementation) class used to generate (fake) GPS position source
 * Useful for for testing purposes (e.g. testing of the application with map for different
 * location then your physical (GPS) location)
 *
 * Simulated position source generates random points in circles around the selected
 * point and radius. Real GPS position is not used in this mode.
 *
 * \note QML Type: not exported
 *
 * \since QGIS 3.2
 */

//!
class QUICK_NO_EXPORT QgsQuickSimulatedPositionSource : public QGeoPositionInfoSource
{
    Q_OBJECT
  public:
    QgsQuickSimulatedPositionSource( QObject *parent, double longitude, double latitude, double flightRadius );

    QGeoPositionInfo lastKnownPosition( bool /*fromSatellitePositioningMethodsOnly = false*/ ) const { return mLastPosition; }
    PositioningMethods supportedPositioningMethods() const { return AllPositioningMethods; }
    int minimumUpdateInterval() const { return 1000; }
    Error error() const { return QGeoPositionInfoSource::NoError; }

  public slots:
    virtual void startUpdates();
    virtual void stopUpdates();

    virtual void requestUpdate( int timeout = 5000 );

  private slots:
    void readNextPosition();

  private:
    QTimer *mTimer;
    QGeoPositionInfo mLastPosition;
    double mAngle;

    double mFlightRadius;
    double mLongitude;
    double mLatitude;
};

/// @endcond

#endif // QGSQUICKSIMULATEDPOSITIONSOURCE_H
