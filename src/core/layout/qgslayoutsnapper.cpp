/***************************************************************************
                             qgslayoutsnapper.cpp
                             --------------------
    begin                : July 2017
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

#include "qgslayoutsnapper.h"
#include "qgslayout.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"

QgsLayoutSnapper::QgsLayoutSnapper( QgsLayout *layout )
  : mLayout( layout )
{
}

QgsLayout *QgsLayoutSnapper::layout()
{
  return mLayout;
}

void QgsLayoutSnapper::setSnapTolerance( const int snapTolerance )
{
  mTolerance = snapTolerance;
}

void QgsLayoutSnapper::setSnapToGrid( bool enabled )
{
  mSnapToGrid = enabled;
}

void QgsLayoutSnapper::setSnapToGuides( bool enabled )
{
  mSnapToGuides = enabled;
}

QPointF QgsLayoutSnapper::snapPoint( QPointF point, double scaleFactor, bool &snapped ) const
{
  snapped = false;

  // highest priority - guides
  bool snappedXToGuides = false;
  double newX = snapPointToGuides( point.x(), QgsLayoutGuide::Vertical, scaleFactor, snappedXToGuides );
  if ( snappedXToGuides )
  {
    snapped = true;
    point.setX( newX );
  }
  bool snappedYToGuides = false;
  double newY = snapPointToGuides( point.y(), QgsLayoutGuide::Horizontal, scaleFactor, snappedYToGuides );
  if ( snappedYToGuides )
  {
    snapped = true;
    point.setY( newY );
  }

  bool snappedXToGrid = false;
  bool snappedYToGrid = false;
  QPointF res = snapPointToGrid( point, scaleFactor, snappedXToGrid, snappedYToGrid );
  if ( snappedXToGrid && !snappedXToGuides )
  {
    snapped = true;
    point.setX( res.x() );
  }
  if ( snappedYToGrid && !snappedYToGuides )
  {
    snapped = true;
    point.setY( res.y() );
  }

  return point;
}

QPointF QgsLayoutSnapper::snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX, bool &snappedY ) const
{
  snappedX = false;
  snappedY = false;
  if ( !mLayout || !mSnapToGrid )
  {
    return point;
  }
  const QgsLayoutGridSettings &grid = mLayout->gridSettings();
  if ( grid.resolution().length() <= 0 )
    return point;

  //calculate y offset to current page
  QPointF pagePoint = mLayout->pageCollection()->positionOnPage( point );

  double yPage = pagePoint.y(); //y-coordinate relative to current page
  double yAtTopOfPage = mLayout->pageCollection()->page( mLayout->pageCollection()->pageNumberForPoint( point ) )->pos().y();

  //snap x coordinate
  double gridRes = mLayout->convertToLayoutUnits( grid.resolution() );
  QPointF gridOffset = mLayout->convertToLayoutUnits( grid.offset() );
  int xRatio = static_cast< int >( ( point.x() - gridOffset.x() ) / gridRes + 0.5 ); //NOLINT
  int yRatio = static_cast< int >( ( yPage - gridOffset.y() ) / gridRes + 0.5 ); //NOLINT

  double xSnapped = xRatio * gridRes + gridOffset.x();
  double ySnapped = yRatio * gridRes + gridOffset.y() + yAtTopOfPage;

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  if ( std::fabs( xSnapped - point.x() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    xSnapped = point.x();
  }
  else
  {
    snappedX = true;
  }
  if ( std::fabs( ySnapped - point.y() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    ySnapped = point.y();
  }
  else
  {
    snappedY = true;
  }

  return QPointF( xSnapped, ySnapped );
}

double QgsLayoutSnapper::snapPointToGuides( double original, QgsLayoutGuide::Orientation orientation, double scaleFactor, bool &snapped ) const
{
  snapped = false;
  if ( !mLayout || !mSnapToGuides )
  {
    return original;
  }

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  double bestPos = original;
  double smallestDiff = DBL_MAX;

  Q_FOREACH ( QgsLayoutGuide *guide, mLayout->guides().guides( orientation ) )
  {
    double guidePos = guide->layoutPosition();
    double diff = std::fabs( original - guidePos );
    if ( diff < smallestDiff )
    {
      smallestDiff = diff;
      bestPos = guidePos;
    }
  }

  if ( smallestDiff <= alignThreshold )
  {
    snapped = true;
    return bestPos;
  }
  else
  {
    return original;
  }
}

bool QgsLayoutSnapper::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Snapper" ) );

  element.setAttribute( QStringLiteral( "tolerance" ), mTolerance );
  element.setAttribute( QStringLiteral( "snapToGrid" ), mSnapToGrid );
  element.setAttribute( QStringLiteral( "snapToGuides" ), mSnapToGuides );

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutSnapper::readXml( const QDomElement &e, const QDomDocument &, const QgsReadWriteContext & )
{
  QDomElement element = e;
  if ( element.nodeName() != QStringLiteral( "Snapper" ) )
  {
    element = element.firstChildElement( QStringLiteral( "Snapper" ) );
  }

  if ( element.nodeName() != QStringLiteral( "Snapper" ) )
  {
    return false;
  }

  mTolerance = element.attribute( QStringLiteral( "tolerance" ), QStringLiteral( "5" ) ).toInt();
  mSnapToGrid = element.attribute( QStringLiteral( "snapToGrid" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  mSnapToGuides = element.attribute( QStringLiteral( "snapToGuides" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  return true;
}


