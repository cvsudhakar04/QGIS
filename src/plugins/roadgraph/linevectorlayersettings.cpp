/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file linevectorlayersettings.cpp
 * \brief implementation of RgLineVectorLayerSettings
 */

#include "linevectorlayersettings.h"
#include "linevectorlayerwidget.h"

// Qgis includes
#include <qgsproject.h>
#include "qgsmaplayercombobox.h"

// QT includes
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

//standard includes

RgLineVectorLayerSettings::RgLineVectorLayerSettings()
  : mDefaultDirection( QgsVectorLayerDirector::Direction::DirectionBoth )
  , mDefaultSpeed( 40 )
{
}

RgLineVectorLayerSettings::~RgLineVectorLayerSettings()
{
}

bool RgLineVectorLayerSettings::test()
{
  // implement me

  // check default speed
  if ( mDefaultSpeed <= 0.0 )
  {
    return false;
  }
  if ( mLayerName.isEmpty() )
  {
    return false;
  }
  // implement me

  return true;
} // RgLineVectorLayerSettings::test()

void RgLineVectorLayerSettings::read( const QgsProject* project )
{
  mDefaultDirection = static_cast<QgsVectorLayerDirector::Direction>( project->readNumEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/defaultDirection" ) ) );
  mDirection = project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/directionField" ) );
  mFirstPointToLastPointDirectionVal =
    project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/FirstPointToLastPointDirectionVal" ) );
  mLastPointToFirstPointDirectionVal =
    project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/LastPointToFirstPointDirectionVal" ) );
  mBothDirectionVal = project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/BothDirectionVal" ) );
  mSpeed = project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/speedField" ) );
  mDefaultSpeed = project->readDoubleEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/defaultSpeed" ) );
  mLayerName = project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/layer" ) );
  mSpeedUnitName = project->readEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/speedUnitName" ) );
} // RgLineVectorLayerSettings::read( const QgsProject *project )

void RgLineVectorLayerSettings::write( QgsProject* project )
{
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/defaultDirection" ), mDefaultDirection );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/directionField" ),   mDirection );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/FirstPointToLastPointDirectionVal" ),
                       mFirstPointToLastPointDirectionVal );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/LastPointToFirstPointDirectionVal" ),
                       mLastPointToFirstPointDirectionVal );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/BothDirectionVal" ), mBothDirectionVal );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/speedField" ),   mSpeed );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/defaultSpeed" ), mDefaultSpeed );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/layer" ),        mLayerName );
  project->writeEntry( QStringLiteral( "roadgraphplugin" ), QStringLiteral( "/speedUnitName" ),    mSpeedUnitName );
} // RgLineVectorLayerSettings::write( QgsProject *project )

QWidget* RgLineVectorLayerSettings::getGui( QWidget* parent )
{
  return new RgLineVectorLayerSettingsWidget( this, parent );
}

void RgLineVectorLayerSettings::setFromGui( QWidget* myGui )
{
  RgLineVectorLayerSettingsWidget* w = dynamic_cast<RgLineVectorLayerSettingsWidget*>( myGui );
  if ( !w )
    return;

  mFirstPointToLastPointDirectionVal  = w->mleFirstPointToLastPointDirection->text();
  mLastPointToFirstPointDirectionVal  = w->mleLastPointToFirstPointDirection->text();
  mBothDirectionVal                   = w->mleBothDirection->text();
  mDirection                          = w->mcbDirection->currentText();
  mLayerName                          = w->mcbLayers->currentText();

  if ( w->mcbDirectionDefault->currentIndex() == 0 )
  {
    mDefaultDirection = QgsVectorLayerDirector::Direction::DirectionBoth;
  }
  else if ( w->mcbDirectionDefault->currentIndex() == 1 )
  {
    mDefaultDirection = QgsVectorLayerDirector::Direction::DirectionForward;
  }
  else if ( w->mcbDirectionDefault->currentIndex() == 2 )
  {
    mDefaultDirection = QgsVectorLayerDirector::Direction::DirectionBackward;
  }

  mSpeed = w->mcbSpeed->currentText();
  mDefaultSpeed = w->msbSpeedDefault->value();

  if ( w->mcbUnitOfSpeed->currentIndex() == 0 )
  {
    mSpeedUnitName = QStringLiteral( "m/s" );
  }
  else if ( w->mcbUnitOfSpeed->currentIndex() == 1 )
  {
    mSpeedUnitName = QStringLiteral( "km/h" );
  }
}
