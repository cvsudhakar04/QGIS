/***************************************************************************
  qgsquickmaptransform.cpp
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickmaptransform.h"
#include "qgsquickmapsettings.h"

QgsQuickMapTransform::QgsQuickMapTransform()
{
}

QgsQuickMapTransform::~QgsQuickMapTransform()
{
}

void QgsQuickMapTransform::applyTo( QMatrix4x4 *matrix ) const
{
  *matrix *= mMatrix;
  matrix->optimize();
}

QgsQuickMapSettings *QgsQuickMapTransform::mapSettings() const
{
  return mMapSettings;
}

void QgsQuickMapTransform::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mapSettings == mMapSettings )
    return;

  if ( mMapSettings )
    disconnect( mMapSettings, &QgsQuickMapSettings::visibleExtentChanged, this, &QgsQuickMapTransform::updateMatrix );

  mMapSettings = mapSettings;

  if ( mMapSettings )
    connect( mMapSettings, &QgsQuickMapSettings::visibleExtentChanged, this, &QgsQuickMapTransform::updateMatrix );

  emit mapSettingsChanged();
}

void QgsQuickMapTransform::updateMatrix()
{
  QMatrix4x4 matrix;
  float scaleFactor = 1 / mMapSettings->mapUnitsPerPixel();

  matrix.scale( scaleFactor, -scaleFactor );
  matrix.translate( -mMapSettings->visibleExtent().xMinimum(), -mMapSettings->visibleExtent().yMaximum() );

  mMatrix = matrix;
  update();
}
