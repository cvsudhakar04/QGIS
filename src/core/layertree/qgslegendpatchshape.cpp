/***************************************************************************
                         qgslegendpatchshape.cpp
                         -------------------
begin                : April 2020
copyright            : (C) 2020 by Nyall Dawson
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

#include "qgslegendpatchshape.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

QgsLegendPatchShape::QgsLegendPatchShape( QgsSymbol::SymbolType type, const QgsGeometry &geometry, bool preserveAspectRatio )
  : mSymbolType( type )
  , mGeometry( geometry )
  , mPreserveAspectRatio( preserveAspectRatio )
{

}

bool QgsLegendPatchShape::isNull() const
{
  return mGeometry.isNull() || mGeometry.isEmpty();
}

QgsGeometry QgsLegendPatchShape::geometry() const
{
  return mGeometry;
}

void QgsLegendPatchShape::setGeometry( const QgsGeometry &geometry )
{
  mGeometry = geometry;
}

bool QgsLegendPatchShape::preserveAspectRatio() const
{
  return mPreserveAspectRatio;
}

void QgsLegendPatchShape::setPreserveAspectRatio( bool preserveAspectRatio )
{
  mPreserveAspectRatio = preserveAspectRatio;
}

QPolygonF lineStringToQPolygonF( const QgsLineString *line )
{
  const double *srcX = line->xData();
  const double *srcY = line->yData();
  const int count = line->numPoints();
  QPolygonF thisRes( count );
  QPointF *dest = thisRes.data();
  for ( int i = 0; i < count; ++i )
  {
    *dest++ = QPointF( *srcX++, *srcY++ );
  }
  return thisRes;
}

QPolygonF curveToPolygonF( const QgsCurve *curve )
{
  if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( curve ) )
  {
    return lineStringToQPolygonF( line );
  }
  else
  {
    std::unique_ptr< QgsLineString > straightened( curve->curveToLine() );
    return lineStringToQPolygonF( straightened.get() );
  }
}

QList<QList<QPolygonF> > QgsLegendPatchShape::toQPolygonF( QgsSymbol::SymbolType type, QSizeF size ) const
{
  if ( isNull() || type != mSymbolType )
    return defaultPatch( type, size );

  // scale and translate to desired size

  const QRectF bounds = mGeometry.boundingBox().toRectF();

  double dx = 0;
  double dy = 0;
  if ( mPreserveAspectRatio && bounds.height() > 0 && bounds.width() > 0 )
  {
    const double scaling = std::min( size.width() / bounds.width(), size.height() / bounds.height() );
    const QSizeF scaledSize = bounds.size() * scaling;
    dx = ( size.width() - scaledSize.width() ) / 2.0;
    dy = ( size.height() - scaledSize.height() ) / 2.0;
    size = scaledSize;
  }

  // important -- the transform needs to flip from north-up to painter style "increasing y down" coordinates
  QPolygonF targetRectPoly = QPolygonF() << QPointF( dx, dy + size.height() )
                             << QPointF( dx + size.width(), dy + size.height() )
                             << QPointF( dx + size.width(), dy )
                             << QPointF( dx, dy );
  QPolygonF patchRectPoly = QPolygonF( bounds );
  //workaround QT Bug #21329
  patchRectPoly.pop_back();
  QTransform t;
  QTransform::quadToQuad( patchRectPoly, targetRectPoly, t );

  QgsGeometry geom = mGeometry;
  geom.transform( t );

  switch ( mSymbolType )
  {
    case QgsSymbol::Marker:
    {
      QPolygonF points;

      if ( QgsWkbTypes::flatType( mGeometry.wkbType() ) == QgsWkbTypes::MultiPoint )
      {
        const QgsGeometry patch = geom;
        for ( auto it = patch.vertices_begin(); it != patch.vertices_end(); ++it )
          points << QPointF( ( *it ).x(), ( *it ).y() );
      }
      else
      {
        points << QPointF( size.width() / 2, size.height() / 2 );
      }
      return QList< QList<QPolygonF> >() << ( QList< QPolygonF >() << points );
    }

    case QgsSymbol::Line:
    {
      QList< QList<QPolygonF> > res;
      const QgsGeometry patch = geom;
      for ( auto it = patch.const_parts_begin(); it != patch.const_parts_end(); ++it )
      {
        res << ( QList< QPolygonF >() << curveToPolygonF( qgsgeometry_cast< const QgsCurve * >( *it ) ) );
      }
      return res;
    }

    case QgsSymbol::Fill:
    {
      QList< QList<QPolygonF> > res;

      const QgsGeometry patch = geom;
      for ( auto it = patch.const_parts_begin(); it != patch.const_parts_end(); ++it )
      {
        QList<QPolygonF> thisPart;
        const QgsCurvePolygon *surface = qgsgeometry_cast< const QgsCurvePolygon * >( *it );
        if ( !surface )
          continue;

        if ( !surface->exteriorRing() )
          continue;

        thisPart << curveToPolygonF( surface->exteriorRing() );

        for ( int i = 0; i < surface->numInteriorRings(); ++i )
          thisPart << curveToPolygonF( surface->interiorRing( i ) );
        res << thisPart;
      }

      return res;
    }

    case QgsSymbol::Hybrid:
      return QList< QList<QPolygonF> >();
  }

  return QList< QList<QPolygonF> >();
}

QList<QList<QPolygonF> > QgsLegendPatchShape::defaultPatch( QgsSymbol::SymbolType type, QSizeF size )
{
  switch ( type )
  {
    case QgsSymbol::Marker:
      return QList< QList< QPolygonF > >() << ( QList< QPolygonF >() << ( QPolygonF() << QPointF( static_cast< int >( size.width() ) / 2,
             static_cast< int >( size.height() ) / 2 ) ) );

    case QgsSymbol::Line:
      // we're adding 0.5 to get rid of blurred preview:
      // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
      return QList< QList<QPolygonF> >() << ( QList< QPolygonF >() << ( QPolygonF()  << QPointF( 0, static_cast< int >( size.height() ) / 2 + 0.5 ) << QPointF( size.width(), static_cast< int >( size.height() ) / 2 + 0.5 ) ) );

    case QgsSymbol::Fill:
      return QList< QList<QPolygonF> >() << ( QList< QPolygonF> () << ( QRectF( QPointF( 0, 0 ), QPointF( static_cast< int >( size.width() ), static_cast< int >( size.height() ) ) ) ) );

    case QgsSymbol::Hybrid:
      return QList< QList<QPolygonF> >();
  }

  return QList< QList<QPolygonF> >();
}

void QgsLegendPatchShape::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mGeometry = QgsGeometry::fromWkt( element.attribute( QStringLiteral( "wkt" ) ) );
  mPreserveAspectRatio = element.attribute( QStringLiteral( "preserveAspect" ) ).toInt();
  mSymbolType = static_cast< QgsSymbol::SymbolType >( element.attribute( QStringLiteral( "type" ) ).toInt() );
}

void QgsLegendPatchShape::writeXml( QDomElement &element, QDomDocument &, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mGeometry.isNull() ? QString() : mGeometry.asWkt( ) );
  element.setAttribute( QStringLiteral( "preserveAspect" ), mPreserveAspectRatio ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "type" ), QString::number( mSymbolType ) );
}

QgsSymbol::SymbolType QgsLegendPatchShape::symbolType() const
{
  return mSymbolType;
}

void QgsLegendPatchShape::setSymbolType( QgsSymbol::SymbolType type )
{
  mSymbolType = type;
}
