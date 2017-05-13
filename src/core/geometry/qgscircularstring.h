/***************************************************************************
                         qgscircularstring.h
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include <QVector>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscurve.h"


/** \ingroup core
 * \class QgsCircularString
 * \brief Circular string geometry type
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsCircularString: public QgsCurve
{
  public:
    QgsCircularString();

    virtual bool operator==( const QgsCurve &other ) const override;
    virtual bool operator!=( const QgsCurve &other ) const override;

    virtual QString geometryType() const override { return QStringLiteral( "CircularString" ); }
    virtual int dimension() const override { return 1; }
    virtual QgsCircularString *clone() const override SIP_FACTORY;
    virtual void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr &wkb ) override;
    virtual bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    bool isEmpty() const override;
    int numPoints() const override;

    /** Returns the point at index i within the circular string.
     */
    QgsPointV2 pointN( int i ) const;

    void points( QgsPointSequence &pts SIP_OUT ) const override;

    /** Sets the circular string's points
     */
    void setPoints( const QgsPointSequence &points );

    virtual double length() const override;
    virtual QgsPointV2 startPoint() const override;
    virtual QgsPointV2 endPoint() const override;

    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform &t ) override;
    void addToPainterPath( QPainterPath &path ) const override;

    void drawAsPolygon( QPainter &p ) const override;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2 &vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2 &newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual double closestSegment( const QgsPointV2 &pt, QgsPointV2 &segmentPt SIP_OUT,
                                   QgsVertexId &vertexAfter SIP_OUT,
                                   bool *leftOf SIP_OUT, double epsilon ) const override;

    bool pointAt( int node, QgsPointV2 &point, QgsVertexId::VertexType &type ) const override;
    void sumUpArea( double &sum SIP_OUT ) const override;
    bool hasCurvedSegments() const override { return true; }

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        \param vertex the vertex id
        \returns rotation in radians, clockwise from north*/
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual QgsCircularString *reversed() const override  SIP_FACTORY;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

    double xAt( int index ) const override;
    double yAt( int index ) const override;

  protected:

    virtual QgsRectangle calculateBoundingBox() const override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    static void arcTo( QPainterPath &path, QPointF pt1, QPointF pt2, QPointF pt3 );
    //bounding box of a single segment
    static QgsRectangle segmentBoundingBox( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3 );
    static QgsPointSequence compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius );
    static double closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
                                     const QgsPointV2 &pt, QgsPointV2 &segmentPt,  QgsVertexId &vertexAfter, bool *leftOf, double epsilon );
    void insertVertexBetween( int after, int before, int pointOnCircle );
    void deleteVertex( int i );

};

#endif // QGSCIRCULARSTRING_H
