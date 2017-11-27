/***************************************************************************
                             qgslayermetadatavalidator.h
                             ---------------------------
    begin                : April 2017
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

#ifndef QGSLAYERMETADATAVALIDATOR_H
#define QGSLAYERMETADATAVALIDATOR_H

#include "qgis.h"
#include "qgis_core.h"

class QgsLayerMetadata;

/**
 * \ingroup core
 * \class QgsMetadataValidator
 * \brief Abstract base class for metadata validators.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsMetadataValidator
{

  public:

    /**
     * Contains the parameters describing a metadata validation
     * failure.
     */
    struct ValidationResult
    {

      /**
       * Constructor for ValidationResult.
       */
      ValidationResult( const QString &section, const QString &note, const QVariant &identifier = QVariant() )
        : section( section )
        , identifier( identifier )
        , note( note )
      {}

      //! Metadata section which failed the validation
      QString section;

      /**
       * Optional identifier for the failed metadata item.
       * For instance, in list type metadata elements this
       * will be set to the list index of the failed metadata
       * item.
       */
      QVariant identifier;

      //! The reason behind the validation failure.
      QString note;
    };

    virtual ~QgsMetadataValidator() = default;

    /**
     * Validates a \a metadata object, and returns true if the
     * metadata is considered valid.
     * If validation fails, the \a results list will be filled with a list of
     * items describing why the validation failed and what needs to be rectified
     * to fix the metadata.
     */
    virtual bool validate( const QgsLayerMetadata &metadata, QList< QgsMetadataValidator::ValidationResult > &results SIP_OUT ) const = 0;

};


/**
 * \ingroup core
 * \class QgsNativeMetadataValidator
 * \brief A validator for the native QGIS metadata schema definition.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsNativeMetadataValidator : public QgsMetadataValidator
{

  public:

    /**
     * Constructor for QgsNativeMetadataValidator.
     */
    QgsNativeMetadataValidator() = default;

    bool validate( const QgsLayerMetadata &metadata, QList< QgsMetadataValidator::ValidationResult > &results SIP_OUT ) const override;

};

#endif // QGSLAYERMETADATAVALIDATOR_H
