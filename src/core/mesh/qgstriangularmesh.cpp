/***************************************************************************
                         qgstriangularmesh.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstriangularmesh.h"
#include "qgsrendercontext.h"
#include "qgscoordinatetransform.h"

static void ENP_centroid_step( const QPolygonF &pX, double &cx, double &cy, double &signedArea, int i, int i1 )
{
  double x0 = 0.0; // Current vertex X
  double y0 = 0.0; // Current vertex Y
  double x1 = 0.0; // Next vertex X
  double y1 = 0.0; // Next vertex Y
  double a = 0.0;  // Partial signed area

  x0 = pX[i].x();
  y0 = pX[i].y();
  x1 = pX[i1].x();
  y1 = pX[i1].y();
  a = x0 * y1 - x1 * y0;
  signedArea += a;
  cx += ( x0 + x1 ) * a;
  cy += ( y0 + y1 ) * a;
}

static void ENP_centroid( const QPolygonF &pX, double &cx, double &cy )
{
  // http://stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon/2792459#2792459
  cx = 0;
  cy = 0;

  if ( pX.isEmpty() )
    return;

  double signedArea = 0.0;

  // For all vertices except last
  int i = 0;
  for ( ; i < pX.size() - 1; ++i )
  {
    ENP_centroid_step( pX, cx, cy, signedArea, i, i + 1 );
  }
  // Do last vertex separately to avoid performing an expensive
  // modulus operation in each iteration.
  ENP_centroid_step( pX, cx, cy, signedArea, i, 0 );

  signedArea *= 0.5;
  cx /= ( 6.0 * signedArea );
  cy /= ( 6.0 * signedArea );
}


void QgsTriangularMesh::update( QgsMesh *nativeMesh, QgsRenderContext *context )
{
  Q_ASSERT( nativeMesh );
  Q_ASSERT( context );

  mTriangularMesh.vertices.clear();
  mTriangularMesh.faces.clear();
  mTrianglesToNativeFaces.clear();
  mNativeMeshFaceCentroids.clear();

  // TRANSFORM VERTICES
  QgsCoordinateTransform transform = context->coordinateTransform();
  mTriangularMesh.vertices.resize( nativeMesh->vertices.size() );
  for ( int i = 0; i < nativeMesh->vertices.size(); ++i )
  {
    const QgsMeshVertex &vertex = nativeMesh->vertices.at( i );
    if ( transform.isValid() )
    {
      QgsPointXY mapPoint = transform.transform( QgsPointXY( vertex.x(), vertex.y() ) );
      QgsMeshVertex mapVertex( mapPoint );
      mapVertex.setZ( vertex.z() );
      mapVertex.setM( vertex.m() );
      mTriangularMesh.vertices[i] = mapVertex;
    }
    else
    {
      mTriangularMesh.vertices[i] = vertex;
    }
  }

  // CREATE TRIANGULAR MESH
  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace &face = nativeMesh->faces.at( i ) ;
    if ( face.size() == 3 )
    {
      // triangle
      mTriangularMesh.faces.push_back( face );
      mTrianglesToNativeFaces.push_back( i );
    }
    else if ( face.size() == 4 )
    {
      // quad
      QgsMeshFace face1;
      face1.push_back( face[0] );
      face1.push_back( face[1] );
      face1.push_back( face[2] );

      mTriangularMesh.faces.push_back( face1 );
      mTrianglesToNativeFaces.push_back( i );

      QgsMeshFace face2;
      face2.push_back( face[0] );
      face2.push_back( face[2] );
      face2.push_back( face[3] );

      mTriangularMesh.faces.push_back( face2 );
      mTrianglesToNativeFaces.push_back( i );
    }
  }

  // CALCULATE CENTROIDS
  mNativeMeshFaceCentroids.resize( nativeMesh->faces.size() );
  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace &face = nativeMesh->faces.at( i ) ;
    QVector<QPointF> points;
    points.reserve( face.size() );
    for ( int j = 0; j < face.size(); ++j )
    {
      int index = face.at( j );
      const QgsMeshVertex &vertex = mTriangularMesh.vertices[index]; // we need projected vertices
      points.push_back( vertex.toQPointF() );
    }
    QPolygonF poly( points );
    double cx, cy;
    ENP_centroid( poly, cx, cy );
    mNativeMeshFaceCentroids[i] = QgsMeshVertex( cx, cy );
  }
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::vertices() const
{
  return mTriangularMesh.vertices;
}

const QVector<QgsMeshFace> &QgsTriangularMesh::triangles() const
{
  return mTriangularMesh.faces;
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::centroids() const
{
  return mNativeMeshFaceCentroids;
}

const QVector<int> &QgsTriangularMesh::trianglesToNativeFaces() const
{
  return mTrianglesToNativeFaces;
}

