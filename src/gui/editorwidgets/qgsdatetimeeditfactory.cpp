/***************************************************************************
    qgsdatetimeeditfactory.cpp
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimeeditfactory.h"
#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditwrapper.h"
#include "qgsdatetimesearchwidgetwrapper.h"
#include "qgsdatetimeedit.h"

#include <QSettings>

QgsDateTimeEditFactory::QgsDateTimeEditFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsDateTimeEditFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsDateTimeEditWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper* QgsDateTimeEditFactory::createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDateTimeSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsDateTimeEditFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDateTimeEditConfig( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsDateTimeEditFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QgsEditorWidgetConfig cfg;

  cfg.insert( QStringLiteral( "field_format" ), configElement.attribute( QStringLiteral( "field_format" ) ) );
  cfg.insert( QStringLiteral( "display_format" ), configElement.attribute( QStringLiteral( "display_format" ) ) );
  cfg.insert( QStringLiteral( "calendar_popup" ), configElement.attribute( QStringLiteral( "calendar_popup" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "allow_null" ), configElement.attribute( QStringLiteral( "allow_null" ) ) == QLatin1String( "1" ) );

  return cfg;
}

void QgsDateTimeEditFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  configElement.setAttribute( QStringLiteral( "field_format" ), config[QStringLiteral( "field_format" )].toString() );
  configElement.setAttribute( QStringLiteral( "display_format" ), config[QStringLiteral( "display_format" )].toString() );
  configElement.setAttribute( QStringLiteral( "calendar_popup" ), config[QStringLiteral( "calendar_popup" )].toBool() );
  configElement.setAttribute( QStringLiteral( "allow_null" ), config[QStringLiteral( "allow_null" )].toBool() );
}

QString QgsDateTimeEditFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( cache )

  QString result;

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
  }

  const QgsField field = vl->fields().at( fieldIdx );
  const QString displayFormat = config.value( QStringLiteral( "display_format" ), QgsDateTimeEditConfig::defaultFormat( field.type() ) ).toString();
  const QString fieldFormat = config.value( QStringLiteral( "field_format" ), QgsDateTimeEditConfig::defaultFormat( field.type() ) ).toString();

  QDateTime date = QDateTime::fromString( value.toString(), fieldFormat );

  if ( date.isValid() )
  {
    result = date.toString( displayFormat );
  }
  else
  {
    result = value.toString();
  }

  return result;
}

Qt::AlignmentFlag QgsDateTimeEditFactory::alignmentFlag( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );

  return Qt::AlignLeft;
}

QHash<const char*, int> QgsDateTimeEditFactory::supportedWidgetTypes()
{
  QHash<const char*, int> map = QHash<const char*, int>();
  map.insert( QDateTimeEdit::staticMetaObject.className(), 10 );
  map.insert( QgsDateTimeEdit::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsDateTimeEditFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  const QVariant::Type type = field.type();
  const QgsEditorWidgetConfig config = vl->editFormConfig().widgetConfig( field.name() );
  if ( type == QVariant::DateTime || type == QVariant::Date || type == QVariant::Time || config.contains( QStringLiteral( "field_format" ) ) )
  {
    return 20;
  }
  else
  {
    return 5;
  }
}
