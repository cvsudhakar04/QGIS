/***************************************************************************
                            qgslayoutitemregistry.cpp
                            -------------------------
    begin                : June 2017
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

#include "qgslayoutitemguiregistry.h"
#include "qgslayoutviewrubberband.h"
#include <QPainter>


QgsLayoutViewRubberBand *QgsLayoutItemAbstractGuiMetadata::createRubberBand( QgsLayoutView *view )
{
  return new QgsLayoutViewRectangularRubberBand( view );
}


QgsLayoutItemGuiRegistry::QgsLayoutItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemGuiRegistry::~QgsLayoutItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsLayoutItemGuiRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  // add temporary item to register
  auto createRubberBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  } );

  addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( 101, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), nullptr, createRubberBand ) );
  return true;
}

QgsLayoutItemAbstractGuiMetadata *QgsLayoutItemGuiRegistry::itemMetadata( int type ) const
{
  return mMetadata.value( type );
}

bool QgsLayoutItemGuiRegistry::addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type() );
  return true;
}

QWidget *QgsLayoutItemGuiRegistry::createItemWidget( int type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItemWidget();
}

QgsLayoutViewRubberBand *QgsLayoutItemGuiRegistry::createItemRubberBand( int type, QgsLayoutView *view ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createRubberBand( view );
}

QList<int> QgsLayoutItemGuiRegistry::itemTypes() const
{
  return mMetadata.keys();
}
