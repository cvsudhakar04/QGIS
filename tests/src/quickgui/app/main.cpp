/***************************************************************************
  main.cpp
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

#include <QtDebug>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlError>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgslayertree.h"
#include "qgsmessagelog.h"
#include "qgsquickutils.h"


int main( int argc, char *argv[] )
{
  // 1) Initialize QGIS
  QgsApplication app( argc, argv, true );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QgsQuick Test App" );
  QCoreApplication::setApplicationVersion( Qgis::QGIS_VERSION );

  QCommandLineParser parser;
  parser.addVersionOption();
  parser.process( app );

  QgsApplication::init();
  QgsApplication::initQgis();

  // 2) Load QGIS Project
  QString dataDir( TEST_DATA_DIR );  // defined in CMakeLists.txt
  QString projectFile = dataDir + "/quickapp_project.qgs";
  qDebug() << "project file: " << projectFile;
  QgsProject project;
  bool res = project.read( projectFile );
  Q_ASSERT( res );

  QQmlEngine engine;
  engine.addImportPath( QgsApplication::qmlImportPath() );
  engine.rootContext()->setContextProperty( "__project", &project );
  engine.rootContext()->setContextProperty( "__layers", QVariant::fromValue( project.layerTreeRoot()->layerOrder() ) );

  // Set simulated position for desktop builds
  bool use_simulated_position = true;
  engine.rootContext()->setContextProperty( "__use_simulated_position", use_simulated_position );

  QQmlComponent component( &engine, QUrl( QStringLiteral( "qrc:/main.qml" ) ) );
  QObject *object = component.create();

  if ( !component.errors().isEmpty() )
  {
    qDebug( "%s", QgsApplication::showSettings().toLocal8Bit().data() );

    qDebug() << "****************************************";
    qDebug() << "*****        QML errors:           *****";
    qDebug() << "****************************************";
    for ( const QQmlError &error : component.errors() )
    {
      qDebug() << "  " << error;
    }
    qDebug() << "****************************************";
    qDebug() << "****************************************";
  }

  if ( object == 0 )
  {
    qDebug() << "FATAL ERROR: unable to create main.qml";
    return EXIT_FAILURE;
  }

  // Add some data for debugging if needed
  QgsApplication::messageLog()->logMessage( QgsQuickUtils::instance()->dumpScreenInfo() );
  QgsApplication::messageLog()->logMessage( "data directory: " + dataDir );
  QgsApplication::messageLog()->logMessage( "All up and running" );

  return app.exec();
}

