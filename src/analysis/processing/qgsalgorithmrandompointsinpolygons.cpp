/***************************************************************************
                         qgsalgorithmrandompointsinpolygons.cpp
                         ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Håvard Tveite
    email                : havard dot tveite at nmbu dot no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmrandompointsinpolygons.h"
#include "random"

// The algorithm parameter names:
static const QString INPUT = QStringLiteral( "INPUT" );
static const QString POINTS_NUMBER = QStringLiteral( "POINTS_NUMBER" );
static const QString MIN_DISTANCE = QStringLiteral( "MIN_DISTANCE" );
static const QString MAX_TRIES_PER_POINT = QStringLiteral( "MAX_TRIES_PER_POINT" );
static const QString SEED = QStringLiteral( "SEED" );
static const QString INCLUDE_POLYGON_ATTRIBUTES = QStringLiteral( "INCLUDE_POLYGON_ATTRIBUTES" );
static const QString OUTPUT = QStringLiteral( "OUTPUT" );
static const QString OUTPUT_POINTS = QStringLiteral( "OUTPUT_POINTS" );
static const QString POINTS_MISSED = QStringLiteral( "POINTS_MISSED" );
static const QString POLYGONS_WITH_MISSED_POINTS = QStringLiteral( "POLYGONS_WITH_MISSED_POINTS" );
static const QString FEATURES_WITH_EMPTY_OR_NO_GEOMETRY = QStringLiteral( "FEATURES_WITH_EMPTY_OR_NO_GEOMETRY" );

///@cond PRIVATE

QString QgsRandomPointsInPolygonsAlgorithm::name() const
{
  return QStringLiteral( "randompointsinpolygons" );
}

QString QgsRandomPointsInPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Random points in polygons" );
}

QStringList QgsRandomPointsInPolygonsAlgorithm::tags() const
{
  return QObject::tr( "seed,attributes,create" ).split( ',' );
}

QString QgsRandomPointsInPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsRandomPointsInPolygonsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsRandomPointsInPolygonsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( INPUT, QObject::tr( "Input polygon layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );
  std::unique_ptr< QgsProcessingParameterNumber > numberPointsParam = qgis::make_unique< QgsProcessingParameterNumber >( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsProcessingParameterNumber::Integer, 1, false, 1 );
  numberPointsParam->setIsDynamic( true );
  numberPointsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsPropertyDefinition::IntegerPositive ) );
  numberPointsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( numberPointsParam.release() );

  std::unique_ptr< QgsProcessingParameterDistance > minDistParam = qgis::make_unique< QgsProcessingParameterDistance >( MIN_DISTANCE, QObject::tr( "Minimum distance between points" ), 0, INPUT, true, 0 );
  minDistParam->setIsDynamic( true );
  minDistParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MIN_DISTANCE, QObject::tr( "Minimum distance between points" ), QgsPropertyDefinition::DoublePositive ) );
  minDistParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( minDistParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > maxAttemptsParam = qgis::make_unique< QgsProcessingParameterNumber >( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of search attempts (for Min. dist. > 0)" ), QgsProcessingParameterNumber::Integer, 10, true, 1 );
  maxAttemptsParam->setFlags( maxAttemptsParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  maxAttemptsParam->setIsDynamic( true );
  maxAttemptsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of attempts per point (for Min. dist. > 0)" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  maxAttemptsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( maxAttemptsParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > randomSeedParam = qgis::make_unique< QgsProcessingParameterNumber >( SEED, QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, QVariant(), true, 1 );
  randomSeedParam->setFlags( randomSeedParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( randomSeedParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > includePolygonAttrParam = qgis::make_unique< QgsProcessingParameterBoolean >( INCLUDE_POLYGON_ATTRIBUTES, QObject::tr( "Include polygon attributes" ), true );
  includePolygonAttrParam->setFlags( includePolygonAttrParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includePolygonAttrParam.release() );

  addParameter( new
                QgsProcessingParameterFeatureSink( OUTPUT, QObject::tr( "Random points in polygons" ), QgsProcessing::TypeVectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( OUTPUT_POINTS, QObject::tr( "Total number of points generated" ) ) );
  addOutput( new QgsProcessingOutputNumber( POINTS_MISSED, QObject::tr( "Number of missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( POLYGONS_WITH_MISSED_POINTS, QObject::tr( "Number of polygons with missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, QObject::tr( "Number of features with empty or no geometry" ) ) );
}

QString QgsRandomPointsInPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "<p>This algorithm creates a point layer, with points placed randomly "
                      "in the polygons of the <i><b>Input polygon layer</b></i>.</p> "
                      "<ul><li>For each feature in the <i><b>Input polygon layer</b></i>, the algorithm attempts to add "
                      "the specified <i><b>Number of points for each feature</b></i> to the output layer.</li> "
                      "<li>A <i><b>Minimum distance between points</b></i> can be specified.<br> "
                      "A point will not be generated if there is an already generated point "
                      "(on any polygon feature) within this (Euclidean) distance from "
                      "the generated location. "
                      "If the <i><b>Minimum distance between points</b></i> is too large, it may not be possible to generate "
                      "the specified <i><b>Number of points for each feature</b></i>.</li> "
                      "<li>The <i><b>Maximum number of attempts per point</b></i> can be specified.</li> "
                      "<li>The seed for the random generator can be provided (<b><i>Random seed</i></b> "
                      "- integer, greater than 0).</li> "
                      "<li>The user can choose not to <i><b>Include polygon feature attributes</b></i> in "
                      "the attributes of the generated point features.</li> "
                      "</ul> "
                      "The total number of points will be<br> <b>'number of input features'</b> * "
                      "<i><b>Number of points for each feature</b></i><br> if there are no misses. "
                      "<p>Output from the algorithm:</p> "
                      "<ul> "
                      "<li> A point layer containing the random points (<code>OUTPUT</code>).</li> "
                      "<li> The number of generated features (<code>POINTS_GENERATED</code>).</li> "
                      "<li> The number of missed points (<code>POINTS_MISSED</code>).</li> "
                      "<li> The number of features with non-empty geometry and missing points (<code>POLYGONS_WITH_MISSED_POINTS</code>).</li> "
                      "<li> The number of features with an empty or no geometry (<code>LINES_WITH_EMPTY_OR_NO_GEOMETRY</code>).</li> "
                      "</ul>"
                    );
}


QgsRandomPointsInPolygonsAlgorithm *QgsRandomPointsInPolygonsAlgorithm::createInstance() const
{
  return new QgsRandomPointsInPolygonsAlgorithm();
}

bool QgsRandomPointsInPolygonsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mNumPoints = parameterAsInt( parameters, POINTS_NUMBER, context );
  mDynamicNumPoints = QgsProcessingParameters::isDynamic( parameters, POINTS_NUMBER );
  if ( mDynamicNumPoints )
    mNumPointsProperty = parameters.value( POINTS_NUMBER ).value< QgsProperty >();

  mMinDistance = parameterAsDouble( parameters, MIN_DISTANCE, context );
  mDynamicMinDistance = QgsProcessingParameters::isDynamic( parameters, MIN_DISTANCE );
  if ( mDynamicMinDistance )
    mMinDistanceProperty = parameters.value( MIN_DISTANCE ).value< QgsProperty >();

  mMaxAttempts = parameterAsInt( parameters, MAX_TRIES_PER_POINT, context );
  mDynamicMaxAttempts = QgsProcessingParameters::isDynamic( parameters, MAX_TRIES_PER_POINT );
  if ( mDynamicMaxAttempts )
    mMaxAttemptsProperty = parameters.value( MAX_TRIES_PER_POINT ).value< QgsProperty >();

  mUseRandomSeed = parameters.value( SEED ).isValid();
  mRandSeed = parameterAsInt( parameters, SEED, context );
  mIncludePolygonAttr = parameterAsBoolean( parameters, INCLUDE_POLYGON_ATTRIBUTES, context );
  return true;
}

QVariantMap QgsRandomPointsInPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters,
    QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > polygonSource( parameterAsSource( parameters, INPUT, context ) );
  if ( !polygonSource )
    throw QgsProcessingException( invalidSourceError( parameters, INPUT ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "rand_point_id" ), QVariant::LongLong ) );
  if ( mIncludePolygonAttr )
    fields.extend( polygonSource->fields() );

  QString ldest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, OUTPUT,
                                          context, ldest, fields, QgsWkbTypes::Point, polygonSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, OUTPUT ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, polygonSource.get() );

  // Initialize random engine
  std::random_device rd;
  std::mt19937 mt( !mUseRandomSeed ? rd() : mRandSeed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

  // Index for finding close points (mMinDistance > 0)
  QgsSpatialIndex index;

  int totNPoints = 0;
  int missedPoints = 0;
  int missedPolygons = 0;
  int emptyOrNullGeom = 0;

  long featureCount = 0;
  long numberOfFeatures = polygonSource->featureCount();
  long long desiredNumberOfPoints = 0;
  const double featureProgressStep = 100.0 / ( numberOfFeatures > 0 ? numberOfFeatures : 1 );
  double baseFeatureProgress = 0.0;
  QgsFeature polyFeat;
  QgsFeatureIterator fitL = mIncludePolygonAttr || mDynamicNumPoints || mDynamicMinDistance || mDynamicMaxAttempts ? polygonSource->getFeatures()
                            : polygonSource->getFeatures( QgsFeatureRequest().setNoAttributes() );
  while ( fitL.nextFeature( polyFeat ) )
  {
    if ( feedback->isCanceled() )
    {
      feedback->setProgress( 0 );
      break;
    }
    if ( !polyFeat.hasGeometry() )
    {
      // Increment invalid features count
      emptyOrNullGeom++;
      featureCount++;
      baseFeatureProgress += featureProgressStep;
      feedback->setProgress( baseFeatureProgress );
      continue;
    }
    QgsGeometry polyGeom( polyFeat.geometry() );
    if ( polyGeom.isEmpty() )
    {
      // Increment invalid features count
      emptyOrNullGeom++;
      featureCount++;
      baseFeatureProgress += featureProgressStep;
      feedback->setProgress( baseFeatureProgress );
      continue;
    }

    if ( mDynamicNumPoints || mDynamicMinDistance || mDynamicMaxAttempts )
    {
      expressionContext.setFeature( polyFeat );
    }

    // double lineLength = polyGeom.length();
    int pointsAddedForThisFeature = 0;

    int numberPointsForThisFeature = mNumPoints;
    if ( mDynamicNumPoints )
      numberPointsForThisFeature = mNumPointsProperty.valueAsInt( expressionContext, numberPointsForThisFeature );
    desiredNumberOfPoints += numberPointsForThisFeature;

    int maxAttemptsForThisFeature = mMaxAttempts;
    if ( mDynamicMaxAttempts )
      maxAttemptsForThisFeature = mMaxAttemptsProperty.valueAsInt( expressionContext, maxAttemptsForThisFeature );

    double minDistanceForThisFeature = mMinDistance;
    if ( mDynamicMinDistance )
      minDistanceForThisFeature = mMinDistanceProperty.valueAsDouble( expressionContext, minDistanceForThisFeature );

    QgsRectangle bbox = polyGeom.boundingBox();

    const double pointProgressIncrement = featureProgressStep / ( numberPointsForThisFeature * maxAttemptsForThisFeature );

    double pointProgress = 0.0;
    for ( long pointIndex = 0; pointIndex < numberPointsForThisFeature; pointIndex++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      // Try to add a point (mMaxAttempts attempts)
      int distCheckIterations = 0;
      while ( distCheckIterations < maxAttemptsForThisFeature )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }
        // Generate a random point
        //double randPos = lineLength * uniformDist( mt );
        //QgsGeometry rpGeom = QgsGeometry( lGeom.interpolate( randPos ) );
        double posX = bbox.xMinimum() + bbox.width() * uniformDist( mt );
        double posY = bbox.yMinimum() + bbox.height() * uniformDist( mt );
        QgsGeometry rpGeom = QgsGeometry::fromPointXY( QgsPointXY( posX, posY) );
        distCheckIterations++;
        pointProgress += pointProgressIncrement;

        if ( !polyGeom.contains(rpGeom) )
        {
            // Outside the polygon
            continue;
        }
        if ( !rpGeom.isNull() && !rpGeom.isEmpty() )
        {
          if ( minDistanceForThisFeature != 0 && totNPoints > 0 )
          {
            // Have to check minimum distance to existing points
            QList<QgsFeatureId> neighbors = index.nearestNeighbor( rpGeom, 1, minDistanceForThisFeature );
            if ( !neighbors.empty() )
            {
              feedback->setProgress( baseFeatureProgress + pointProgress );
              continue;
            }
          }
          // point OK to add
          QgsFeature f = QgsFeature( totNPoints );
          QgsAttributes pAttrs = QgsAttributes();
          pAttrs.append( totNPoints );
          if ( mIncludePolygonAttr )
          {
            pAttrs.append( polyFeat.attributes() );
          }
          f.setAttributes( pAttrs );
          f.setGeometry( rpGeom );

          if ( minDistanceForThisFeature != 0 )
          {
            index.addFeature( f );
          }
          sink->addFeature( f, QgsFeatureSink::FastInsert );
          totNPoints++;
          pointsAddedForThisFeature++;
          pointProgress += pointProgressIncrement * ( maxAttemptsForThisFeature - distCheckIterations );
          break;
        }
        else
        {
          feedback->setProgress( baseFeatureProgress + pointProgress );
        }
      } // while not maxattempts
      feedback->setProgress( baseFeatureProgress + pointProgress );
    } // for points
    baseFeatureProgress += featureProgressStep;
    if ( pointsAddedForThisFeature < numberPointsForThisFeature )
    {
      missedPolygons++;
    }
    featureCount++;
    feedback->setProgress( baseFeatureProgress );
  } // while features
  missedPoints = desiredNumberOfPoints - totNPoints;
  feedback->pushInfo( QObject::tr( "Total number of points generated: "
                                   " %1\nNumber of missed points: %2\nPolygons with missing points: "
                                   " %3\nFeatures with empty or missing geometries: %4"
                                 ).arg( totNPoints ).arg( missedPoints ).arg( missedPolygons ).arg( emptyOrNullGeom ) );
  QVariantMap outputs;
  outputs.insert( OUTPUT, ldest );
  outputs.insert( OUTPUT_POINTS, totNPoints );
  outputs.insert( POINTS_MISSED, missedPoints );
  outputs.insert( POLYGONS_WITH_MISSED_POINTS, missedPolygons );
  outputs.insert( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, emptyOrNullGeom );

  return outputs;
}

///@endcond
