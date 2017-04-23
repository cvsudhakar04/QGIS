/***************************************************************************
    qgssublayersdialog.cpp  - dialog for selecting sublayers
    ---------------------
    begin                : January 2009
    copyright            : (C) 2009 by Florian El Ahdab
    email                : felahdab at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssublayersdialog.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QTableWidgetItem>
#include <QPushButton>


QgsSublayersDialog::QgsSublayersDialog( ProviderType providerType, const QString &name,
                                        QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mName( name )
  , mProviderType( providerType )
{
  setupUi( this );

  if ( mProviderType == QgsSublayersDialog::Ogr )
  {
    setWindowTitle( tr( "Select vector layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Number of features" ) << tr( "Geometry type" ) );
    mShowCount = true;
    mShowType = true;
  }
  else if ( mProviderType == QgsSublayersDialog::Gdal )
  {
    setWindowTitle( tr( "Select raster layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) );
  }
  else
  {
    setWindowTitle( tr( "Select layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Type" ) );
    mShowType = true;
  }

  // add a "Select All" button - would be nicer with an icon
  QPushButton *button = new QPushButton( tr( "Select All" ) );
  buttonBox->addButton( button, QDialogButtonBox::ActionRole );
  connect( button, SIGNAL( pressed() ), layersTable, SLOT( selectAll() ) );
  // connect( pbnSelectNone, SIGNAL( pressed() ), SLOT( layersTable->selectNone() ) );

  QgsSettings settings;
  restoreGeometry( settings.value( "/Windows/" + mName + "SubLayers/geometry" ).toByteArray() );

  // Checkbox about adding sublayers to a group
  if ( mShowAddToGroupCheckbox )
  {
    mCheckboxAddToGroup = new QCheckBox( tr( "Add layers to a group" ) );
    bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), false ).toBool();
    mCheckboxAddToGroup->setChecked( addToGroup );
    buttonBox->addButton( mCheckboxAddToGroup, QDialogButtonBox::ActionRole );
  }
}

QgsSublayersDialog::~QgsSublayersDialog()
{
  QgsSettings settings;
  settings.setValue( "/Windows/" + mName + "SubLayers/geometry", saveGeometry() );
  settings.setValue( "/Windows/" + mName + "SubLayers/headerState",
                     layersTable->header()->saveState() );
}

static bool _isLayerIdUnique( int layerId, QTreeWidget *layersTable )
{
  int count = 0;
  for ( int j = 0; j < layersTable->topLevelItemCount(); j++ )
  {
    if ( layersTable->topLevelItem( j )->text( 0 ).toInt() == layerId )
    {
      count++;
    }
  }
  return count == 1;
}

QgsSublayersDialog::LayerDefinitionList QgsSublayersDialog::selection()
{
  LayerDefinitionList list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    QTreeWidgetItem *item = layersTable->selectedItems().at( i );

    LayerDefinition def;
    def.layerId = item->text( 0 ).toInt();
    def.layerName = item->text( 1 );
    if ( mShowType )
    {
      // If there are more sub layers of the same name (virtual for geometry types),
      // add geometry type
      if ( !_isLayerIdUnique( def.layerId, layersTable ) )
        def.type = item->text( mShowCount ? 3 : 2 );
    }
    if ( mProviderType == QgsSublayersDialog::Ogr )
    {
      // Ogr-Internal use only - no needed to show to user
      def.count = item->text( 2 ).toInt();
      if ( item->text( 3 ) != "Unknown" )
      {
        def.type = item->text( 3 );
      }
      def.geometryName = item->text( mShowCount ? 4 : 3 );
      def.geometryId = item->text( mShowCount ? 5 : 4 ).toInt();
      def.retrievalMethod = item->text( mShowCount ? 6 : 5 ).toInt();
    }

    list << def;
  }
  return list;
}


void QgsSublayersDialog::populateLayerTable( const QgsSublayersDialog::LayerDefinitionList &list )
{
  Q_FOREACH ( const LayerDefinition &item, list )
  {
    QStringList elements;
    elements << QString::number( item.layerId ) << item.layerName;
    if ( mShowCount )
      elements << QString::number( item.count );
    if ( mShowType )
      elements << item.type;
    if ( mProviderType == QgsSublayersDialog::Ogr )
    {
      //  layer_id:layer_name:feature_count:geometry_type:geometry_name:field_geometry_id:ogr_get_type
      elements << item.geometryName;
      elements << QString::number( item.geometryId );
      elements << QString::number( item.retrievalMethod );
    }
    layersTable->addTopLevelItem( new QTreeWidgetItem( elements ) );
  }

  // resize columns
  QgsSettings settings;
  QByteArray ba = settings.value( "/Windows/" + mName + "SubLayers/headerState" ).toByteArray();
  if ( ! ba.isNull() )
  {
    layersTable->header()->restoreState( ba );
  }
  else
  {
    for ( int i = 0; i < layersTable->columnCount(); i++ )
      layersTable->resizeColumnToContents( i );
    layersTable->setColumnWidth( 1, layersTable->columnWidth( 1 ) + 10 );
  }
}

// override exec() instead of using showEvent()
// because in some case we don't want the dialog to appear (depending on user settings)
// TODO alert the user when dialog is not opened
int QgsSublayersDialog::exec()
{
  QgsSettings settings;
  QString promptLayers = settings.value( QStringLiteral( "qgis/promptForSublayers" ), 1 ).toString();

  // make sure three are sublayers to choose
  if ( layersTable->topLevelItemCount() == 0 )
    return QDialog::Rejected;

  // check promptForSublayers settings - perhaps this should be in QgsDataSource instead?
  if ( promptLayers == QLatin1String( "no" ) )
    return QDialog::Rejected;
  else if ( promptLayers == QLatin1String( "all" ) )
  {
    layersTable->selectAll();
    return QDialog::Accepted;
  }

  // if there is only 1 sublayer (probably the main layer), just select that one and return
  if ( layersTable->topLevelItemCount() == 1 )
  {
    layersTable->selectAll();
    return QDialog::Accepted;
  }

  layersTable->sortByColumn( 1, Qt::AscendingOrder );
  layersTable->setSortingEnabled( true );

  // if we got here, disable override cursor, open dialog and return result
  // TODO add override cursor where it is missing (e.g. when opening via "Add Raster")
  QCursor cursor;
  bool overrideCursor = nullptr != QApplication::overrideCursor();
  if ( overrideCursor )
  {
    cursor = QCursor( * QApplication::overrideCursor() );
    QApplication::restoreOverrideCursor();
  }
  int ret = QDialog::exec();
  if ( overrideCursor )
    QApplication::setOverrideCursor( cursor );

  if ( mCheckboxAddToGroup )
    settings.setValue( QStringLiteral( "/qgis/openSublayersInGroup" ), mCheckboxAddToGroup->isChecked() );
  return ret;
}
