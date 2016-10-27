/***************************************************************************
    testqgseditorwidgetregistry.cpp
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>

#include "qgseditorwidgetregistry.h"
#include "qgseditorwidgetautoconf.h"


class TestQgsEditorWidgetRegistry: public QObject
{
    Q_OBJECT

    class DummyPlugin: public QgsEditorWidgetAutoConfPlugin
    {
      public:
        QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName, int& score ) const override
        {
          Q_UNUSED( vl )
          if ( fieldName == QLatin1String( "special" ) )
          {
            score = 100;
            return QgsEditorWidgetSetup( QStringLiteral( "Special" ), QgsEditorWidgetConfig() );
          }
          score = 0;
          return QgsEditorWidgetSetup();
        }
    };


  private slots:
    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsEditorWidgetRegistry::initEditors();
      QgsEditorWidgetRegistry::instance()->registerAutoConfPlugin( new DummyPlugin() );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void stringType()
    {
      checkSimple( QStringLiteral( "string" ), QStringLiteral( "TextEdit" ) );
    }

    void datetimeType()
    {
      checkSimple( QStringLiteral( "datetime" ), QStringLiteral( "DateTime" ) );
    }

    void integerType()
    {
      checkSimple( QStringLiteral( "integer" ), QStringLiteral( "Range" ) );
    }

    void longLongType()
    {
      checkSimple( QStringLiteral( "int8" ), QStringLiteral( "TextEdit" ) ); // no current widget supports 64 bit integers => default to TextEdit
    }

    void doubleType()
    {
      checkSimple( QStringLiteral( "double" ), QStringLiteral( "Range" ) );
    }

    void arrayType()
    {
      checkSimple( QStringLiteral( "double[]" ), QStringLiteral( "List" ) );
      checkSimple( QStringLiteral( "int[]" ), QStringLiteral( "List" ) );
      checkSimple( QStringLiteral( "string[]" ), QStringLiteral( "List" ) );
    }

    void configuredType()
    {
      QgsVectorLayer vl( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      QgsEditFormConfig formConfig = vl.editFormConfig();
      formConfig.setWidgetType( QStringLiteral( "col1" ), QStringLiteral( "FooEdit" ) );
      QgsEditorWidgetConfig config;
      config[QStringLiteral( "a" )] = QVariant( 12 );
      config[QStringLiteral( "b" )] = QVariant( "bar" );
      formConfig.setWidgetConfig( QStringLiteral( "col1" ), config );
      vl.setEditFormConfig( formConfig );
      const QgsEditorWidgetSetup setup = QgsEditorWidgetRegistry::instance()->findBest( &vl, QStringLiteral( "col1" ) );
      QCOMPARE( setup.type(), QString( "FooEdit" ) );
      QCOMPARE( setup.config(), config );
    }

    void wrongFieldName()
    {
      const QgsVectorLayer vl( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      const QgsEditorWidgetSetup setup = QgsEditorWidgetRegistry::instance()->findBest( &vl, QStringLiteral( "col2" ) );
      // an unknown fields leads to a default setup with a TextEdit
      QCOMPARE( setup.type(), QString( "TextEdit" ) );
      QCOMPARE( setup.config().count(), 0 );
    }

    void typeFromPlugin()
    {
      const QgsVectorLayer vl( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=special:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      const QgsEditorWidgetSetup setup = QgsEditorWidgetRegistry::instance()->findBest( &vl, QStringLiteral( "special" ) );
      QCOMPARE( setup.type(), QString( "Special" ) );
    }

  private:

    static void checkSimple( const QString& dataType, const QString& widgetType )
    {
      const QgsVectorLayer vl( "LineString?crs=epsg:3111&field=pk:int&field=col1:" + dataType, QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      const QgsEditorWidgetSetup setup = QgsEditorWidgetRegistry::instance()->findBest( &vl, QStringLiteral( "col1" ) );
      QCOMPARE( setup.type(), widgetType );
      QCOMPARE( setup.config().count(), 0 );
    }
};

QTEST_MAIN( TestQgsEditorWidgetRegistry )
#include "testqgseditorwidgetregistry.moc"
