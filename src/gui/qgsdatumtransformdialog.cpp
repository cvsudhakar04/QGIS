/***************************************************************************
                         qgsdatumtransformdialog.cpp
                         ---------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatumtransformdialog.h"
#include "qgscoordinatetransform.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"

#include <QDir>
#include <QPushButton>

QgsDatumTransformDialog::QgsDatumTransformDialog( const QgsCoordinateReferenceSystem &sourceCrs,
    const QgsCoordinateReferenceSystem &destinationCrs,
    QPair<int, int> selectedDatumTransforms,
    QWidget *parent,
    Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );

  mSourceProjectionSelectionWidget->setCrs( sourceCrs );
  mDestinationProjectionSelectionWidget->setCrs( destinationCrs );

  connect( mHideDeprecatedCheckBox, &QCheckBox::stateChanged, this, &QgsDatumTransformDialog::mHideDeprecatedCheckBox_stateChanged );
  connect( mDatumTransformTreeWidget, &QTreeWidget::currentItemChanged, this, &QgsDatumTransformDialog::mDatumTransformTreeWidget_currentItemChanged );

  connect( mSourceProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setSourceCrs );
  connect( mDestinationProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setDestinationCrs );

  //get list of datum transforms
  mSourceCrs = sourceCrs;
  mDestinationCrs = destinationCrs;
  mDatumTransforms = QgsCoordinateTransform::datumTransformations( sourceCrs, destinationCrs );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  setOKButtonEnabled();

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/DatumTransformDialog/geometry" ) ).toByteArray() );
  mHideDeprecatedCheckBox->setChecked( settings.value( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), false ).toBool() );

  mLabelSrcDescription->clear();
  mLabelDstDescription->clear();

  for ( int i = 0; i < 2; i++ )
  {
    mDatumTransformTreeWidget->setColumnWidth( i, settings.value( QStringLiteral( "Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTreeWidget->columnWidth( i ) ).toInt() );
  }

  load( selectedDatumTransforms );
}

void QgsDatumTransformDialog::load( const QPair<int, int> &selectedDatumTransforms )
{
  mDatumTransformTreeWidget->clear();

  for ( const QgsCoordinateTransform::TransformPair &transform : qgis::as_const( mDatumTransforms ) )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    bool itemDisabled = false;
    bool itemHidden = false;

    for ( int i = 0; i < 2; ++i )
    {
      int nr = i == 0 ? transform.sourceTransformId : transform.destinationTransformId;
      item->setData( i, Qt::UserRole, nr );
      if ( nr == -1 )
        continue;

      item->setText( i, QgsCoordinateTransform::datumTransformToProj( nr ) );

      //Describe datums in a tooltip
      QgsCoordinateTransform::TransformInfo info = QgsCoordinateTransform::datumTransformInfo( nr );
      if ( info.datumTransformId == -1 )
        continue;

      if ( mHideDeprecatedCheckBox->isChecked() && info.deprecated )
      {
        itemHidden = true;
      }

      QString toolTipString;
      if ( gridShiftTransformation( item->text( i ) ) )
      {
        toolTipString.append( QStringLiteral( "<p><b>NTv2</b></p>" ) );
      }

      if ( info.epsgCode > 0 )
        toolTipString.append( QStringLiteral( "<p><b>EPSG Transformations Code:</b> %1</p>" ).arg( info.epsgCode ) );

      toolTipString.append( QStringLiteral( "<p><b>Source CRS:</b> %1</p><p><b>Destination CRS:</b> %2</p>" ).arg( info.sourceCrsDescription, info.destinationCrsDescription ) );

      if ( !info.remarks.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Remarks:</b> %1</p>" ).arg( info.remarks ) );
      if ( !info.scope.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Scope:</b> %1</p>" ).arg( info.scope ) );
      if ( info.preferred )
        toolTipString.append( "<p><b>Preferred transformation</b></p>" );
      if ( info.deprecated )
        toolTipString.append( "<p><b>Deprecated transformation</b></p>" );

      item->setToolTip( i, toolTipString );

      if ( gridShiftTransformation( item->text( i ) ) && !testGridShiftFileAvailability( item, i ) )
      {
        itemDisabled = true;
      }
    }

    if ( !itemHidden )
    {
      item->setDisabled( itemDisabled );
      mDatumTransformTreeWidget->addTopLevelItem( item );
      if ( transform.sourceTransformId == selectedDatumTransforms.first &&
           transform.destinationTransformId == selectedDatumTransforms.second )
      {
        mDatumTransformTreeWidget->setCurrentItem( item );
      }
    }
    else
    {
      delete item;
    }
  }

  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setOKButtonEnabled()
{
  QTreeWidgetItem *item = mDatumTransformTreeWidget->currentItem();
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mSourceCrs.isValid() && mDestinationCrs.isValid() && item );
}

QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), mHideDeprecatedCheckBox->isChecked() );

  for ( int i = 0; i < 2; i++ )
  {
    settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTreeWidget->columnWidth( i ) );
  }

  QApplication::restoreOverrideCursor();
}

int QgsDatumTransformDialog::availableTransformationCount()
{
  return mDatumTransforms.count();
}


QPair<QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int> > QgsDatumTransformDialog::selectedDatumTransforms()
{
  QTreeWidgetItem *item = mDatumTransformTreeWidget->currentItem();
  QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > sdt;
  sdt.first.first = mSourceCrs;
  sdt.second.first = mDestinationCrs;

  if ( item )
  {
    sdt.first.second = item->data( 0, Qt::UserRole ).toInt();
    sdt.second.second = item->data( 1, Qt::UserRole ).toInt();
  }
  else
  {
    sdt.first.second = -1;
    sdt.second.second = -1;
  }
  return sdt;
}

bool QgsDatumTransformDialog::gridShiftTransformation( const QString &itemText ) const
{
  return !itemText.isEmpty() && !itemText.contains( QLatin1String( "towgs84" ), Qt::CaseInsensitive );
}

bool QgsDatumTransformDialog::testGridShiftFileAvailability( QTreeWidgetItem *item, int col ) const
{
  if ( !item )
  {
    return true;
  }

  QString itemText = item->text( col );
  if ( itemText.isEmpty() )
  {
    return true;
  }

  char *projLib = getenv( "PROJ_LIB" );
  if ( !projLib ) //no information about installation directory
  {
    return true;
  }

  QStringList itemEqualSplit = itemText.split( '=' );
  QString filename;
  for ( int i = 1; i < itemEqualSplit.size(); ++i )
  {
    if ( i > 1 )
    {
      filename.append( '=' );
    }
    filename.append( itemEqualSplit.at( i ) );
  }

  QDir projDir( projLib );
  if ( projDir.exists() )
  {
    //look if filename in directory
    QStringList fileList = projDir.entryList();
    QStringList::const_iterator fileIt = fileList.constBegin();
    for ( ; fileIt != fileList.constEnd(); ++fileIt )
    {
#if defined(Q_OS_WIN)
      if ( fileIt->compare( filename, Qt::CaseInsensitive ) == 0 )
#else
      if ( fileIt->compare( filename ) == 0 )
#endif //Q_OS_WIN
      {
        return true;
      }
    }
    item->setToolTip( col, tr( "File '%1' not found in directory '%2'" ).arg( filename, projDir.absolutePath() ) );
    return false; //not found in PROJ_LIB directory
  }
  return true;
}

void QgsDatumTransformDialog::mHideDeprecatedCheckBox_stateChanged( int )
{
  load();
}

void QgsDatumTransformDialog::mDatumTransformTreeWidget_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  if ( !current )
    return;

  mLabelSrcDescription->setText( current->toolTip( 0 ) );
  mLabelDstDescription->setText( current->toolTip( 1 ) );

  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mSourceCrs = sourceCrs;
  mDatumTransforms = QgsCoordinateTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  load();
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
  mDatumTransforms = QgsCoordinateTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  load();
  setOKButtonEnabled();
}
