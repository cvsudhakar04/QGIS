/***************************************************************************
                         testqgsprocessingalgs.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"
#include "qgsalgorithmimportphotos.h"

class TestQgsProcessingAlgs: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void packageAlg();
    void renameLayerAlg();
    void loadLayerAlg();
    void parseGeoTags();
    void featureFilterAlg();

  private:

    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

};

void TestQgsProcessingAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QString pointsFileName = dataDir + "/points.shp";
  QFileInfo pointFileInfo( pointsFileName );
  mPointLayerPath = pointFileInfo.filePath();
  mPointsLayer = new QgsVectorLayer( mPointLayerPath,
                                     QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointsLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  QString polysFileName = dataDir + "/polys.shp";
  QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                      QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );
}

void TestQgsProcessingAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingAlgs::packageAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QString outputGpkg = QDir::tempPath() + "/package_alg.gpkg";
  if ( QFile::exists( outputGpkg ) )
    QFile::remove( outputGpkg );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << mPointsLayer->id() << mPolygonLayer->id() );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );
  bool ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context.reset();

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->wkbType(), mPointsLayer->wkbType() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  std::unique_ptr< QgsVectorLayer > polygonLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
}

void TestQgsProcessingAlgs::renameLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:renamelayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name" ) );
  QCOMPARE( results.value( "OUTPUT" ), QVariant::fromValue( layer ) );

  // with input layer name as parameter
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "new name" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name2" ) );
  ok = false;
  results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name2" ) );
  // result should use new name as value
  QCOMPARE( results.value( "OUTPUT" ).toString(), QStringLiteral( "new name2" ) );
}

void TestQgsProcessingAlgs::loadLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:loadlayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "my layer" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !context->layersToLoadOnCompletion().empty() );
  QString layerId = context->layersToLoadOnCompletion().keys().at( 0 );
  QCOMPARE( results.value( QStringLiteral( "OUTPUT" ) ).toString(), layerId );
  QVERIFY( !layerId.isEmpty() );
  QVERIFY( context->temporaryLayerStore()->mapLayer( layerId ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).name, QStringLiteral( "my layer" ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).project, &p );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).outputName, QStringLiteral( "my layer" ) );
}

void TestQgsProcessingAlgs::parseGeoTags()
{
  // parseCoord
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "x" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "1 2 3" ).isValid() );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(36) (13) (15.21)" ).toDouble(), 36.220892, 0.000001 );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(3) (1) (5.21)" ).toDouble(), 3.018114, 0.000001 );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(149) (7) (54.76)" ).toDouble(), 149.131878, 0.000001 );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "(149) (7) (c)" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "(149) (7) ()" ).isValid() );

  // parseMetadataValue
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc" ).toString(), QStringLiteral( "abc" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "(abc)" ).toString(), QStringLiteral( "(abc)" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc (123)" ).toString(), QStringLiteral( "abc (123)" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "(123)" ).toDouble(), 123.0 );

  // parseMetadataList
  QVariantMap md = QgsImportPhotosAlgorithm::parseMetadataList( QStringList() << "EXIF_Contrast=(1)"
                   << "EXIF_ExposureTime=(0.008339)"
                   << "EXIF_Model=Pixel"
                   << "EXIF_GPSLatitude=(36) (13) (15.21)"
                   << "EXIF_GPSLongitude=(149) (7) (54.76)" );
  QCOMPARE( md.count(), 5 );
  QCOMPARE( md.value( "EXIF_Contrast" ).toInt(), 1 );
  QCOMPARE( md.value( "EXIF_ExposureTime" ).toDouble(), 0.008339 );
  QCOMPARE( md.value( "EXIF_Model" ).toString(), QStringLiteral( "Pixel" ) );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLatitude" ).toDouble(), 36.220892, 0.000001 );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLongitude" ).toDouble(), 149.131878, 0.000001 );

  // test extractGeoTagFromMetadata
  md = QVariantMap();
  QgsPointXY point;
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), 142.0 );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), 37.0 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), QStringLiteral( "x" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), 142.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), QStringLiteral( "x" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "E" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "W" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "w" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "...W" ) ); // apparently any string ENDING in W is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QString() );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "N" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "S" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "s" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "...S" ) ); // any string ending in s is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );

  // extractAltitudeFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_GPSAltitude" ), 10.5 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "0" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "00" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "1" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "01" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), 1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), -1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );

  // extractDirectionFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_GPSImgDirection" ), 15 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).toDouble(), 15.0 );

  // extractTimestampFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_DateTimeOriginal" ), QStringLiteral( "xx" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_DateTimeOriginal" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( QStringLiteral( "EXIF_DateTimeOriginal" ) );
  md.insert( QStringLiteral( "EXIF_DateTimeDigitized" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( QStringLiteral( "EXIF_DateTimeDigitized" ) );
  md.insert( QStringLiteral( "EXIF_DateTime" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );

}

void TestQgsProcessingAlgs::featureFilterAlg()
{
  const QgsProcessingAlgorithm *filterAlgTemplate = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:filter" ) );

  Q_ASSERT( filterAlgTemplate->outputDefinitions().isEmpty() );

  QVariantList outputs;
  QVariantMap output1;
  output1.insert( QStringLiteral( "name" ), QStringLiteral( "test" ) );
  output1.insert( QStringLiteral( "expression" ), QStringLiteral( "TRUE" ) );
  output1.insert( QStringLiteral( "isModelOutput" ), true );

  outputs.append( output1 );

  QVariantMap config1;
  config1.insert( QStringLiteral( "outputs" ), outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg( filterAlgTemplate->create( config1 ) );

  QCOMPARE( filterAlg->outputDefinitions().size(), 1 );

  auto outputDef = filterAlg->outputDefinition( QStringLiteral( "OUTPUT_test" ) );
  QCOMPARE( outputDef->type(), QStringLiteral( "outputVector" ) );

  auto outputParamDef = filterAlg->parameterDefinition( "OUTPUT_test" );
  Q_ASSERT( outputParamDef->flags() & QgsProcessingParameterDefinition::FlagIsModelOutput );
  Q_ASSERT( outputParamDef->flags() & QgsProcessingParameterDefinition::FlagHidden );

  QVariantMap output2;
  output2.insert( QStringLiteral( "name" ), QStringLiteral( "nonmodeloutput" ) );
  output2.insert( QStringLiteral( "expression" ), QStringLiteral( "TRUE" ) );
  output2.insert( QStringLiteral( "isModelOutput" ), false );

  outputs.append( output2 );

  QVariantMap config2;
  config2.insert( QStringLiteral( "outputs" ), outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg2( filterAlgTemplate->create( config2 ) );

  QCOMPARE( filterAlg2->outputDefinitions().size(), 2 );

  auto outputDef2 = filterAlg2->outputDefinition( QStringLiteral( "OUTPUT_nonmodeloutput" ) );
  QCOMPARE( outputDef2->type(), QStringLiteral( "outputVector" ) );

  auto outputParamDef2 = filterAlg2->parameterDefinition( "OUTPUT_nonmodeloutput" );
  Q_ASSERT( !outputParamDef2->flags().testFlag( QgsProcessingParameterDefinition::FlagIsModelOutput ) );
  Q_ASSERT( outputParamDef2->flags() & QgsProcessingParameterDefinition::FlagHidden );
}


QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
