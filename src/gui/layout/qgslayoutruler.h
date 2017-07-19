/***************************************************************************
                             qgslayoutruler.h
                             ----------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTRULER_H
#define QGSLAYOUTRULER_H

#include "qgscomposeritem.h"
#include <QWidget>
#include "qgis_gui.h"

class QgsLayout;
class QGraphicsLineItem;
class QgsLayoutView;

/**
 * \ingroup gui
 * A custom ruler widget for use with QgsLayoutView, displaying the
 * current zoom and position of the visible layout and for interacting
 * with guides in a layout.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutRuler: public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutRuler, with the specified \a parent widget and \a orientation.
     */
    explicit QgsLayoutRuler( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::Orientation orientation = Qt::Horizontal );

    QSize minimumSizeHint() const override;

    /**
     * Sets the current scene \a transform. This is usually the transform set for a view
     * showing the associated scene, in order to synchronize the view's display of
     * the scene with the rulers.
     */
    void setSceneTransform( const QTransform &transform );

    /**
     * Returns the current layout view associated with the ruler.
     * \see setLayoutView()
     */
    QgsLayoutView *layoutView() { return mView; }

    /**
     * Sets the current layout \a view to synchronize the ruler with.
     * \see layoutView()
     */
    void setLayoutView( QgsLayoutView *view );

    /**
     * Returns the ruler size (either the height of a horizontal ruler or the
     * width of a vertical rule).
     */
    int rulerSize() const { return mRulerMinSize; }

  public slots:

    /**
     * Updates the \a position of the marker showing the current mouse position within
     * the view.
     * \a position is in layout coordinates.
     */
    void setCursorPosition( QPointF position );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;

  private:
    static const int VALID_SCALE_MULTIPLES[];
    static const int VALID_SCALE_MAGNITUDES[];

    Qt::Orientation mOrientation = Qt::Horizontal;
    QgsLayoutView *mView = nullptr;

    QTransform mTransform;
    QPointF mMarkerPos;

    QFont mRulerFont;
    std::unique_ptr< QFontMetrics > mRulerFontMetrics;

    double mScaleMinPixelsWidth = 0.0;
    int mRulerMinSize;
    int mMinPixelsPerDivision;
    int mPixelsBetweenLineAndText;
    int mTextBaseline;
    int mMinSpacingVerticalLabels;

    //! Calculates the optimum labeled units for ruler so that labels are a good distance apart
    int optimumScale( double minPixelDiff, int &magnitude, int &multiple );

    /**
     * Calculate the number of small divisions for each ruler unit, ensuring that they
     * are sufficiently spaced.
     */
    int optimumNumberDivisions( double rulerScale, int scaleMultiple );

    //! Draws vertical text on a painter
    void drawRotatedText( QPainter *painter, QPointF pos, const QString &text );

    /**
     * Draws small ruler divisions.
     * Starting at startPos in mm, for numDivisions divisions, with major division spacing of rulerScale (in mm)
     * Stop drawing if position exceeds maxPos
     */
    void drawSmallDivisions( QPainter *painter, double startPos, int numDivisions, double rulerScale, double maxPos = 0 );

    //! Draw current marker pos on ruler
    void drawMarkerPos( QPainter *painter );

  signals:
    //! Is emitted when mouse cursor coordinates change
    void cursorPosChanged( QPointF );

};

#endif // QGSLAYOUTRULER_H
