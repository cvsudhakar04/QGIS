/***************************************************************************
  qgspolygon3dsymbolwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspolygon3dsymbolwidget.h"

#include "qgspolygon3dsymbol.h"


QgsPolygon3DSymbolWidget::QgsPolygon3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  spinHeight->setClearValue( 0.0 );
  spinExtrusion->setClearValue( 0.0 );

  setSymbol( QgsPolygon3DSymbol(), nullptr );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboCullingMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( chkInvertNormals, &QCheckBox::clicked, this, &QgsPolygon3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsPolygon3DSymbolWidget::changed );
  connect( btnHeightDD, &QgsPropertyOverrideButton::changed, this, &QgsPolygon3DSymbolWidget::changed );
  connect( btnExtrusionDD, &QgsPropertyOverrideButton::changed, this, &QgsPolygon3DSymbolWidget::changed );
}


static int _cullingModeToIndex( Qt3DRender::QCullFace::CullingMode mode )
{
  switch ( mode )
  {
    case Qt3DRender::QCullFace::NoCulling: return 0;
    case Qt3DRender::QCullFace::Front: return 1;
    case Qt3DRender::QCullFace::Back: return 2;
    case Qt3DRender::QCullFace::FrontAndBack: return 3;
  }
  return 0;
}

static Qt3DRender::QCullFace::CullingMode _cullingModeFromIndex( int index )
{
  switch ( index )
  {
    case 0: return Qt3DRender::QCullFace::NoCulling;
    case 1: return Qt3DRender::QCullFace::Front;
    case 2: return Qt3DRender::QCullFace::Back;
    case 3: return Qt3DRender::QCullFace::FrontAndBack;
  }
  return Qt3DRender::QCullFace::NoCulling;
}


void QgsPolygon3DSymbolWidget::setSymbol( const QgsPolygon3DSymbol &symbol, QgsVectorLayer *layer )
{
  spinHeight->setValue( symbol.height() );
  spinExtrusion->setValue( symbol.extrusionHeight() );
  cboAltClamping->setCurrentIndex( ( int ) symbol.altitudeClamping() );
  cboAltBinding->setCurrentIndex( ( int ) symbol.altitudeBinding() );
  cboCullingMode->setCurrentIndex( _cullingModeToIndex( symbol.cullingMode() ) );
  chkInvertNormals->setChecked( symbol.invertNormals() );
  widgetMaterial->setMaterial( symbol.material() );

  btnHeightDD->init( QgsAbstract3DSymbol::PropertyHeight, symbol.dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );
  btnExtrusionDD->init( QgsAbstract3DSymbol::PropertyExtrusionHeight, symbol.dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );
}

QgsPolygon3DSymbol QgsPolygon3DSymbolWidget::symbol() const
{
  QgsPolygon3DSymbol sym;
  sym.setHeight( spinHeight->value() );
  sym.setExtrusionHeight( spinExtrusion->value() );
  sym.setAltitudeClamping( ( AltitudeClamping ) cboAltClamping->currentIndex() );
  sym.setAltitudeBinding( ( AltitudeBinding ) cboAltBinding->currentIndex() );
  sym.setCullingMode( _cullingModeFromIndex( cboCullingMode->currentIndex() ) );
  sym.setInvertNormals( chkInvertNormals->isChecked() );
  sym.setMaterial( widgetMaterial->material() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::PropertyHeight, btnHeightDD->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::PropertyExtrusionHeight, btnExtrusionDD->toProperty() );
  sym.setDataDefinedProperties( ddp );

  return sym;
}
