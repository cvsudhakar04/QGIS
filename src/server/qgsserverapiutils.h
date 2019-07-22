/***************************************************************************
                          qgsserverapiutils.h

  Class defining utilities for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSERVERAPIUTILS_H
#define QGSSERVERAPIUTILS_H

#include "qgis_server.h"
#include <QString>
#include "qgsproject.h"
#include "qgsserverprojectutils.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsVectorLayer;

#ifndef SIP_RUN
#include "nlohmann/json_fwd.hpp"
using json = nlohmann::json;
#endif

/**
 * \ingroup server
 * The QgsServerApiUtils class contains helper functions to handle common API operations.
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiUtils
{

  public:

    /**
     * Parses a comma separated \a bbox into a (possibily empty) QgsRectangle.
     *
     * \note Z values (i.e. a 6 elements bbox) are silently discarded
     */
    static QgsRectangle parseBbox( const QString &bbox );

    /**
     * layerExtent returns json array with [xMin,yMin,xMax,yMax] CRS84 extent for the given \a layer
     * FIXME: the OpenAPI swagger docs say that it is inverted axis order: West, north, east, south edges of the spatial extent.
     *        but current example implementations and GDAL assume it's not.
     * TODO: maybe consider advertised extent instead?
     */
    static json layerExtent( const QgsVectorLayer *layer ) SIP_SKIP;

    /**
     * Parses the CRS URI \a bboxCrs (example: "http://www.opengis.net/def/crs/OGC/1.3/CRS84") into a QGIS CRS object
     */
    static QgsCoordinateReferenceSystem parseCrs( const QString &bboxCrs );

    /**
     * Returns the list of fields accessible to the service for a given \a layer.
     *
     * This method takes into account the ACL restrictions provided by QGIS Server Access Control plugins.
     * TODO: implement ACL
     */
    static const QgsFields publishedFields( const QgsVectorLayer *layer );

    /**
     * Returns the list of layers accessible to the service for a given \a project.
     *
     * This method takes into account the ACL restrictions provided by QGIS Server Access Control plugins.
     *
     * \note project must not be NULL
     * TODO: implement ACL
     */
    static const QVector<QgsMapLayer *> publishedWfsLayers( const QgsProject *project );

#ifndef SIP_RUN

    /**
     * Returns the list of layers of type T accessible to the WFS service for a given \a project.
     *
     * Example:
     *
     *     QVector<QgsVectorLayer*> vectorLayers = publishedLayers<QgsVectorLayer>();
     *
     * TODO: implement ACL
     * \note not available in Python bindings
     * \see publishedWfsLayers()
     * \since QGIS 3.10
     */
    template <typename T>
    static const QVector<T *> publishedWfsLayers( const QgsProject *project )
    {
      const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
      const QStringList wfstUpdateLayersId = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
      const QStringList wfstInsertLayersId = QgsServerProjectUtils::wfstInsertLayerIds( *project );
      const QStringList wfstDeleteLayersId = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
      QVector<T *> result;
      const auto constLayers { project->layers<T *>() };
      for ( const auto &layer : constLayers )
      {
        if ( wfstUpdateLayersId.contains( layer->id() ) ||
             wfstInsertLayersId.contains( layer->id() ) ||
             wfstDeleteLayersId.contains( layer->id() ) )
        {
          result.push_back( layer );
        }

      }
      return result;
    }

#endif

    /**
     * Sanitize the input \a value by removing URL encoding and checking for malicious content.
     * In case of failure returns an empty string.
     */
    static const QString sanitizedFieldValue( const QString &value );

    /**
     * Returns the list of CRSs (format: http://www.opengis.net/def/crs/OGC/1.3/CRS84) available for this \a project.
     * Information is read from project WMS configuration.
     */
    static const QStringList publishedCrsList( const QgsProject *project );

    /**
     * Returns a \a crs into OGC URI (format: http://www.opengis.net/def/crs/OGC/1.3/CRS84)
     * Returns an empty string on failure.
     */
    static const QString crsToOgcUri( const QgsCoordinateReferenceSystem &crs );


};
#endif // QGSSERVERAPIUTILS_H
