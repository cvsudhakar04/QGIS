/***************************************************************************
  qgssettings.cpp
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>

#include "qgssettings.h"
#include <QFileInfo>
#include <QSettings>
#include <QDir>

QString QgsSettings::sGlobalSettingsPath = QString();

bool QgsSettings::setGlobalSettingsPath( QString path )
{
  if ( QFileInfo::exists( path ) )
  {
    sGlobalSettingsPath = path;
    return true;
  }
  return false;
}

void QgsSettings::init()
{
  if ( ! sGlobalSettingsPath.isEmpty( ) )
  {
    mGlobalSettings = new QSettings( sGlobalSettingsPath, QSettings::IniFormat );
    mGlobalSettings->setIniCodec( "UTF-8" );
  }
}


QgsSettings::QgsSettings( const QString &organization, const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( organization, application, parent );
  init( );
}

QgsSettings::QgsSettings( QSettings::Scope scope, const QString &organization,
                          const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( scope, organization, application, parent );
  init( );
}

QgsSettings::QgsSettings( QSettings::Format format, QSettings::Scope scope,
                          const QString &organization, const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( format, scope, organization, application, parent );
  init( );
}

QgsSettings::QgsSettings( const QString &fileName, QSettings::Format format, QObject *parent )
{
  mUserSettings = new QSettings( fileName, format, parent );
  init( );
}

QgsSettings::QgsSettings( QObject *parent )
{
  mUserSettings = new QSettings( parent );
  init( );
}

QgsSettings::~QgsSettings()
{
  delete mUserSettings;
  delete mGlobalSettings;
}


void QgsSettings::beginGroup( const QString &prefix )
{
  mUserSettings->beginGroup( prefix );
  if ( mGlobalSettings )
  {
    mGlobalSettings->beginGroup( prefix );
  }
}

void QgsSettings::endGroup()
{
  mUserSettings->endGroup( );
  if ( mGlobalSettings )
  {
    mGlobalSettings->endGroup( );
  }
}


QStringList QgsSettings::allKeys() const
{
  QStringList keys = mUserSettings->allKeys( );
  if ( mGlobalSettings )
  {
  for ( auto &s : mGlobalSettings->allKeys() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}


QStringList QgsSettings::childKeys() const
{
  QStringList keys = mUserSettings->childKeys( );
  if ( mGlobalSettings )
  {
  for ( auto &s : mGlobalSettings->childKeys() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}

QStringList QgsSettings::childGroups() const
{
  QStringList keys = mUserSettings->childGroups( );
  if ( mGlobalSettings )
  {
  for ( auto &s : mGlobalSettings->childGroups() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}

QVariant QgsSettings::value( const QString &key, const QVariant &defaultValue, const QgsSettings::Section section ) const
{
  QString pKey = prefixedKey( key, section );
  if ( ! mUserSettings->value( pKey ).isNull() )
  {
    return mUserSettings->value( pKey );
  }
  if ( mGlobalSettings )
  {
    return mGlobalSettings->value( pKey, defaultValue );
  }
  return defaultValue;
}

bool QgsSettings::contains( const QString &key ) const
{
  return mUserSettings->contains( key ) ||
         ( mGlobalSettings && mGlobalSettings->contains( key ) );
}

QString QgsSettings::fileName() const
{
  return mUserSettings->fileName( );
}

void QgsSettings::sync()
{
  return mUserSettings->sync( );
}

void QgsSettings::remove( const QString &key )
{
  mGlobalSettings->remove( key );
}

QString QgsSettings::prefixedKey( const QString &key, const Section section ) const
{
  QString prefix;
  switch ( section )
  {
    case Section::Core :
      prefix = "core";
      break;
    case Section::Server :
      prefix = "server";
      break;
    case Section::Gui :
      prefix = "gui";
      break;
    case Section::Plugins :
      prefix = "plugins";
      break;
    case Section::Misc :
      prefix = "misc";
      break;
    case Section::NoSection:
    default:
      return sanitizeKey( key );
  }
  return prefix  + "/" + sanitizeKey( key );
}


int QgsSettings::beginReadArray( const QString &prefix )
{
  int size = mUserSettings->beginReadArray( prefix );
  if ( 0 == size && mGlobalSettings )
  {
    size = mGlobalSettings->beginReadArray( prefix );
    mUsingGlobalArray = ( size > 0 );
  }
  return size;
}

void QgsSettings::endArray()
{
  mUserSettings->endArray();
  if ( mGlobalSettings )
  {
    mGlobalSettings->endArray();
  }
  mUsingGlobalArray = false;
}

void QgsSettings::setArrayIndex( int i )
{
  if ( mGlobalSettings && mUsingGlobalArray )
  {
    mGlobalSettings->setArrayIndex( i );
  }
  else
  {
    mUserSettings->setArrayIndex( i );
  }
}

void QgsSettings::setValue( const QString &key, const QVariant &value , const QgsSettings::Section section )
{
  // TODO: add valueChanged signal
  mUserSettings->setValue( prefixedKey( key, section ), value );
}

// To lower case and clean the path
QString QgsSettings::sanitizeKey( QString key ) const
{
  return QDir::cleanPath( key.toLower() );
}
