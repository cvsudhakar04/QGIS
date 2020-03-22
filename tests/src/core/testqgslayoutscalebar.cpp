
/***************************************************************************
                         testqgslayoutscalebar.cpp
                         ---------------------------
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

#include "qgsapplication.h"
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsfontutils.h"
#include "qgsproperty.h"
#include "qgsproject.h"
#include "qgspallabeling.h"
#include "qgsbasicnumericformat.h"
#include "qgslinesymbollayer.h"
#include "qgslayoutmanager.h"
#include "qgsprintlayout.h"
#include <QLocale>
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutScaleBar : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutScaleBar() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void singleBox();
    void singleBoxLineSymbol();
    void singleBoxLabelBelowSegment();
    void singleBoxAlpha();
    void doubleBox();
    void doubleBoxLineSymbol();
    void doubleBoxLabelCenteredSegment();
    void numeric();
    void tick();
    void tickLineSymbol();
    void dataDefined();
    void oldDataDefinedProject();
    void textFormat();
    void numericFormat();

  private:
    QString mReport;
};

void TestQgsLayoutScaleBar::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // the scale denominator is formatted in a locale aware manner
  // so 10000 is rendered as "10,000" in C (or en_US) locale, however
  // other locales may render the number differently (e.g. "10 000" in cs_CZ)
  // so we enforce C locale to make sure we get expected result
  QLocale::setDefault( QLocale::c() );

  //reproject to WGS84
  QgsCoordinateReferenceSystem destCRS( QStringLiteral( "EPSG:4326" ) );
  QgsProject::instance()->setCrs( destCRS );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );

  mReport = QStringLiteral( "<h1>Layout Scalebar Tests</h1>\n" );
}

void TestQgsLayoutScaleBar::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsLayoutScaleBar::init()
{

}

void TestQgsLayoutScaleBar::cleanup()
{

}

void TestQgsLayoutScaleBar::singleBox()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::singleBoxLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr< QgsLineSymbol > lineSymbol = qgis::make_unique< QgsLineSymbol >();
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox_linesymbol" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::singleBoxLabelBelowSegment()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 18 ) ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setLabelVerticalPlacement( QgsScaleBarSettings::LabelBelowSegment );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox_labelbelowsegment" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::singleBoxAlpha()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setColor( QColor( 255, 0, 255 ) );
  format.setOpacity( 100.0 / 255 );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  scalebar->setFillColor( QColor( 255, 0, 0, 100 ) );
  scalebar->setFillColor2( QColor( 0, 255, 0, 50 ) );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineColor( QColor( 0, 0, 255, 150 ) );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox_alpha" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::doubleBox()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setColor( Qt::black );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Double Box" ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_doublebox" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::doubleBoxLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr< QgsLineSymbol > lineSymbol = qgis::make_unique< QgsLineSymbol >();
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Double Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_doublebox_linesymbol" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::doubleBoxLabelCenteredSegment()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 18 ) );
  format.setColor( Qt::black );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 3 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Double Box" ) );

  scalebar->setLabelVerticalPlacement( QgsScaleBarSettings::LabelBelowSegment );
  scalebar->setLabelHorizontalPlacement( QgsScaleBarSettings::LabelCenteredSegment );
  scalebar->setUnitLabel( QStringLiteral( "units" ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_doublebox_labelcenteredsegment" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::numeric()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 0 );

  scalebar->setStyle( QStringLiteral( "Numeric" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_numeric" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  bool result = checker.testLayout( mReport, 0, 0 );
  QVERIFY( result );
}

void TestQgsLayoutScaleBar::tick()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_tick" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::tickLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr< QgsLineSymbol > lineSymbol = qgis::make_unique< QgsLineSymbol >();
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_tick_linesymbol" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::dataDefined()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr< QgsLineSymbol > lineSymbol = qgis::make_unique< QgsLineSymbol >();
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( 1 );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setColor( QColor( 0, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
  scalebar->setLineSymbol( lineSymbol.release() );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 0 );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  // this is the deprecated way of doing this -- the new way is using data defined properties on the scalebar line symbol.
  // so this test is to ensure old projects/api use works correctly
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor, QgsProperty::fromExpression( QStringLiteral( "'red'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor2, QgsProperty::fromExpression( QStringLiteral( "'blue'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineColor, QgsProperty::fromExpression( QStringLiteral( "'yellow'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineWidth, QgsProperty::fromExpression( QStringLiteral( "1.2*3" ) ) );
  scalebar->refreshDataDefinedProperty();

  QgsLayoutChecker checker2( QStringLiteral( "layoutscalebar_datadefined" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker2.testLayout( mReport, 0, 0 ) );

  QCOMPARE( scalebar->brush().color().name(), QColor( 255, 0, 0 ).name() );
  QCOMPARE( scalebar->brush2().color().name(), QColor( 0, 0, 255 ).name() );
}

void TestQgsLayoutScaleBar::oldDataDefinedProject()
{
    QgsProject project;
    // read a project with the older data defined line width and color
    project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/scalebar_old_datadefined.qgs" );
    QgsLayout* l = project.layoutManager()->printLayouts().at( 0 );
    QList< QgsLayoutItemScaleBar* > scaleBars;
    l->layoutItems( scaleBars );
    QgsLayoutItemScaleBar* scaleBar = scaleBars.at( 0 );

    // ensure the deprecated scalebar datadefined properties were automatically copied to the scalebar's line symbol
    QgsLineSymbol* ls = scaleBar->lineSymbol();
    QgsSimpleLineSymbolLayer* sll = dynamic_cast< QgsSimpleLineSymbolLayer* >( ls->symbolLayer( 0 ) );

    QVERIFY( sll->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth ).isActive() );
    QCOMPARE( sll->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth).asExpression(), QStringLiteral("3"));
    QVERIFY( sll->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeColor ).isActive() );
    QCOMPARE( sll->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeColor).asExpression(), QStringLiteral("'red'"));

    // deprecated properties should be gone
    QVERIFY( !scaleBar->dataDefinedProperties().property( QgsLayoutObject::ScalebarLineColor ).isActive() );
    QVERIFY( !scaleBar->dataDefinedProperties().property( QgsLayoutObject::ScalebarLineWidth ).isActive() );
}

void TestQgsLayoutScaleBar::textFormat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 16 );
  format.dataDefinedProperties().setProperty( QgsPalLayerSettings::Color, QgsProperty::fromExpression( QStringLiteral( "case when @scale_value = 2000 then '#ff00ff' else '#ffff00' end" ) ) );
  scalebar->setTextFormat( format );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_textformat" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutScaleBar::numericFormat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowThousandsSeparator( true );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setShowPlusSign( true );
  dynamic_cast< QgsBasicNumericFormat *>( const_cast< QgsNumericFormat * >( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 1 );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_numericformat" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}


QGSTEST_MAIN( TestQgsLayoutScaleBar )
#include "testqgslayoutscalebar.moc"
