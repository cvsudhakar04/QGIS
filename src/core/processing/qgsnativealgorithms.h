/***************************************************************************
                         qgsnativealgorithms.h
                         ---------------------
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

#ifndef QGSNATIVEALGORITHMS_H
#define QGSNATIVEALGORITHMS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingprovider.h"

///@cond PRIVATE

class QgsNativeAlgorithms: public QgsProcessingProvider
{
  public:

    QgsNativeAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

  protected:

    void loadAlgorithms() override;

};

/**
 * Native centroid algorithm.
 */
class QgsCentroidAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsCentroidAlgorithm();

    QString name() const override { return QStringLiteral( "centroids" ); }
    QString displayName() const override { return QObject::tr( "Centroids" ); }
    virtual QStringList tags() const override { return QObject::tr( "centroid,center,average,point,middle" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry tools" ); }
    QString shortHelpString() const override;
    QgsCentroidAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native transform algorithm.
 */
class QgsTransformAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsTransformAlgorithm();

    QString name() const override { return QStringLiteral( "reprojectlayer" ); }
    QString displayName() const override { return QObject::tr( "Reproject layer" ); }
    virtual QStringList tags() const override { return QObject::tr( "transform,reproject,crs,srs,warp" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector general tools" ); }
    QString shortHelpString() const override;
    QgsTransformAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native buffer algorithm.
 */
class QgsBufferAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsBufferAlgorithm();

    QString name() const override { return QStringLiteral( "buffer" ); }
    QString displayName() const override { return QObject::tr( "Buffer" ); }
    virtual QStringList tags() const override { return QObject::tr( "buffer,grow" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry tools" ); }
    QString shortHelpString() const override;
    QgsBufferAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native dissolve algorithm.
 */
class QgsDissolveAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsDissolveAlgorithm();

    QString name() const override { return QStringLiteral( "dissolve" ); }
    QString displayName() const override { return QObject::tr( "Dissolve" ); }
    virtual QStringList tags() const override { return QObject::tr( "dissolve,union,combine,collect" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry tools" ); }
    QString shortHelpString() const override;
    QgsDissolveAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extract by attribute algorithm.
 */
class QgsExtractByAttributeAlgorithm : public QgsProcessingAlgorithm
{

  public:

    enum Operation
    {
      Equals,
      NotEquals,
      GreaterThan,
      GreaterThanEqualTo,
      LessThan,
      LessThanEqualTo,
      BeginsWith,
      Contains,
      IsNull,
      IsNotNull,
      DoesNotContain,
    };

    QgsExtractByAttributeAlgorithm();

    QString name() const override { return QStringLiteral( "extractbyattribute" ); }
    QString displayName() const override { return QObject::tr( "Extract by attribute" ); }
    virtual QStringList tags() const override { return QObject::tr( "extract,filter,attribute,value,contains,null,field" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection tools" ); }
    QString shortHelpString() const override;
    QgsExtractByAttributeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extract by expression algorithm.
 */
class QgsExtractByExpressionAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsExtractByExpressionAlgorithm();

    QString name() const override { return QStringLiteral( "extractbyexpression" ); }
    QString displayName() const override { return QObject::tr( "Extract by expression" ); }
    virtual QStringList tags() const override { return QObject::tr( "extract,filter,expression,field" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection tools" ); }
    QString shortHelpString() const override;
    QgsExtractByExpressionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native clip algorithm.
 */
class QgsClipAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsClipAlgorithm();

    QString name() const override { return QStringLiteral( "clip" ); }
    QString displayName() const override { return QObject::tr( "Clip" ); }
    virtual QStringList tags() const override { return QObject::tr( "clip,intersect,intersection,mask" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector overlay tools" ); }
    QString shortHelpString() const override;
    QgsClipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};


/**
 * Native subdivide algorithm.
 */
class QgsSubdivideAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsSubdivideAlgorithm();

    QString name() const override { return QStringLiteral( "subdivide" ); }
    QString displayName() const override { return QObject::tr( "Subdivide" ); }
    virtual QStringList tags() const override { return QObject::tr( "subdivide,segmentize,split,tesselate" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry tools" ); }
    QString shortHelpString() const override;
    QgsSubdivideAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native multipart to singlepart algorithm.
 */
class QgsMultipartToSinglepartAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsMultipartToSinglepartAlgorithm();

    QString name() const override { return QStringLiteral( "multiparttosingleparts" ); }
    QString displayName() const override { return QObject::tr( "Multipart to singleparts" ); }
    virtual QStringList tags() const override { return QObject::tr( "multi,single,multiple,split,dump" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry tools" ); }
    QString shortHelpString() const override;
    QgsMultipartToSinglepartAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSNATIVEALGORITHMS_H


