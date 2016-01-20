/***************************************************************************
    qgsmaptoolcapture.h  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCAPTURE_H
#define QGSMAPTOOLCAPTURE_H


#include "qgsmaptooladvanceddigitizing.h"
#include "qgscompoundcurvev2.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgslayertreeview.h"

#include <QPoint>
#include <QList>

class QgsRubberBand;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;

class GUI_EXPORT QgsMapToolCapture : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolCapture( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget, CaptureMode mode = CaptureNone );

    //! destructor
    virtual ~QgsMapToolCapture();

    //! active the tool
    virtual void activate() override;

    //! deactive the tool
    virtual void deactivate() override;

    /** Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS*/
    int addCurve( QgsCurveV2* c );

    /**
     * Get the capture curve
     *
     * @return Capture curve
     */
    const QgsCompoundCurveV2* captureCurve() const { return &mCaptureCurve; }


    /**
     * Update the rubberband according to mouse position
     *
     * @param e The mouse event
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent * e ) override;

    /**
     * Intercept key events like Esc or Del to delete the last point
     * @param e key event
     */
    virtual void keyPressEvent( QKeyEvent* e ) override;

#ifdef Q_OS_WIN
    virtual bool eventFilter( QObject *obj, QEvent *e ) override;
#endif

    /**
     * Clean a temporary rubberband
     */
    void deleteTempRubberBand();

  private slots:
    void validationFinished();
    void currentLayerChanged( QgsMapLayer *layer );
    void addError( QgsGeometry::Error );


  protected:
    int nextPoint( const QgsPoint& mapPoint, QgsPoint& layerPoint );
    int nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    /** Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
      * @param mapPoint The point in map coordinates
      * @param layerPoint The point in the crs of the current layer
      * @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed*/
    int addVertex( const QgsPoint& mapPoint, const QgsPoint& layerPoint );

    /** Removes the last vertex from mRubberBand and mCaptureList*/
    void undo();

    /**
     * Start capturing
     */
    void startCapturing();

    /**
     * Are we currently capturing?
     *
     * @return Is the tool in capture mode?
     */
    bool isCapturing() const;

    /**
     * Stop capturing
     */
    void stopCapturing();

    /**
     * Number of points digitized
     *
     * @return Number of points
     */
    int size();

    /**
     * List of digitized points
     * @return List of points
     */
    QList<QgsPoint> points();

    /**
     * Set the points on which to work
     *
     * @param pointList A list of points
     */
    void setPoints( const QList<QgsPoint>& pointList );

    /**
     * Close an open polygon
     */
    void closePolygon();

    // deprecated methods

    /** Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates).
      * @deprecated Use addVertex( mapPoint, layerPoint ) instead
      * @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed*/
    Q_DECL_DEPRECATED int addVertex( const QgsPoint& point );

  private:
    //! whether tracing has been requested by the user
    bool tracingEnabled();
    //! first point that will be used as a start of the trace
    QgsPoint tracingStartPoint();
    //! handle of mouse movement when tracing enabled and capturing has started
    bool tracingMouseMove( QgsMapMouseEvent* e );
    //! handle of addition of clicked point (with the rest of the trace) when tracing enabled
    bool tracingAddVertex( const QgsPoint& point );

  private:
    /** Flag to indicate a map canvas capture operation is taking place */
    bool mCapturing;

    /** Rubber band for polylines and polygons */
    QgsRubberBand* mRubberBand;

    /** Temporary rubber band for polylines and polygons. this connects the last added point to the mouse cursor position */
    QgsRubberBand* mTempRubberBand;

    /** List to store the points of digitised lines and polygons (in layer coordinates)*/
    QgsCompoundCurveV2 mCaptureCurve;

    void validateGeometry();
    QStringList mValidationWarnings;
    QgsGeometryValidator *mValidator;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;

    bool mCaptureModeFromLayer;

    QgsVertexMarker* mSnappingMarker;

#ifdef Q_OS_WIN
    int mSkipNextContextMenuEvent;
#endif
};

#endif
