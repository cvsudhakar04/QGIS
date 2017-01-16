/***************************************************************************
                     qgsfeatureaction.cpp  -  description
                              -------------------
      begin                : 2010-09-20
      copyright            : (C) 2010 by Juergen E. Fischer
      email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsattributedialog.h"
#include "qgsdistancearea.h"
#include "qgsfeatureaction.h"
#include "qgsguivectorlayertools.h"
#include "qgsidentifyresultsdialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsactionmanager.h"
#include "qgsaction.h"
#include "qgsvectorlayerutils.h"

#include <QPushButton>
#include <QSettings>

QgsFeatureAction::QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *layer, const QUuid& actionId, int defaultAttr, QObject *parent )
    : QAction( name, parent )
    , mLayer( layer )
    , mFeature( &f )
    , mActionId( actionId )
    , mIdx( defaultAttr )
    , mFeatureSaved( false )
{
}

void QgsFeatureAction::execute()
{
  mLayer->actions()->doAction( mActionId, *mFeature, mIdx );
}

QgsAttributeDialog *QgsFeatureAction::newDialog( bool cloneFeature )
{
  QgsFeature *f = cloneFeature ? new QgsFeature( *mFeature ) : mFeature;

  QgsAttributeEditorContext context;

  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs() );
  myDa.setEllipsoidalMode( true );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  context.setDistanceArea( myDa );
  context.setVectorLayerTools( QgisApp::instance()->vectorLayerTools() );
  context.setFormMode( QgsAttributeEditorContext::StandaloneDialog );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( mLayer, f, cloneFeature, parentWidget(), true, context );
  dialog->setWindowFlags( dialog->windowFlags() | Qt::Tool );

  QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Feature" ) );
  if ( !actions.isEmpty() )
  {
    dialog->setContextMenuPolicy( Qt::ActionsContextMenu );

    QAction *a = new QAction( tr( "Run actions" ), dialog );
    a->setEnabled( false );
    dialog->addAction( a );

    Q_FOREACH ( const QgsAction& action, actions )
    {
      if ( !action.runable() )
        continue;

      QgsFeature& feat = const_cast<QgsFeature&>( *dialog->feature() );
      QgsFeatureAction *a = new QgsFeatureAction( action.name(), feat, mLayer, action.id(), -1, dialog );
      dialog->addAction( a );
      connect( a, SIGNAL( triggered() ), a, SLOT( execute() ) );

      QAbstractButton *pb = dialog->findChild<QAbstractButton *>( action.name() );
      if ( pb )
        connect( pb, SIGNAL( clicked() ), a, SLOT( execute() ) );
    }
  }

  return dialog;
}

bool QgsFeatureAction::viewFeatureForm( QgsHighlight *h )
{
  if ( !mLayer )
    return false;

  QgsAttributeDialog *dialog = newDialog( true );
  dialog->setHighlight( h );
  // delete the dialog when it is closed
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();

  return true;
}

bool QgsFeatureAction::editFeature( bool showModal )
{
  if ( !mLayer )
    return false;

  if ( showModal )
  {
    QScopedPointer<QgsAttributeDialog> dialog( newDialog( false ) );

    if ( !mFeature->isValid() )
      dialog->setMode( QgsAttributeForm::AddFeatureMode );

    int rv = dialog->exec();
    mFeature->setAttributes( dialog->feature()->attributes() );
    return rv;
  }
  else
  {
    QgsAttributeDialog* dialog = newDialog( false );

    if ( !mFeature->isValid() )
      dialog->setMode( QgsAttributeForm::AddFeatureMode );

    // delete the dialog when it is closed
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
  }

  return true;
}

bool QgsFeatureAction::addFeature( const QgsAttributeMap& defaultAttributes, bool showModal )
{
  if ( !mLayer || !mLayer->isEditable() )
    return false;

  QSettings settings;
  bool reuseLastValues = settings.value( QStringLiteral( "/qgis/digitizing/reuseLastValues" ), false ).toBool();
  QgsDebugMsg( QString( "reuseLastValues: %1" ).arg( reuseLastValues ) );

  QgsFields fields = mLayer->fields();
  QgsAttributeMap initialAttributeValues;

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QVariant v;

    if ( defaultAttributes.contains( idx ) )
    {
      initialAttributeValues.insert( idx, defaultAttributes.value( idx ) );
    }
    else if ( reuseLastValues && sLastUsedValues.contains( mLayer ) && sLastUsedValues[ mLayer ].contains( idx ) )
    {
      initialAttributeValues.insert( idx, sLastUsedValues[ mLayer ][idx] );
    }
  }

  // create new feature template - this will initialize the attributes to valid values, handling default
  // values and field constraints
  QgsExpressionContext context = mLayer->createExpressionContext();
  QgsFeature newFeature = QgsVectorLayerUtils::createFeature( mLayer, mFeature->geometry(), initialAttributeValues,
                          &context );
  *mFeature = newFeature;

  //show the dialog to enter attribute values
  //only show if enabled in settings and layer has fields
  bool isDisabledAttributeValuesDlg = ( fields.count() == 0 ) || settings.value( QStringLiteral( "/qgis/digitizing/disable_enter_attribute_values_dialog" ), false ).toBool();

  // override application-wide setting with any layer setting
  switch ( mLayer->editFormConfig().suppress() )
  {
    case QgsEditFormConfig::SuppressOn:
      isDisabledAttributeValuesDlg = true;
      break;
    case QgsEditFormConfig::SuppressOff:
      isDisabledAttributeValuesDlg = false;
      break;
    case QgsEditFormConfig::SuppressDefault:
      break;
  }
  if ( isDisabledAttributeValuesDlg )
  {
    mLayer->beginEditCommand( text() );
    mFeatureSaved = mLayer->addFeature( *mFeature );

    if ( mFeatureSaved )
      mLayer->endEditCommand();
    else
      mLayer->destroyEditCommand();
  }
  else
  {
    QgsAttributeDialog *dialog = newDialog( false );
    // delete the dialog when it is closed
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setMode( QgsAttributeForm::AddFeatureMode );
    dialog->setEditCommandMessage( text() );

    connect( dialog->attributeForm(), SIGNAL( featureSaved( const QgsFeature & ) ), this, SLOT( onFeatureSaved( const QgsFeature & ) ) );

    if ( !showModal )
    {
      setParent( dialog ); // keep dialog until the dialog is closed and destructed
      dialog->show();
      mFeature = nullptr;
      return true;
    }

    dialog->exec();
  }

  // Will be set in the onFeatureSaved SLOT
  return mFeatureSaved;
}

void QgsFeatureAction::onFeatureSaved( const QgsFeature& feature )
{
  QgsAttributeForm* form = qobject_cast<QgsAttributeForm*>( sender() );
  Q_UNUSED( form ) // only used for Q_ASSERT
  Q_ASSERT( form );

  // Assign provider generated values
  if ( mFeature )
    *mFeature = feature;

  mFeatureSaved = true;

  QSettings settings;
  bool reuseLastValues = settings.value( QStringLiteral( "/qgis/digitizing/reuseLastValues" ), false ).toBool();
  QgsDebugMsg( QString( "reuseLastValues: %1" ).arg( reuseLastValues ) );

  if ( reuseLastValues )
  {
    QgsFields fields = mLayer->fields();
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      QgsAttributes newValues = feature.attributes();
      QgsAttributeMap origValues = sLastUsedValues[ mLayer ];
      if ( origValues[idx] != newValues.at( idx ) )
      {
        QgsDebugMsg( QString( "saving %1 for %2" ).arg( sLastUsedValues[ mLayer ][idx].toString() ).arg( idx ) );
        sLastUsedValues[ mLayer ][idx] = newValues.at( idx );
      }
    }
  }
}

QHash<QgsVectorLayer *, QgsAttributeMap> QgsFeatureAction::sLastUsedValues;
