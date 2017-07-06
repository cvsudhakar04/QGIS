/***************************************************************************
                            qgspagesizeregistry.cpp
                            ------------------------
    begin                : June 2017
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

#include "qgspagesizeregistry.h"

//
// QgsPageSizeRegistry
//

QgsPageSizeRegistry::QgsPageSizeRegistry()
{
  add( QgsPageSize( QStringLiteral( "A6" ), QgsLayoutSize( 105, 148 ) ) );
  add( QgsPageSize( QStringLiteral( "A5" ), QgsLayoutSize( 148, 210 ) ) );
  add( QgsPageSize( QStringLiteral( "A4" ), QgsLayoutSize( 210, 297 ) ) );
  add( QgsPageSize( QStringLiteral( "A3" ), QgsLayoutSize( 297, 420 ) ) );
  add( QgsPageSize( QStringLiteral( "A2" ), QgsLayoutSize( 420, 594 ) ) );
  add( QgsPageSize( QStringLiteral( "A1" ), QgsLayoutSize( 594, 841 ) ) );
  add( QgsPageSize( QStringLiteral( "A0" ), QgsLayoutSize( 841, 1189 ) ) );
  add( QgsPageSize( QStringLiteral( "B6" ), QgsLayoutSize( 125, 176 ) ) );
  add( QgsPageSize( QStringLiteral( "B5" ), QgsLayoutSize( 176, 250 ) ) );
  add( QgsPageSize( QStringLiteral( "B4" ), QgsLayoutSize( 250, 353 ) ) );
  add( QgsPageSize( QStringLiteral( "B3" ), QgsLayoutSize( 353, 500 ) ) );
  add( QgsPageSize( QStringLiteral( "B2" ), QgsLayoutSize( 500, 707 ) ) );
  add( QgsPageSize( QStringLiteral( "B1" ), QgsLayoutSize( 707, 1000 ) ) );
  add( QgsPageSize( QStringLiteral( "B0" ), QgsLayoutSize( 1000, 1414 ) ) );
  add( QgsPageSize( QStringLiteral( "Legal" ), QgsLayoutSize( 215.9, 355.6 ) ) );
  add( QgsPageSize( QStringLiteral( "Letter" ), QgsLayoutSize( 215.9, 279.4 ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI A" ), QgsLayoutSize( 215.9, 279.4 ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI B" ), QgsLayoutSize( 279.4, 431.8 ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI C" ), QgsLayoutSize( 431.8, 558.8 ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI D" ), QgsLayoutSize( 558.8, 863.6 ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI E" ), QgsLayoutSize( 863.6, 1117.6 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch A" ), QgsLayoutSize( 228.6, 304.8 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch B" ), QgsLayoutSize( 304.8, 457.2 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch C" ), QgsLayoutSize( 457.2, 609.6 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch D" ), QgsLayoutSize( 609.6, 914.4 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E" ), QgsLayoutSize( 914.4, 1219.2 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E1" ), QgsLayoutSize( 762, 1066.8 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E2" ), QgsLayoutSize( 660, 965 ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E3" ), QgsLayoutSize( 686, 991 ) ) );
}

void QgsPageSizeRegistry::add( const QgsPageSize &size )
{
  mPageSizes.append( size );
}

QList<QgsPageSize> QgsPageSizeRegistry::entries() const
{
  QList< QgsPageSize > result;
  QList< QgsPageSize >::const_iterator it = mPageSizes.constBegin();
  for ( ; it != mPageSizes.constEnd(); ++it )
  {
    result.push_back( *it );
  }
  return result;
}

QList<QgsPageSize> QgsPageSizeRegistry::find( const QString &name ) const
{
  QList< QgsPageSize > result;
  QList< QgsPageSize >::const_iterator it = mPageSizes.constBegin();
  for ( ; it != mPageSizes.constEnd(); ++it )
  {
    if ( ( *it ).name.compare( name, Qt::CaseInsensitive ) == 0 )
    {
      result.push_back( *it );
    }
  }
  return result;
}

bool QgsPageSizeRegistry::decodePageSize( const QString &pageSizeName, QgsPageSize &pageSize )
{
  QList< QgsPageSize > matches = find( pageSizeName.trimmed() );
  if ( matches.length() > 0 )
  {
    pageSize = matches.at( 0 );
    return true;
  }
  return false;
}

//
// QgsPageSize
//

QgsPageSize::QgsPageSize()
  : size( QgsLayoutSize( 0.0, 0.0 ) )
{
}

QgsPageSize::QgsPageSize( const QString &pageName, const QgsLayoutSize &pageSize )
  : name( pageName )
  , size( pageSize )
{
}

QgsPageSize::QgsPageSize( const QgsLayoutSize &pageSize )
  :  size( pageSize )
{

}

bool QgsPageSize::operator==( const QgsPageSize &other ) const
{
  return ( name == other.name && size == other.size );
}

bool QgsPageSize::operator!=( const QgsPageSize &other ) const
{
  return ( ! operator==( other ) );
}
