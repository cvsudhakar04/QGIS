/***************************************************************************
  qgsquickscalebarkit.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKSCALEBARKIT_H
#define QGSQUICKSCALEBARKIT_H

#include <QObject>
#include <QString>

#include "qgis_quick.h"

class QgsQuickMapSettings;

/**
 * \ingroup quick
 *
 * The class QgsQuickScaleBarKit encapsulates the utilies to calculate
 * scale bar properties
 *
 * It requires connection to mapSettings of the active canvas to automatically
 * update text and width
 *
 * From preferred width in pixel, it calculates the width (pixel) of scalebar
 * distance in meters or kilometers (int) rounded to "nice" number (e.g. 72.4 to 100)
 * and units text (e.g. km)
 *
 * \note QML Type: ScaleBarKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickScaleBarKit : public QObject
{
    Q_OBJECT

    /**
      * Associated map settings
      */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
      * Preferred width of scalebar in pixels. Defaults to 300
      */
    Q_PROPERTY( int preferredWidth MEMBER mPreferredWidth NOTIFY preferredWidthChanged )

    /**
      * Units of distance (e.g. km or m) Read-only (result)
      */
    Q_PROPERTY( QString units READ units NOTIFY scaleBarChanged )

    /**
      * Distance rounded to "nice" number (e.g. 100, 20). To be used with units property for labels. Read-only (result)
      */
    Q_PROPERTY( int distance READ distance NOTIFY scaleBarChanged )

    /**
      * Calculated width of scalebar in pixels representing distance + units. Differs minimum possible from preferredWidth to
      * get "nice" distance number.
      */
    Q_PROPERTY( int width READ width NOTIFY scaleBarChanged )

  public:
    //! create new scale bar kit
    explicit QgsQuickScaleBarKit( QObject *parent = nullptr );
    ~QgsQuickScaleBarKit() = default;

    //! Set map settings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! Returns calculated width in pixels
    int width() const;

    /**
     * Returns distance corensponding to width
     *
     * \see QgsQuickScaleBarKit::units()
     */
    int distance() const;

    /**
     * Returns units of distance (m, km)
     *
     * \see QgsQuickScaleBarKit::distance()
     */
    QString units() const;

  signals:
    //! width, distance and/or units changed
    void scaleBarChanged();

    //! map settings changed
    void mapSettingsChanged();

    //! preferred width changed
    void preferredWidthChanged();

  public slots:
    //! recalculate width, distance and units
    void updateScaleBar();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr;

    int mPreferredWidth; // pixels
    int mWidth; // pixels
    int mDistance; // in meters or kilometers, rounded
    QString mUnits; // km or m
};


#endif // QGSQUICKSCALEBARKIT_H
