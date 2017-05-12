/***************************************************************************
  qgsosmdownloaddialog.cpp
  --------------------------------------
  Date                 : February 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsosmdownloaddialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "qgis.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransform.h"
#include "qgssettings.h"

#include "qgsosmdownload.h"

QgsOSMDownloadDialog::QgsOSMDownloadDialog( QWidget *parent )
  : QDialog( parent )
  , mDownload( new QgsOSMDownload )
{
  setupUi( this );

  editXMin->setValidator( new QDoubleValidator( -180.0, 180.0, 6, this ) );
  editXMax->setValidator( new QDoubleValidator( -180.0, 180.0, 6, this ) );
  editYMin->setValidator( new QDoubleValidator( -90.0, 90.0, 6, this ) );
  editYMax->setValidator( new QDoubleValidator( -90.0, 90.0, 6, this ) );

  populateLayers();
  onExtentCanvas();

  connect( radExtentCanvas, &QAbstractButton::clicked, this, &QgsOSMDownloadDialog::onExtentCanvas );
  connect( radExtentLayer, &QAbstractButton::clicked, this, &QgsOSMDownloadDialog::onExtentLayer );
  connect( radExtentManual, &QAbstractButton::clicked, this, &QgsOSMDownloadDialog::onExtentManual );
  connect( cboLayers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOSMDownloadDialog::onCurrentLayerChanged );
  connect( btnBrowse, &QAbstractButton::clicked, this, &QgsOSMDownloadDialog::onBrowseClicked );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsOSMDownloadDialog::onOK );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsOSMDownloadDialog::onClose );

  connect( mDownload, &QgsOSMDownload::finished, this, &QgsOSMDownloadDialog::onFinished );
  connect( mDownload, &QgsOSMDownload::downloadProgress, this, &QgsOSMDownloadDialog::onDownloadProgress );
}

QgsOSMDownloadDialog::~QgsOSMDownloadDialog()
{
  delete mDownload;
}


void QgsOSMDownloadDialog::populateLayers()
{
  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  QMap<QString, QgsMapLayer *>::iterator it;
  for ( it = layers.begin(); it != layers.end(); ++it )
  {
    cboLayers->addItem( it.value()->name(), it.key() );
  }
  cboLayers->setCurrentIndex( 0 );
}

void QgsOSMDownloadDialog::setRect( const QgsRectangle &rect )
{
  // these coords should be already lat/lon
  editXMin->setText( QString::number( rect.xMinimum() ) );
  editXMax->setText( QString::number( rect.xMaximum() ) );
  editYMin->setText( QString::number( rect.yMinimum() ) );
  editYMax->setText( QString::number( rect.yMaximum() ) );
}

QgsRectangle QgsOSMDownloadDialog::rect() const
{
  return QgsRectangle( editXMin->text().toDouble(), editYMin->text().toDouble(),
                       editXMax->text().toDouble(), editYMax->text().toDouble() );
}


void QgsOSMDownloadDialog::setRectReadOnly( bool readonly )
{
  editXMin->setReadOnly( readonly );
  editXMax->setReadOnly( readonly );
  editYMin->setReadOnly( readonly );
  editYMax->setReadOnly( readonly );
}


void QgsOSMDownloadDialog::onExtentCanvas()
{
  QgsRectangle r( QgisApp::instance()->mapCanvas()->extent() );

  QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );

  QgsCoordinateTransform ct( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs(), dst );
  r = ct.transformBoundingBox( r );
  if ( !r.isFinite() )
  {
    QMessageBox::information( this, tr( "OpenStreetMap download" ), tr( "Could not transform canvas extent." ) );
    return;
  }

  setRect( r );
  setRectReadOnly( true );
  cboLayers->setEnabled( false );
}

void QgsOSMDownloadDialog::onExtentLayer()
{
  onCurrentLayerChanged( cboLayers->currentIndex() );
  setRectReadOnly( true );
  cboLayers->setEnabled( true );
}

void QgsOSMDownloadDialog::onExtentManual()
{
  setRectReadOnly( false );
  cboLayers->setEnabled( false );
}

void QgsOSMDownloadDialog::onCurrentLayerChanged( int index )
{
  if ( index < 0 )
    return;

  QString layerId = cboLayers->itemData( index ).toString();
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  if ( !layer )
    return;

  QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );

  QgsCoordinateTransform ct( layer->crs(), dst );
  QgsRectangle rect( ct.transformBoundingBox( layer->extent() ) );
  if ( rect.isFinite() )
    setRect( rect );
  else
    QMessageBox::information( this, tr( "OpenStreetMap download" ), tr( "Could not transform layer extent." ) );
}

void QgsOSMDownloadDialog::onBrowseClicked()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "osm/lastDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getSaveFileName( this, QString(), lastDir, tr( "OpenStreetMap files (*.osm)" ) );
  if ( fileName.isNull() )
    return;

  settings.setValue( QStringLiteral( "osm/lastDir" ), QFileInfo( fileName ).absolutePath() );
  editFileName->setText( fileName );
}

void QgsOSMDownloadDialog::onOK()
{
  mDownload->setQuery( QgsOSMDownload::queryFromRect( rect() ) );
  mDownload->setOutputFileName( editFileName->text() );
  if ( !mDownload->start() )
  {
    QMessageBox::critical( this, tr( "Download error" ), mDownload->errorString() );
    return;
  }

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  progress->setRange( 0, 0 ); // this will start animating progress bar
}

void QgsOSMDownloadDialog::onClose()
{
  if ( !mDownload->isFinished() )
  {
    int res = QMessageBox::question( this, tr( "OpenStreetMap download" ),
                                     tr( "Would you like to abort download?" ), QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      return;
  }

  reject();
}

void QgsOSMDownloadDialog::onFinished()
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  progress->setRange( 0, 1 );

  if ( mDownload->hasError() )
  {
    QMessageBox::critical( this, tr( "OpenStreetMap download" ), tr( "Download failed.\n%1" ).arg( mDownload->errorString() ) );
  }
  else
  {
    QMessageBox::information( this, tr( "OpenStreetMap download" ), tr( "Download has been successful." ) );
  }
}

void QgsOSMDownloadDialog::onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  Q_UNUSED( bytesTotal ); // it's -1 anyway (= unknown)
  double mbytesReceived = ( double )bytesReceived / ( 1024 * 1024 );
  editSize->setText( QStringLiteral( "%1 MB" ).arg( QString::number( mbytesReceived, 'f', 1 ) ) );
}
