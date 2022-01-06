/***************************************************************************
  qgsalgorithmextractlabels.cpp - QgsExtractLabelsAlgorithm
 ---------------------
 begin                : 30.12.2021
 copyright            : (C) 2021 by Mathieu Pellerin
 email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmextractlabels.h"
#include "qgsprocessingparameters.h"
#include "qgsmapthemecollection.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsnullpainterdevice.h"
#include "qgslabelsink.h"
#include "qgslayertree.h"
#include "qgsvectorlayer.h"
#include "qgsscalecalculator.h"
#include "qgstextlabelfeature.h"
#include "qgsnullsymbolrenderer.h"

#include "pal/feature.h"
#include "pal/pointset.h"
#include "pal/labelposition.h"

#include <QPainter>

#include <cmath>

///@cond PRIVATE

QString QgsExtractLabelsAlgorithm::name() const
{
  return QStringLiteral( "extractlabels" );
}

QString QgsExtractLabelsAlgorithm::displayName() const
{
  return QObject::tr( "Extract labels" );
}

QStringList QgsExtractLabelsAlgorithm::tags() const
{
  return QObject::tr( "map themes,font,position" ).split( ',' );
}

QgsProcessingAlgorithm::Flags QgsExtractLabelsAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagRequiresProject;
}

QString QgsExtractLabelsAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsExtractLabelsAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

void QgsExtractLabelsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent(
                  QStringLiteral( "EXTENT" ),
                  QObject::tr( "Map extent" ) ) );

  addParameter( new QgsProcessingParameterScale(
                  QStringLiteral( "SCALE" ),
                  QObject::tr( "Map scale" ) ) );

  std::unique_ptr<QgsProcessingParameterMapTheme> mapThemeParameter = std::make_unique<QgsProcessingParameterMapTheme>(
        QStringLiteral( "MAP_THEME" ),
        QObject::tr( "Map theme" ),
        QVariant(), true );
  mapThemeParameter->setHelp( QObject::tr( "This parameter is optional. When left unset, the algorithm will fallback to extracting labels from all currently visible layers in the project." ) );
  addParameter( mapThemeParameter.release() );

  addParameter( new QgsProcessingParameterBoolean(
                  QStringLiteral( "INCLUDE_UNPLACED" ),
                  QObject::tr( "Include unplaced labels" ),
                  QVariant( true ), true ) );

  std::unique_ptr<QgsProcessingParameterNumber> dpiParameter = std::make_unique<QgsProcessingParameterNumber>(
        QStringLiteral( "DPI" ),
        QObject::tr( "Map resolution (in DPI)" ),
        QgsProcessingParameterNumber::Double,
        QVariant( 96.0 ), true );
  dpiParameter->setFlags( dpiParameter->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( dpiParameter.release() );

  addParameter( new QgsProcessingParameterFeatureSink(
                  QStringLiteral( "OUTPUT" ),
                  QObject::tr( "Extracted labels" ),
                  QgsProcessing::TypeVectorPoint ) );
}

QString QgsExtractLabelsAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts map labels to a point layer with relevant details saved as attributes." );
}

QString QgsExtractLabelsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts label information from a rendered map at a given extent and scale.\n\n"
                      "If a map theme is provided, the rendered map will match the visibility and symbology of that theme. If left blank, all visible layers from the project will be used.\n\n"
                      "Extracted label information include: position (served as point geometries), the associated layer name and feature ID, label text, rotation (in degree, clockwise), multiline alignment, and font details." );
}

QgsExtractLabelsAlgorithm *QgsExtractLabelsAlgorithm::createInstance() const
{
  return new QgsExtractLabelsAlgorithm();
}

class ExtractLabelSink : public QgsLabelSink
{
  public:
    ExtractLabelSink( QMap<QString, QString> mapLayerNames, QgsProcessingFeedback *feedback )
      : mMapLayerNames( mapLayerNames )
      , mFeedback( feedback )
    {
    };

    void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) override
    {
      processLabel( layerId, context, label, settings );
    }

    void drawUnplacedLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) override
    {
      processLabel( layerId, context, label, settings );
    }

    void processLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings )
    {
      if ( mFeedback->isCanceled() )
      {
        context.setRenderingStopped( true );
      }

      const QgsFeatureId fid = label->getFeaturePart()->featureId();
      if ( ( settings.placement == QgsPalLayerSettings::Curved ||
             settings.placement == QgsPalLayerSettings::PerimeterCurved )
           && !mCurvedWarningPushed.contains( layerId ) )
      {
        mCurvedWarningPushed << layerId;
        mFeedback->pushWarning( QObject::tr( "Curved placement not supported, skipping labels from layer %1" ).arg( mMapLayerNames.value( layerId ) ) );
        return;
      }

      QgsTextLabelFeature *labelFeature = dynamic_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );
      if ( !labelFeature )
        return;

      QgsPalLayerSettings labelSettings( settings );
      const QMap< QgsPalLayerSettings::Property, QVariant > &dataDefinedValues = labelFeature->dataDefinedValues();

      if ( dataDefinedValues.contains( QgsPalLayerSettings::MultiLineWrapChar ) )
      {
        labelSettings.wrapChar = dataDefinedValues.value( QgsPalLayerSettings::MultiLineWrapChar ).toString();
      }
      if ( dataDefinedValues.contains( QgsPalLayerSettings::AutoWrapLength ) )
      {
        labelSettings.autoWrapLength = dataDefinedValues.value( QgsPalLayerSettings::AutoWrapLength ).toInt();
      }
      const QString labelText = QgsPalLabeling::splitToLines( labelFeature->text( -1 ),
                                labelSettings.wrapChar,
                                labelSettings.autoWrapLength,
                                labelSettings.useMaxLineLengthForAutoWrap ).join( "\n" );

      QString labelAlignment;
      if ( dataDefinedValues.contains( QgsPalLayerSettings::MultiLineAlignment ) )
      {
        labelSettings.multilineAlign = static_cast< QgsPalLayerSettings::MultiLineAlign >( dataDefinedValues.value( QgsPalLayerSettings::MultiLineAlignment ).toInt() );
      }
      switch ( labelSettings.multilineAlign )
      {
        case QgsPalLayerSettings::QgsPalLayerSettings::MultiRight:
          labelAlignment = QStringLiteral( "right" );
          break;

        case QgsPalLayerSettings::QgsPalLayerSettings::MultiCenter:
          labelAlignment = QStringLiteral( "center" );
          break;

        case QgsPalLayerSettings::QgsPalLayerSettings::MultiLeft:
          labelAlignment = QStringLiteral( "left" );
          break;

        case QgsPalLayerSettings::QgsPalLayerSettings::MultiJustify:
          labelAlignment = QStringLiteral( "justify" );
          break;

        case QgsPalLayerSettings::MultiFollowPlacement:
          switch ( label->getQuadrant() )
          {
            case pal::LabelPosition::QuadrantAboveLeft:
            case pal::LabelPosition::QuadrantLeft:
            case pal::LabelPosition::QuadrantBelowLeft:
              labelAlignment = QStringLiteral( "right" );
              break;

            case pal::LabelPosition::QuadrantAbove:
            case pal::LabelPosition::QuadrantOver:
            case pal::LabelPosition::QuadrantBelow:
              labelAlignment = QStringLiteral( "center" );
              break;

            case pal::LabelPosition::QuadrantAboveRight:
            case pal::LabelPosition::QuadrantRight:
            case pal::LabelPosition::QuadrantBelowRight:
              labelAlignment = QStringLiteral( "left" );
              break;
          }
          break;
      }

      const double labelRotation = !qgsDoubleNear( label->getAlpha(), 0.0 )
                                   ? -( label->getAlpha() * 180 / M_PI ) + 360
                                   : 0.0;

      const QFont font = labelFeature->definedFont();
      const QString fontFamily = font.family();
      const QString fontStyle = font.styleName();
      const double fontSize = static_cast<double>( font.pixelSize() ) * 72 / context.painter()->device()->logicalDpiX();
      const bool fontItalic = font.italic();
      const bool fontBold = font.bold();
      const double fontLetterSpacing = font.letterSpacing();
      const double fontWordSpacing = font.wordSpacing();

      if ( dataDefinedValues.contains( QgsPalLayerSettings::MultiLineHeight ) )
      {
        QgsTextFormat format = labelSettings.format();
        format.setLineHeight( dataDefinedValues.value( QgsPalLayerSettings::MultiLineHeight ).toDouble() );
        labelSettings.setFormat( format );
      }

      QgsTextFormat format = labelSettings.format();
      double formatLineHeight = format.lineHeight();

      QgsAttributes attributes;
      attributes << mMapLayerNames.value( layerId ) << fid
                 << labelText << label->getWidth() << label->getHeight() << labelRotation << label->conflictsWithObstacle()
                 << fontFamily << fontSize << fontItalic << fontBold << fontStyle << fontLetterSpacing << fontWordSpacing
                 << labelAlignment << formatLineHeight;

      double x = label->getX();
      double y = label->getY();
      QgsGeometry geometry( new QgsPoint( x, y ) );

      QgsFeature feature;
      feature.setAttributes( attributes );
      feature.setGeometry( geometry );
      features << feature;
    };

    QList<QgsFeature> features;

  private:

    QMap<QString, QString> mMapLayerNames;
    QList<QString> mCurvedWarningPushed;

    QgsProcessingFeedback *mFeedback = nullptr;
};

QVariantMap QgsExtractLabelsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context );
  const double scale = parameterAsDouble( parameters, QStringLiteral( "SCALE" ), context );
  if ( qgsDoubleNear( scale, 0.0 ) )
  {
    throw QgsProcessingException( QObject::tr( "Invalid scale value, a number greater than 0 is required" ) );
  }
  double dpi = parameterAsDouble( parameters, QStringLiteral( "DPI" ), context );
  if ( qgsDoubleNear( dpi, 0.0 ) )
  {
    dpi = 96.0;
  }

  QgsScaleCalculator calculator;
  calculator.setDpi( dpi );
  calculator.setMapUnits( mCrs.mapUnits() );
  const QSize imageSize = calculator.calculateImageSize( extent, scale ).toSize();

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "Layer" ), QVariant::String, QString(), 0, 0 ) );
  fields.append( QgsField( QStringLiteral( "FeatureID" ), QVariant::LongLong, QString(), 20 ) );
  fields.append( QgsField( QStringLiteral( "LabelText" ), QVariant::String, QString(), 0, 0 ) );
  fields.append( QgsField( QStringLiteral( "LabelWidth" ), QVariant::String, QString(), 20, 8 ) );
  fields.append( QgsField( QStringLiteral( "LabelHeight" ), QVariant::String, QString(), 20, 8 ) );
  fields.append( QgsField( QStringLiteral( "LabelRotation" ), QVariant::Double, QString(), 20, 2 ) );
  fields.append( QgsField( QStringLiteral( "LabelUnplaced" ), QVariant::Bool, QString(), 1, 0 ) );
  fields.append( QgsField( QStringLiteral( "Family" ), QVariant::String, QString(), 0, 0 ) );
  fields.append( QgsField( QStringLiteral( "Size" ), QVariant::Double, QString(), 20, 4 ) );
  fields.append( QgsField( QStringLiteral( "Italic" ), QVariant::Bool, QString(), 1, 0 ) );
  fields.append( QgsField( QStringLiteral( "Bold" ), QVariant::Bool, QString(), 1, 0 ) );
  fields.append( QgsField( QStringLiteral( "FontStyle" ), QVariant::String, QString(), 0, 0 ) );
  fields.append( QgsField( QStringLiteral( "FontLetterSpacing" ), QVariant::Double, QString(), 20, 4 ) );
  fields.append( QgsField( QStringLiteral( "FontWordSpacing" ), QVariant::Double, QString(), 20, 4 ) );
  fields.append( QgsField( QStringLiteral( "MultiLineAlignment" ), QVariant::String, QString(), 0, 0 ) );
  fields.append( QgsField( QStringLiteral( "MultiLineHeight" ), QVariant::Double, QString(), 20, 2 ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Point, mCrs, QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsMapSettings mapSettings;
  mapSettings.setDestinationCrs( mCrs );
  mapSettings.setExtent( extent );
  mapSettings.setOutputSize( imageSize );
  mapSettings.setOutputDpi( dpi );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );
  mapSettings.setFlag( Qgis::MapSettingsFlag::SkipSymbolRendering, true );
  mapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, true );
  mapSettings.setLayers( mMapLayers );
  mapSettings.setLayerStyleOverrides( mMapThemeStyleOverrides );
  mapSettings.setLabelingEngineSettings( mLabelSettings );

  QgsNullPaintDevice nullPaintDevice;
  nullPaintDevice.setOutputSize( imageSize );
  nullPaintDevice.setOutputDpi( dpi );
  QPainter painter( &nullPaintDevice );

  QgsMapRendererCustomPainterJob renderJob( mapSettings, &painter );
  ExtractLabelSink labelSink( mMapLayerNames, feedback );
  renderJob.setLabelSink( &labelSink );

  feedback->pushInfo( QObject::tr( "Extracting labels" ) );

  QEventLoop loop;
  QObject::connect( feedback, &QgsFeedback::canceled, &renderJob, &QgsMapRendererCustomPainterJob::cancel );
  QObject::connect( &renderJob, &QgsMapRendererJob::renderingLayersFinished, feedback, [feedback]() { feedback->pushInfo( QObject::tr( "Calculating label placement" ) ); } );
  QObject::connect( &renderJob, &QgsMapRendererJob::layerRenderingStarted, feedback, [this, feedback]( const QString & layerId ) { feedback->pushInfo( QObject::tr( "Collecting labels for %1" ).arg( mMapLayerNames.value( layerId ) ) ); } );
  QObject::connect( &renderJob, &QgsMapRendererJob::finished, &loop, [&loop]() { loop.exit(); } );
  renderJob.start();
  loop.exec();

  qDeleteAll( mMapLayers );
  mMapLayers.clear();

  feedback->pushInfo( QObject::tr( "Writing %n label(s) to output layer", "", labelSink.features.count() ) );
  for ( QgsFeature &feature : labelSink.features )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }
  sink.reset();

  if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( dest, context ) ) )
  {
    vl->setRenderer( new QgsNullSymbolRenderer() );
    vl->renderer()->setReferenceScale( scale );

    QgsPalLayerSettings settings;
    QgsPropertyCollection settingsProperties;

    settings.fieldName = QStringLiteral( "LabelText" );
    settings.obstacleSettings().setIsObstacle( false );
    settings.placement = QgsPalLayerSettings::OverPoint;
    settings.quadOffset = QgsPalLayerSettings::QuadrantAboveRight;
    settings.displayAll = true;

    QgsTextFormat textFormat;
    textFormat.setSize( 9 );
    textFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
    textFormat.setColor( QColor( 0, 0, 0 ) );
    settings.setFormat( textFormat );

    settingsProperties.setProperty( QgsPalLayerSettings::Color, QgsProperty::fromExpression( QStringLiteral( "if(\"LabelUnplaced\",'255,0,0','0,0,0')" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::Family, QgsProperty::fromExpression( QStringLiteral( "\"Family\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::Italic, QgsProperty::fromExpression( QStringLiteral( "\"Italic\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::Bold, QgsProperty::fromExpression( QStringLiteral( "\"Bold\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::Size, QgsProperty::fromExpression( QStringLiteral( "\"Size\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::FontLetterSpacing, QgsProperty::fromExpression( QStringLiteral( "\"FontLetterSpacing\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::FontWordSpacing, QgsProperty::fromExpression( QStringLiteral( "\"FontWordSpacing\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::MultiLineAlignment, QgsProperty::fromExpression( QStringLiteral( "\"MultiLineAlignment\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::MultiLineHeight, QgsProperty::fromExpression( QStringLiteral( "\"MultiLineHeight\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromExpression( QStringLiteral( "\"LabelRotation\"" ) ) );
    settingsProperties.setProperty( QgsPalLayerSettings::Show, QgsProperty::fromExpression( QStringLiteral( "\"LabelUnplaced\"=false" ) ) );
    settings.setDataDefinedProperties( settingsProperties );

    QgsAbstractVectorLayerLabeling *labeling = new QgsVectorLayerSimpleLabeling( settings );
    vl->setLabeling( labeling );
    vl->setLabelsEnabled( true );

    QString errorMessage;
    vl->saveStyleToDatabase( QString(), QString(), true, QString(), errorMessage );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


bool QgsExtractLabelsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  // Retrieve and clone layers
  const QString mapTheme = parameterAsString( parameters, QStringLiteral( "MAP_THEME" ), context );
  if ( !mapTheme.isEmpty() && context.project()->mapThemeCollection()->hasMapTheme( mapTheme ) )
  {
    const QList<QgsMapLayer *> constLayers = context.project()->mapThemeCollection()->mapThemeVisibleLayers( mapTheme );
    for ( const QgsMapLayer *l : constLayers )
    {
      // only copy vector layers as other layer types aren't actors in the labeling process
      if ( l->type() == QgsMapLayerType::VectorLayer )
        mMapLayers.push_back( l->clone() );
    }
    mMapThemeStyleOverrides = context.project()->mapThemeCollection( )->mapThemeStyleOverrides( mapTheme );
  }

  if ( mMapLayers.isEmpty() )
  {
    QList<QgsMapLayer *> layers;
    QgsLayerTree *root = context.project()->layerTreeRoot();
    for ( QgsLayerTreeLayer *nodeLayer : root->findLayers() )
    {
      QgsMapLayer *layer = nodeLayer->layer();
      if ( nodeLayer->isVisible() && root->layerOrder().contains( layer ) )
        layers << layer;
    }

    for ( const QgsMapLayer *l : std::as_const( layers ) )
    {
      if ( l->type() == QgsMapLayerType::VectorLayer )
        mMapLayers.push_back( l->clone() );
    }
  }

  for ( const QgsMapLayer *l : std::as_const( mMapLayers ) )
  {
    mMapLayerNames.insert( l->id(), l->name() );
  }

  mCrs = parameterAsExtentCrs( parameters, QStringLiteral( "EXTENT" ), context );
  if ( !mCrs.isValid() )
    mCrs = context.project()->crs();

  bool includeUnplaced = parameterAsBoolean( parameters, QStringLiteral( "INCLUDE_UNPLACED" ), context );
  mLabelSettings = context.project()->labelingEngineSettings();
  mLabelSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, includeUnplaced );
  mLabelSettings.setFlag( QgsLabelingEngineSettings::CollectUnplacedLabels, includeUnplaced );

  return true;
}


///@endcond
