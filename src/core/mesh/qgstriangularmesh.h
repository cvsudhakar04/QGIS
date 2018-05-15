/***************************************************************************
                         qgstriangularmesh.h
                         -------------------
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

#ifndef QGSTRIANGULARMESH_H
#define QGSTRIANGULARMESH_H


#define SIP_NO_FILE

#include <QVector>

#include "qgis_core.h"
#include "qgsmeshdataprovider.h"

class QgsRenderContext;

//! Mesh - vertices and faces
struct CORE_EXPORT QgsMesh
{
  //! vertices
  QVector<QgsMeshVertex> vertices;
  //! faces
  QVector<QgsMeshFace> faces;
};

/**
 * \ingroup core
 *
 * Triangular/Derived Mesh
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsTriangularMesh
{
  public:
    //! Ctor
    QgsTriangularMesh() = default;
    //! Dtor
    ~QgsTriangularMesh() = default;

    /**
     * Construct triangular mesh from layer's native mesh and context
     * \param nativeMesh QgsMesh to access native vertices and faces
     * \param context Rendering context to estimate number of triagles to create for an face
    */
    void update( QgsMesh *nativeMesh, QgsRenderContext *context );

    /**
     * Return vertices in map CRS
     *
     * The list of consist of vertices from native mesh (0-N) and
     * extra vertices needed to create triangles (N+1 - len)
     */
    const QVector<QgsMeshVertex> &vertices() const ;
    //! Return triangles
    const QVector<QgsMeshFace> &triangles() const ;
    //! Return mapping between triangles and original faces
    const QVector<int> &trianglesToNativeFaces() const ;

  private:
    // vertices: map CRS; 0-N ... native vertices, N+1 - len ... extra vertices
    // faces are derived triangles
    QgsMesh mTriangularMesh;
    QVector<int> mTrianglesToNativeFaces; //len(mTrianglesToNativeFaces) == len(mTriangles). Mapping derived -> native
};


#endif // QGSTRIANGULARMESH_H
