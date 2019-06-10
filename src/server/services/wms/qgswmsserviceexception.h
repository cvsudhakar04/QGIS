/***************************************************************************
                              qgsserviceexception.h
                              ------------------------
  begin                : June 13, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSSERVICEEXCEPTION_H
#define QGSWMSSERVICEEXCEPTION_H

#include <QString>
#include <QMetaEnum>

#include "qgsserverexception.h"
#include "qgswmsparameters.h"

namespace QgsWms
{

  /**
   * \ingroup server
   * \class  QgsWms::QgsServiceException
   * \brief Exception class for WMS service exceptions.
   * \since QGIS 3.0
   */
  class QgsServiceException : public QgsOgcServiceException
  {
      Q_GADGET

    public:

      /**
       * Exception codes as defined in OGC scpecifications for WMS 1.1.1 and
       * WMS 1.3.0. Some custom QGIS codes are defined too.
       * \since QGIS 3.8
       */
      enum ExceptionCode
      {
        OGC_INVALID_FORMAT,
        OGC_INVALID_SRS,
        OGC_LAYER_NOT_DEFINED,
        OGC_STYLE_NOT_DEFINED,
        OGC_LAYER_NOT_QUERYABLE,
        OGC_CURRENT_UPDATE_SEQUENCE,
        OGC_INVALID_UPDATE_SEQUENCE,
        OGC_MISSING_DIMENSION_VALUE,
        OGC_INVALID_DIMENSION_VALUE,
        OGC_INVALID_POINT, // new in WMS 1.3.0
        OGC_INVALID_CRS, // new in WMS 1.3.0
        OGC_OPERATION_NOT_SUPPORTED, // new in WMS 1.3.0
        QGIS_MISSING_PARAMETER_VALUE,
        QGIS_INVALID_PARAMETER_VALUE
      };
      Q_ENUM( ExceptionCode )

      /**
       * Constructor for QgsServiceException.
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message, const QString &locator = QString(),
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, locator, responseCode, QStringLiteral( "1.3.0" ) )
      {}

      /**
       * Constructor for QgsServiceException (empty locator attribute).
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message, int responseCode )
        : QgsOgcServiceException( code, message, QString(), responseCode, QStringLiteral( "1.3.0" ) )
      {}

      /**
       * Constructor for QgsServiceException (empty locator attribute).
       * \param code Error code
       * \param message Exception message to return to the client
       * \param responseCode HTTP error code
       * \since QGIS 3.8
       */
      QgsServiceException( ExceptionCode code, const QString &message, int responseCode )
        : QgsServiceException( formatCode( code ), message, QString(), responseCode )
      {}

      /**
       * Constructor for QgsServiceException (empty locator attribute).
       * \param code Error code
       * \param parameter The WMS parameter on which an error has been detected
       * \param responseCode HTTP error code
       * \since QGIS 3.8
       */
      QgsServiceException( ExceptionCode code, const QgsWmsParameter &parameter, int responseCode )
        : QgsServiceException( formatCode( code ), formatMessage( code, parameter ), QString(), responseCode )
      {}

    private:
      static QString formatMessage( ExceptionCode code, const QgsWmsParameter &parameter )
      {
        const QString name = parameter.name();
        QString message;

        switch ( code )
        {
          case QgsServiceException::QGIS_MISSING_PARAMETER_VALUE:
          {
            message = QStringLiteral( "The %1 parameter is missing." ).arg( name );
            break;
          }
          case QGIS_INVALID_PARAMETER_VALUE:
          {
            message = QStringLiteral( "The %1 parameter is invalid." ).arg( name );
            break;
          }
          case OGC_INVALID_FORMAT:
          {
            message = QStringLiteral( "The format '%1' from %2 is not supported." ).arg( parameter.toString(), name );
            break;
          }
          case OGC_INVALID_SRS:
          {
            message = QStringLiteral( "The SRS is not valid." );
            break;
          }
          case OGC_INVALID_CRS:
          {
            message = QStringLiteral( "The CRS is not valid." );
            break;
          }
          case OGC_LAYER_NOT_DEFINED:
          {
            message = QStringLiteral( "The layer '%1' does not exist." ).arg( parameter.toString() );
            break;
          }
          case OGC_LAYER_NOT_QUERYABLE:
          {
            message = QStringLiteral( "The layer '%1' is not queryable." ).arg( parameter.toString() );
            break;
          }
          case OGC_INVALID_POINT:
          {
            message = QStringLiteral( "The point '%1' from '%2' is invalid." ).arg( parameter.toString(), name );
            break;
          }
          case OGC_STYLE_NOT_DEFINED:
          case OGC_CURRENT_UPDATE_SEQUENCE:
          case OGC_INVALID_UPDATE_SEQUENCE:
          case OGC_MISSING_DIMENSION_VALUE:
          case OGC_INVALID_DIMENSION_VALUE:
          case OGC_OPERATION_NOT_SUPPORTED:
          {
            break;
          }
        }

        return message;
      }

      static QString formatCode( ExceptionCode code )
      {
        // get key as a string from enum
        const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServiceException::ExceptionCode>() );
        QString key = metaEnum.valueToKey( code );

        // remove prefix
        key.replace( QStringLiteral( "OGC_" ), QString() );
        key.replace( QStringLiteral( "QGIS_" ), QString() );

        // build the exception name
        QString formattedCode;
        for ( auto &part : key.split( '_' ) )
        {
          part = part.toLower().replace( 0, 1, part[0].toUpper() );
          part.replace( QStringLiteral( "Srs" ), QStringLiteral( "SRS" ) );
          part.replace( QStringLiteral( "Crs" ), QStringLiteral( "CRS" ) );
          formattedCode = QString( "%1%2" ).arg( formattedCode ).arg( part );
        }

        return formattedCode;
      }
  };

  /**
   * \ingroup server
   * \class  QgsWms::QgsSecurityException
   * \brief Exception thrown when data access violates access controls
   * \since QGIS 3.0
   */
  class QgsSecurityException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsSecurityException (HTTP error code 403 with
       * Security code name).
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       */
      QgsSecurityException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "Security" ), message, locator, 403 )
      {}
  };

  /**
   * \ingroup server
   * \class  QgsWms::QgsBadRequestException
   * \brief Exception thrown in case of malformed request
   * \since QGIS 3.0
   */
  class QgsBadRequestException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsBadRequestException (HTTP error code 400).
       * \param code Error code
       * \param message Exception message to return to the client
       * \since QGIS 3.8
       */
      QgsBadRequestException( ExceptionCode code, const QString &message )
        : QgsServiceException( code, message, 400 )
      {}

      /**
       * Constructor for QgsBadRequestException (HTTP error code 400).
       * \param code Error code
       * \param parameter The WMS parameter on which an error has been detected
       * \since QGIS 3.8
       */
      QgsBadRequestException( ExceptionCode code, const QgsWmsParameter &parameter )
        : QgsServiceException( code, parameter, 400 )
      {}
  };
} // namespace QgsWms

#endif
