/***************************************************************************
                             qgslayoutview.h
                             ---------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTVIEW_H
#define QGSLAYOUTVIEW_H

#include "qgis.h"
#include "qgsprevieweffect.h" // for QgsPreviewEffect::PreviewMode
#include "qgis_gui.h"
#include <QPointer>
#include <QGraphicsView>

class QgsLayout;
class QgsLayoutViewTool;
class QgsLayoutViewToolTemporaryKeyPan;
class QgsLayoutViewToolTemporaryKeyZoom;
class QgsLayoutViewToolTemporaryMousePan;
class QgsLayoutRuler;

/**
 * \ingroup gui
 * A graphical widget to display and interact with QgsLayouts.
 *
 * QgsLayoutView manages the layout interaction tools and mouse/key events.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutView: public QGraphicsView
{

    Q_OBJECT

    Q_PROPERTY( QgsLayout *currentLayout READ currentLayout WRITE setCurrentLayout NOTIFY layoutSet )
    Q_PROPERTY( QgsLayoutViewTool *tool READ tool WRITE setTool NOTIFY toolSet )

  public:

    /**
     * Constructor for QgsLayoutView.
     */
    QgsLayoutView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current layout associated with the view.
     * \see setCurrentLayout()
     * \see layoutSet()
     */
    QgsLayout *currentLayout();

    /**
     * Sets the current \a layout to edit in the view.
     * \see currentLayout()
     * \see layoutSet()
     */
    void setCurrentLayout( QgsLayout *layout SIP_KEEPREFERENCE );

    /**
     * Returns the currently active tool for the view.
     * \see setTool()
     */
    QgsLayoutViewTool *tool();

    /**
     * Sets the \a tool currently being used in the view.
     * \see unsetTool()
     * \see tool()
     */
    void setTool( QgsLayoutViewTool *tool );

    /**
     * Unsets the current view tool, if it matches the specified \a tool.
     *
     * This is called from destructor of view tools to make sure
     * that the tool won't be used any more.
     * You don't have to call it manually, QgsLayoutViewTool takes care of it.
     */
    void unsetTool( QgsLayoutViewTool *tool );

    /**
     * Scales the view in a safe way, by limiting the acceptable range
     * of the scale applied. The \a scale parameter specifies the zoom factor to scale the view by.
     */
    void scaleSafe( double scale );

    /**
     * Sets the zoom \a level for the view, where a zoom level of 1.0 corresponds to 100%.
     */
    void setZoomLevel( double level );

    /**
     * Sets a horizontal \a ruler to synchronize with the view state.
     * \see setVerticalRuler()
     */
    void setHorizontalRuler( QgsLayoutRuler *ruler );

    /**
     * Sets a vertical \a ruler to synchronize with the view state.
     * \see setHorizontalRuler()
     */
    void setVerticalRuler( QgsLayoutRuler *ruler );

  public slots:

    /**
     * Zooms the view to the full extent of the layout.
     * \see zoomIn()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomFull();

    /**
     * Zooms the view to the full width of the layout.
     * \see zoomIn()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomWidth();

    /**
     * Zooms in to the view by a preset amount.
     * \see zoomFull()
     * \see zoomOut()
     * \see zoomActual()
     */
    void zoomIn();

    /**
     * Zooms out of the view by a preset amount.
     * \see zoomFull()
     * \see zoomIn()
     * \see zoomActual()
     */
    void zoomOut();

    /**
     * Zooms to the actual size of the layout.
     * \see zoomFull()
     * \see zoomIn()
     * \see zoomOut()
     */
    void zoomActual();

    /**
     * Emits the zoomLevelChanged() signal. This should be called after
     * calling any of the QGraphicsView base class methods which alter
     * the view's zoom level, i.e. QGraphicsView::fitInView().
     */
    // NOTE - I realize these emitXXX methods are gross, but there's no clean
    // alternative here. We can't override the non-virtual Qt QGraphicsView
    // methods, and adding our own renamed methods which call the base class
    // methods also adds noise to the API.
    void emitZoomLevelChanged();

    /**
     * Updates associated rulers after view extent or zoom has changed.
     * This should be called after calling any of the QGraphicsView
     * base class methods which alter the view's zoom level or extent,
     * i.e. QGraphicsView::fitInView().
     */
    void updateRulers();

  signals:

    /**
     * Emitted when a \a layout is set for the view.
     * \see currentLayout()
     * \see setCurrentLayout()
     */
    void layoutSet( QgsLayout *layout );

    /**
     * Emitted when the current \a tool is changed.
     * \see setTool()
     */
    void toolSet( QgsLayoutViewTool *tool );

    /**
     * Is emitted whenever the zoom level of the view is changed.
     */
    void zoomLevelChanged();

    /**
     * Is emitted when the mouse cursor coordinates change within the view.
     * The \a layoutPoint argument indicates the cursor position within
     * the layout coordinate system.
     */
    void cursorPosChanged( QPointF layoutPoint );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;

  private slots:

  private:

    //! Zoom layout from a mouse wheel event
    void wheelZoom( QWheelEvent *event );

    QPointer< QgsLayoutViewTool > mTool;

    QgsLayoutViewToolTemporaryKeyPan *mSpacePanTool = nullptr;
    QgsLayoutViewToolTemporaryMousePan *mMidMouseButtonPanTool = nullptr;
    QgsLayoutViewToolTemporaryKeyZoom *mSpaceZoomTool = nullptr;

    QPoint mMouseCurrentXY;

    QgsLayoutRuler *mHorizontalRuler = nullptr;
    QgsLayoutRuler *mVerticalRuler = nullptr;

    friend class TestQgsLayoutView;

};

#endif // QGSLAYOUTVIEW_H
