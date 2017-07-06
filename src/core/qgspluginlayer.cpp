/***************************************************************************
    qgspluginlayer.cpp
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspluginlayer.h"

#include "qgsmaplayerlegend.h"
#include "qgsmaplayerrenderer.h"

QgsPluginLayer::QgsPluginLayer( const QString &layerType, const QString &layerName )
  : QgsMapLayer( PluginLayer, layerName )
  , mPluginLayerType( layerType )
{
}

QgsPluginLayer::~QgsPluginLayer()
{
  // TODO: shall we move the responsibility of emitting the signal to plugin
  // layer implementations before they start doing their part of cleanup...?
  emit willBeDeleted();
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}

void QgsPluginLayer::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
}

void QgsPluginLayer::setSource( const QString &source )
{
  mDataSource = source;
}
