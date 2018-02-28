/***************************************************************************
  qgsquickplugin.cpp
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

#include <qqml.h>

#include "qgsfeature.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsvectorlayer.h"

#include "qgsquickattributeformmodel.h"
#include "qgsquickattributeformmodelbase.h"
#include "qgsquickcoordinatetransformer.h"
#include "qgsquickfeaturemodel.h"
#include "qgsquickfeaturehighlight.h"
#include "qgsquickidentifykit.h"
#include "qgsquickfeature.h"
#include "qgsquickmapcanvasmap.h"
#include "qgsquickmapsettings.h"
#include "qgsquickmaptransform.h"
#include "qgsquickmessagelogmodel.h"
#include "qgsquickplugin.h"
#include "qgsquickpositionkit.h"
#include "qgsquickscalebarkit.h"
#include "qgsquicksubmodel.h"
#include "qgsquickutils.h"

static QObject *_utilsProvider( QQmlEngine *engine, QJSEngine *scriptEngine )
{
  Q_UNUSED( engine )
  Q_UNUSED( scriptEngine )
  return QgsQuickUtils::instance();  // the object will be owned by QML engine and destroyed by the engine on exit
}

void QgisQuickPlugin::registerTypes( const char *uri )
{
  qDebug( "REGISTERING QQmlExtensionInterface: QgisQuick" );

  qRegisterMetaType< QList<QgsMapLayer *> >( "QList<QgsMapLayer*>" );
  qRegisterMetaType< QgsAttributes > ( "QgsAttributes" );
  qRegisterMetaType< QgsCoordinateReferenceSystem >( "QgsCoordinateReferenceSystem" );
  qRegisterMetaType< QgsCoordinateTransformContext >( "QgsCoordinateTransformContext" );
  qRegisterMetaType< QgsFeature > ( "QgsFeature " );
  qRegisterMetaType< QgsFeatureId > ( "QgsFeatureId" );
  qRegisterMetaType< QgsPoint >( "QgsPoint" );
  qRegisterMetaType< QgsPointXY >( "QgsPointXY" );
  qRegisterMetaType< QgsQuickFeature >( "QgsQuickFeature" );

  qmlRegisterType< QgsProject >( uri, 0, 1, "Project" );
  qmlRegisterType< QgsQuickAttributeFormModel >( uri, 0, 1, "AttributeFormModel" );
  qmlRegisterType< QgsQuickCoordinateTransformer >( uri, 0, 1, "CoordinateTransformer" );
  qmlRegisterType< QgsQuickFeatureModel >( uri, 0, 1, "FeatureModel" );
  qmlRegisterType< QgsQuickFeatureHighlight >( uri, 0, 1, "FeatureHighlight" );
  qmlRegisterType< QgsQuickIdentifyKit >( uri, 0, 1, "IdentifyKit" );
  qmlRegisterType< QgsQuickMapCanvasMap >( uri, 0, 1, "MapCanvasMap" );
  qmlRegisterType< QgsQuickMapSettings >( uri, 0, 1, "MapSettings" );
  qmlRegisterType< QgsQuickMapTransform >( uri, 0, 1, "MapTransform" );
  qmlRegisterType< QgsQuickMessageLogModel >( uri, 0, 1, "MessageLogModel" );
  qmlRegisterType< QgsQuickPositionKit >( uri, 0, 1, "PositionKit" );
  qmlRegisterType< QgsQuickScaleBarKit >( uri, 0, 1, "ScaleBarKit" );
  qmlRegisterType< QgsQuickSubModel >( uri, 0, 1, "SubModel" );
  qmlRegisterType< QgsRelationManager >( uri, 0, 1, "RelationManager" );
  qmlRegisterType< QgsVectorLayer >( uri, 0, 1, "VectorLayer" );

  qmlRegisterSingletonType< QgsQuickUtils >( uri, 0, 1, "Utils", _utilsProvider );

  qmlRegisterUncreatableType< QgsMessageLog >( uri, 0, 1, "QgsMessageLog", "Expose MessageLevel" );

  qDebug( "REGISTERING FINISHED" );
}

