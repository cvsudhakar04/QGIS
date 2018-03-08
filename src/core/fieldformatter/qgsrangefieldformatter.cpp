/***************************************************************************
  qgsrangefieldformatter.cpp - QgsRangeFieldFormatter

 ---------------------
 begin                : 01/02/2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLocale>

#include "qgsrangefieldformatter.h"

#include "qgssettings.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"


QString QgsRangeFieldFormatter::id() const
{
  return QStringLiteral( "Range" );
}

QString QgsRangeFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )
  Q_UNUSED( config )

  if ( value.isNull() )
  {
    return QgsApplication::nullRepresentation();
  }

  QString result;

  // Prepare locale
  std::function<QLocale()> f_locale = [ ]
  {
    QLocale locale( QgsApplication::instance()->locale() );
    QLocale::NumberOptions options( locale.numberOptions() );
    options |= QLocale::NumberOption::OmitGroupSeparator;
    locale.setNumberOptions( options );
    return locale;
  };

  const QgsField field = layer->fields().at( fieldIndex );

  if ( field.type() == QVariant::Double &&
       config.contains( QStringLiteral( "Precision" ) ) &&
       value.isValid( ) )
  {
    bool ok;
    double val( value.toDouble( &ok ) );
    if ( ok )
    {
      int precision( config[ QStringLiteral( "Precision" ) ].toInt( &ok ) );
      if ( ok )
      {
        // TODO: make the format configurable!
        result = f_locale().toString( val, 'f', precision );
      }
    }
  }
  else if ( field.type() == QVariant::Int &&
            value.isValid( ) )
  {
    bool ok;
    double val( value.toInt( &ok ) );
    if ( ok )
    {
      result =  f_locale().toString( val );
    }
  }
  else
  {
    result = value.toString();
  }
  return result;
}
