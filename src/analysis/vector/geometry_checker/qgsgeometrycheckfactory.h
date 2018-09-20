/***************************************************************************
    qgsgeometrycheckfactory.h
     --------------------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKFACTORY_H
#define QGSGEOMETRYCHECKFACTORY_H

#include <QString>
#include <QMap>
#include <QVariantMap>

#include "qgsgeometrycheck.h"
#include "qgis_sip.h"
#include "qgis_analysis.h"

class QgsGeometryCheck;
class QgsSingleGeometryCheck;

struct QgsGeometryCheckContext;

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryCheckFactory SIP_ABSTRACT
{
  public:

    /**
     * Destructor
     *
     * Deletes all the registered checks
     */
    virtual ~QgsGeometryCheckFactory() = default;

    virtual QgsGeometryCheck *createGeometryCheck( QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration ) const = 0 SIP_FACTORY;

    virtual QgsSingleGeometryCheck *createSingleGeometryCheck( const QString &checkId ) const = 0 SIP_FACTORY;

    virtual QString id() const = 0;

    virtual bool isCompatible( QgsVectorLayer *layer ) const;

    virtual QgsGeometryCheck::Flags flags() const;
};

template<class T>
class QgsGeometryCheckFactoryT : public QgsGeometryCheckFactory
{
  public:
    QgsGeometryCheck *createGeometryCheck( QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration ) const override
    {
      return new T( context, geometryCheckConfiguration );
    }
};


#endif // QGSGEOMETRYCHECKFACTORY_H
