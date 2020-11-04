/***************************************************************************
     testqgspointcloudattribute.cpp
     -------------------
    Date                 : November 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"
#include "qgspointcloudattribute.h"

class TestQgsPointCloudAttribute: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testAttribute();
    void testCollection();

  private:

    QString mTestDataDir;
};

void TestQgsPointCloudAttribute::initTestCase()
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::registerOgrDrivers();
}

void TestQgsPointCloudAttribute::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointCloudAttribute::init()
{

}

void TestQgsPointCloudAttribute::cleanup()
{

}

void TestQgsPointCloudAttribute::testAttribute()
{
  // basic tests
  QgsPointCloudAttribute attribute( QStringLiteral( "name" ), QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.name(), QStringLiteral( "name" ) );
  QCOMPARE( attribute.type(), QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.size(), static_cast< std::size_t >( 4 ) );
}

void TestQgsPointCloudAttribute::testCollection()
{
  // test collections
  QgsPointCloudAttributeCollection collection;
  QVERIFY( collection.attributes().empty() );
  QCOMPARE( collection.pointRecordSize(), static_cast< std::size_t >( 0 ) );
  int offset = 0;
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float ) );
  QCOMPARE( collection.attributes().size(), 1 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.pointRecordSize(), static_cast< std::size_t >( 4 ) );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short ) );
  QCOMPARE( collection.attributes().size(), 2 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.pointRecordSize(), static_cast< std::size_t >( 6 ) );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.attributes().at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( collection.pointRecordSize(), static_cast< std::size_t >( 14 ) );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );

  // populate from other attributes
  QgsPointCloudAttributeCollection collection2( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection2.attributes().size(), 3 );
  QCOMPARE( collection2.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection2.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection2.attributes().at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( collection2.pointRecordSize(), static_cast< std::size_t >( 14 ) );
  QVERIFY( !collection2.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection2.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection2.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection2.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );
}

QGSTEST_MAIN( TestQgsPointCloudAttribute )
#include "testqgspointcloudattribute.moc"
