/***************************************************************************
                              qgswcs.cpp
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgswcsutils.h"
#include "qgswcsgetcapabilities.h"
#include "qgswcsdescribecoverage.h"
#include "qgswcsgetcoverage.h"

#define QSTR_COMPARE( str, lit )\
  (str.compare( QStringLiteral( lit ), Qt::CaseInsensitive ) == 0)

namespace QgsWcs
{

  class Service: public QgsService
  {
    public:
      // Constructor
      Service( QgsServerInterface *serverIface )
        : mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WCS" ); }
      QString version() const override { return implementationVersion(); }

      bool allowMethod( QgsServerRequest::Method method ) const override
      {
        return method == QgsServerRequest::GetMethod || method == QgsServerRequest::PostMethod;
      }

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response,
                           const QgsProject *project ) override
      {
        Q_UNUSED( project );

        QgsServerRequest::Parameters params = request.parameters();
        QString versionString = params.value( "VERSION" );

        // Set the default version
        if ( versionString.isEmpty() )
        {
          versionString = version();
        }

        // Get the request
        QString req = params.value( QStringLiteral( "REQUEST" ) );
        if ( req.isEmpty() )
        {
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Please check the value of the REQUEST parameter" ) );
        }

        if ( QSTR_COMPARE( req, "GetCapabilities" ) )
        {
          writeGetCapabilities( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "DescribeCoverage" ) )
        {
          writeDescribeCoverage( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetCoverage" ) )
        {
          writeGetCoverage( mServerIface, project, versionString, request, response );
        }
        else
        {
          // Operation not supported
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Request %1 is not supported" ).arg( req ) );
        }
      }

    private:
      QgsServerInterface *mServerIface = nullptr;
  };


} // namespace QgsWfs


// Module
class QgsWcsModule: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsDebugMsg( "WCSModule::registerSelf called" );
      registry.registerService( new  QgsWcs::Service( serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWcsModule module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}





