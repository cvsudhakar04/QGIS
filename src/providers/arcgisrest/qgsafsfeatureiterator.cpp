/***************************************************************************
      qgsafsfeatureiterator.cpp
      -------------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsafsfeatureiterator.h"
#include "qgsspatialindex.h"
#include "qgsafsprovider.h"
#include "qgsmessagelog.h"
#include "geometry/qgsgeometry.h"


QgsAfsFeatureSource::QgsAfsFeatureSource( const QgsAfsProvider *provider )
// FIXME: ugly const_cast...
  : mProvider( const_cast<QgsAfsProvider *>( provider ) )
  , mCrs( provider->crs() )
{
}

QgsFeatureIterator QgsAfsFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsAfsFeatureIterator( this, false, request ) );
}

QgsAfsProvider *QgsAfsFeatureSource::provider() const
{
  return mProvider;
}

///////////////////////////////////////////////////////////////////////////////

QgsAfsFeatureIterator::QgsAfsFeatureIterator( QgsAfsFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsAfsFeatureSource>( source, ownSource, request )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs() );
  }
  mFilterRect = transformedFilterRect( mTransform );
}

QgsAfsFeatureIterator::~QgsAfsFeatureIterator()
{
  close();
}

bool QgsAfsFeatureIterator::fetchFeature( QgsFeature &f )
{
  if ( mClosed )
    return false;

  if ( mFeatureIterator >= mSource->provider()->featureCount() )
    return false;

  bool fetchGeometries = ( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0;
  QgsAttributeList fetchAttribures;
  if ( ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) != 0 )
    fetchAttribures = mRequest.subsetOfAttributes();
  else
  {
    for ( int i = 0; i < mSource->provider()->fields().size(); ++i )
      fetchAttribures.append( i );
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    bool result = mSource->provider()->getFeature( mRequest.filterFid(), f, fetchGeometries, fetchAttribures );
    transformFeatureGeometry( f, mTransform );
    return result;
  }
  else
  {
    QgsRectangle filterRect = mSource->provider()->extent();
    if ( !mRequest.filterRect().isNull() )
      filterRect = filterRect.intersect( &mFilterRect );
    while ( mFeatureIterator < mSource->provider()->featureCount() )
    {
      bool success = mSource->provider()->getFeature( mFeatureIterator, f, fetchGeometries, fetchAttribures, filterRect );
      ++mFeatureIterator;
      if ( !success )
        continue;
      transformFeatureGeometry( f, mTransform );
      return true;
    }
  }
  return false;
}

bool QgsAfsFeatureIterator::rewind()
{
  if ( mClosed )
    return false;
  mFeatureIterator = 0;
  return true;
}

bool QgsAfsFeatureIterator::close()
{
  if ( mClosed )
    return false;
  iteratorClosed();
  mClosed = true;
  return true;
}
