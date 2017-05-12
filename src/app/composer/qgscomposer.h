/***************************************************************************
                         qgscomposer.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSER_H
#define QGSCOMPOSER_H
#include "ui_qgscomposerbase.h"

#include "qgspanelwidget.h"
#include "qgsvectorlayer.h"
#include "qgscomposerinterface.h"

class QgisApp;
class QgsComposerArrow;
class QgsComposerPolygon;
class QgsComposerPolyline;
class QgsComposerFrame;
class QgsComposerHtml;
class QgsComposerLabel;
class QgsComposerLegend;
class QgsComposerPicture;
class QgsComposerPictureWidget;
class QgsComposerRuler;
class QgsComposerScaleBar;
class QgsComposerShape;
class QgsComposerAttributeTable;
class QgsComposerAttributeTableV2;
class QgsComposerView;
class QgsComposition;
class QgsMapCanvas;
class QgsAtlasComposition;
class QgsMapLayerAction;
class QgsComposerMap;
class QgsComposerItem;
class QgsDockWidget;
class QgsMapLayer;
class QgsFeature;
class QgsPanelWidgetStack;

class QGridLayout;
class QDomNode;
class QDomDocument;
class QDomElement;
class QMoveEvent;
class QResizeEvent;
class QFile;
class QSizeGrip;
class QUndoView;
class QComboBox;
class QLabel;
class QTreeView;
class QPrinter;
class QgsComposer;

class QgsAppComposerInterface : public QgsComposerInterface
{
    Q_OBJECT

  public:
    QgsAppComposerInterface( QgsComposer *composer );
    QgsComposerView *view() override;
    QgsComposition *composition() override;
    void close() override;

  private:

    QgsComposer *mComposer = nullptr;
};

/** \ingroup app
 * \brief A gui for composing a printable map.
 */
class QgsComposer: public QMainWindow, private Ui::QgsComposerBase
{
    Q_OBJECT

  public:

    enum OutputMode
    {
      Single = 0,
      Atlas
    };

    QgsComposer( QgsComposition *composition );
    ~QgsComposer();

    QgsComposerInterface *iface();

    //! Set the pixmap / icons on the toolbar buttons
    void setupTheme();

    void setIconSizes( int size );

    //! Open and show, set defaults if first time
    void open();

    //! Zoom to full extent of the paper
    void zoomFull();

    //! Return pointer to map canvas
    QgsMapCanvas *mapCanvas();

    //! Return pointer to composer view
    QgsComposerView *view();

    //! Return current composition
    QgsComposition *composition() { return mComposition; }

    //! Restore the window and toolbar state
    void restoreWindowState();

    /** Loads the contents of a template document into the composer's composition.
     * \param templateDoc template document to load
     * \param clearExisting set to true to remove all existing composition settings and items before loading template
     * \returns true if template load was successful
     */
    bool loadFromTemplate( const QDomDocument &templateDoc, bool clearExisting );

    //! Sets the specified feature as the current atlas feature
    //! \since QGIS 2.1
    void setAtlasFeature( QgsMapLayer *layer, const QgsFeature &feat );

  protected:
    //! Move event
    virtual void moveEvent( QMoveEvent * ) override;

    virtual void closeEvent( QCloseEvent * ) override;

    //! Resize event
    virtual void resizeEvent( QResizeEvent * ) override;

  signals:
    //! Is emitted every time the view zoom has changed
    void zoomLevelChanged();

    //! Is emitted when the atlas preview feature changes
    void atlasPreviewFeatureChanged();

    void aboutToClose();

  public slots:

    //! Zoom to full extent of the paper
    void on_mActionZoomAll_triggered();

    //! Zoom in
    void on_mActionZoomIn_triggered();

    //! Zoom out
    void on_mActionZoomOut_triggered();

    //! Zoom actual
    void on_mActionZoomActual_triggered();

    //! Refresh view
    void on_mActionRefreshView_triggered();

    //! Print the composition
    void on_mActionPrint_triggered();

    //! Page Setup for composition
    void on_mActionPageSetup_triggered();

    //! Print as image
    void on_mActionExportAsImage_triggered();

    //! Print as SVG
    void on_mActionExportAsSVG_triggered();

    //! Print as PDF
    void on_mActionExportAsPDF_triggered();

    //! Select item
    void on_mActionSelectMoveItem_triggered();

    //! Add arrow
    void on_mActionAddArrow_triggered();

    //! Add new map
    void on_mActionAddNewMap_triggered();

    //! Add new legend
    void on_mActionAddNewLegend_triggered();

    //! Add new label
    void on_mActionAddNewLabel_triggered();

    //! Add new scalebar
    void on_mActionAddNewScalebar_triggered();

    //! Add new picture
    void on_mActionAddImage_triggered();

    void on_mActionAddRectangle_triggered();

    void on_mActionAddTriangle_triggered();

    void on_mActionAddEllipse_triggered();

    //! Nodes based shape
    void on_mActionEditNodesItem_triggered();
    void on_mActionAddPolygon_triggered();
    void on_mActionAddPolyline_triggered();

    //! Add attribute table
    void on_mActionAddTable_triggered();

    //! Add attribute table
    void on_mActionAddAttributeTable_triggered();

    void on_mActionAddHtml_triggered();

    //! Save parent project
    void on_mActionSaveProject_triggered();

    //! Create new composer
    void on_mActionNewComposer_triggered();

    //! Duplicate current composer
    void on_mActionDuplicateComposer_triggered();

    //! Show composer manager

    void on_mActionComposerManager_triggered();

    //! Save composer as template
    void on_mActionSaveAsTemplate_triggered();

    void on_mActionLoadFromTemplate_triggered();

    //! Set tool to move item content
    void on_mActionMoveItemContent_triggered();

    //! Set tool to move item content
    void on_mActionPan_triggered();

    //! Set tool to mouse zoom
    void on_mActionMouseZoom_triggered();

    //! Group selected items
    void on_mActionGroupItems_triggered();

    //! Cut item(s)
    void actionCutTriggered();

    //! Copy item(s)
    void actionCopyTriggered();

    //! Paste item(s)
    void actionPasteTriggered();

    //! Paste in place item(s)
    void on_mActionPasteInPlace_triggered();

    //! Delete selected item(s)
    void on_mActionDeleteSelection_triggered();

    //! Select all items
    void on_mActionSelectAll_triggered();

    //! Deselect all items
    void on_mActionDeselectAll_triggered();

    //! Invert selection
    void on_mActionInvertSelection_triggered();

    //! Ungroup selected item group
    void on_mActionUngroupItems_triggered();

    //! Lock selected items
    void on_mActionLockItems_triggered();

    //! Unlock all items
    void on_mActionUnlockAll_triggered();

    //! Select next item below
    void on_mActionSelectNextAbove_triggered();

    //! Select next item above
    void on_mActionSelectNextBelow_triggered();

    //! Move selected items one position up
    void on_mActionRaiseItems_triggered();

    //!Move selected items one position down
    void on_mActionLowerItems_triggered();

    //!Move selected items to top
    void on_mActionMoveItemsToTop_triggered();

    //!Move selected items to bottom
    void on_mActionMoveItemsToBottom_triggered();

    //!Align selected composer items left
    void on_mActionAlignLeft_triggered();

    //!Align selected composere items horizontally centered
    void on_mActionAlignHCenter_triggered();

    //!Align selected composer items right
    void on_mActionAlignRight_triggered();

    //!Align selected composer items to top
    void on_mActionAlignTop_triggered();

    //!Align selected composer items vertically centered
    void on_mActionAlignVCenter_triggered();

    //!Align selected composer items to bottom
    void on_mActionAlignBottom_triggered();

    //!Undo last composer change
    void on_mActionUndo_triggered();

    //!Redo last composer change
    void on_mActionRedo_triggered();

    //!Show/hide grid
    void on_mActionShowGrid_triggered( bool checked );

    //!Enable or disable snap items to grid
    void on_mActionSnapGrid_triggered( bool checked );

    //!Show/hide guides
    void on_mActionShowGuides_triggered( bool checked );

    //!Enable or disable snap items to guides
    void on_mActionSnapGuides_triggered( bool checked );

    //!Enable or disable smart guides
    void on_mActionSmartGuides_triggered( bool checked );

    //!Show/hide bounding boxes
    void on_mActionShowBoxes_triggered( bool checked );

    //!Show/hide pages
    void on_mActionShowPage_triggered( bool checked );

    //!Show/hide rulers
    void toggleRulers( bool checked );

    //!Clear guides
    void on_mActionClearGuides_triggered();

    //!Show options dialog
    void on_mActionOptions_triggered();

    //!Toggle atlas preview
    void on_mActionAtlasPreview_triggered( bool checked );

    //!Next atlas feature
    void on_mActionAtlasNext_triggered();

    //!Previous atlas feature
    void on_mActionAtlasPrev_triggered();

    //!First atlas feature
    void on_mActionAtlasFirst_triggered();

    //!Last atlas feature
    void on_mActionAtlasLast_triggered();

    //!Jump to a specific atlas page
    void atlasPageComboEditingFinished();

    //! Print the atlas
    void on_mActionPrintAtlas_triggered();

    //! Print atlas as image
    void on_mActionExportAtlasAsImage_triggered();

    //! Print atlas as SVG
    void on_mActionExportAtlasAsSVG_triggered();

    //! Print atlas as PDF
    void on_mActionExportAtlasAsPDF_triggered();

    //! Atlas settings
    void on_mActionAtlasSettings_triggered();

    //! Toggle full screen mode
    void on_mActionToggleFullScreen_triggered();

    //! Toggle panels
    void on_mActionHidePanels_triggered();

    //! Save window state
    void saveWindowState();

    //! Removes item from the item/widget map and deletes the configuration widget. Does not delete the item itself
    void deleteItem( QgsComposerItem *item );

    //! Shows the configuration widget for a composer item
    void showItemOptions( QgsComposerItem *i );

    void setSelectionTool();

    //! Raise, unminimize and activate this window
    void activate();

    //! Updates cursor position in status bar
    void updateStatusCursorPos( QPointF position );

    //! Updates zoom level in status bar
    void updateStatusZoom();

    void statusZoomCombo_currentIndexChanged( int index );

    void statusZoomCombo_zoomEntered();

    //! Updates status bar composition message
    void updateStatusCompositionMsg( const QString &message );

    //! Updates status bar atlas message
    void updateStatusAtlasMsg( const QString &message );

  private:

    //! Establishes the signal slot connections from the QgsComposerView to the composer
    void connectViewSlots();

    //! Establishes the signal slot connections from the QgsComposition to the composer
    void connectCompositionSlots();

    //! Establishes other signal slot connections for the composer
    void connectOtherSlots();

    //! Creates the composition widget
    void createCompositionWidget();

    //! True if a composer map contains a WMS layer
    bool containsWmsLayer() const;

    //! True if a composer contains advanced effects, such as blend modes
    bool containsAdvancedEffects() const;

    //! Displays a warning because of possible min/max size in WMS
    void showWmsPrintingWarning();

    //! Displays a warning because of incompatibility between blend modes and QPrinter
    void showAdvancedEffectsWarning();

    //! Changes elements that are not suitable for this project
    void cleanupAfterTemplateRead();

    //! Create composer view and rulers
    void createComposerView();

    //! Write a world file
    void writeWorldFile( const QString &fileName, double a, double b, double c, double d, double e, double f ) const;

    //! Updates the grid/guide action status based on compositions grid/guide settings
    void restoreGridSettings();

    //! Prints either the whole atlas or just the current feature, depending on mode
    void printComposition( QgsComposer::OutputMode mode );

    //! Exports either either the whole atlas or just the current feature as an image, depending on mode
    void exportCompositionAsImage( QgsComposer::OutputMode mode );

    //! Exports either either the whole atlas or just the current feature as an SVG, depending on mode
    void exportCompositionAsSVG( QgsComposer::OutputMode mode );

    //! Exports either either the whole atlas or just the current feature as a PDF, depending on mode
    void exportCompositionAsPDF( QgsComposer::OutputMode mode );

    //! Load predefined scales from the project's properties
    void loadAtlasPredefinedScalesFromProject();

    QPrinter *printer();

    QgsPanelWidget *createItemWidget( QgsComposerItem *item );

    QgsAppComposerInterface *mInterface = nullptr;

    //! Labels in status bar which shows current mouse position
    QLabel *mStatusCursorXLabel = nullptr;
    QLabel *mStatusCursorYLabel = nullptr;
    QLabel *mStatusCursorPageLabel = nullptr;
    //! Combobox in status bar which shows/adjusts current zoom level
    QComboBox *mStatusZoomCombo = nullptr;
    QList<double> mStatusZoomLevelsList;
    //! Label in status bar which shows messages from the composition
    QLabel *mStatusCompositionLabel = nullptr;
    //! Label in status bar which shows atlas details
    QLabel *mStatusAtlasLabel = nullptr;

    //! Pointer to composer view
    QgsComposerView *mView = nullptr;
    QGridLayout *mViewLayout = nullptr;
    QgsComposerRuler *mHorizontalRuler = nullptr;
    QgsComposerRuler *mVerticalRuler = nullptr;
    QWidget *mRulerLayoutFix = nullptr;

    //! Current composition
    QgsComposition *mComposition = nullptr;

    //! Pointer to QGIS application
    QgisApp *mQgis = nullptr;

    //! Layout
    QGridLayout *mItemOptionsLayout = nullptr;

    //! Size grip
    QSizeGrip *mSizeGrip = nullptr;

    //! Copy/cut/paste actions
    QAction *mActionCut = nullptr;
    QAction *mActionCopy = nullptr;
    QAction *mActionPaste = nullptr;

    //! Page & Printer Setup
    QPrinter *mPrinter = nullptr;
    bool mSetPageOrientation = false;

    QUndoView *mUndoView = nullptr;

    //! Preview mode actions
    QAction *mActionPreviewModeOff = nullptr;
    QAction *mActionPreviewModeGrayscale = nullptr;
    QAction *mActionPreviewModeMono = nullptr;
    QAction *mActionPreviewProtanope = nullptr;
    QAction *mActionPreviewDeuteranope = nullptr;

    QComboBox *mAtlasPageComboBox = nullptr;

    QgsDockWidget *mItemDock = nullptr;
    QgsPanelWidgetStack *mItemPropertiesStack = nullptr;
    QgsDockWidget *mUndoDock = nullptr;
    QgsDockWidget *mGeneralDock = nullptr;
    QgsPanelWidgetStack *mGeneralPropertiesStack = nullptr;
    QgsDockWidget *mAtlasDock = nullptr;
    QgsDockWidget *mItemsDock = nullptr;

    QTreeView *mItemsTreeView = nullptr;

    QMenu *mPanelMenu = nullptr;
    QMenu *mToolbarMenu = nullptr;

    //! Print Composers menu as mirror of main app's
    QMenu *mPrintComposersMenu = nullptr;

    //! Window menu as mirror of main app's (on Mac)
    QMenu *mWindowMenu = nullptr;

    //! Help menu as mirror of main app's (on Mac)
    QMenu *mHelpMenu = nullptr;

    struct PanelStatus
    {
      PanelStatus( bool visible = true, bool active = false )
        : isVisible( visible )
        , isActive( active )
      {}
      bool isVisible;
      bool isActive;
    };

    QMap< QString, PanelStatus > mPanelStatus;

  signals:
    void printAsRasterChanged( bool state );

  private slots:

    //! Populate Print Composers menu from main app's
    void populatePrintComposersMenu();

    //! Populate Window menu from main app's (on Mac)
    void populateWindowMenu();

    //! Populate Help menu from main app's (on Mac)
    void populateHelpMenu();

    //! Populate one menu from another menu (for Mac)
    void populateWithOtherMenu( QMenu *thisMenu, QMenu *otherMenu );

    //! Create a duplicate of a menu (for Mac)
    QMenu *mirrorOtherMenu( QMenu *otherMenu );

    //! Toggles the state of the atlas preview and navigation controls
    //! \since QGIS 2.1
    void toggleAtlasControls( bool atlasEnabled );

    //! Sets the printer page orientation when the page orientation changes
    void pageOrientationChanged( const QString &orientation );

    void setPrinterPageOrientation();

    void disablePreviewMode();
    void activateGrayscalePreview();
    void activateMonoPreview();
    void activateProtanopePreview();
    void activateDeuteranopePreview();

    void dockVisibilityChanged( bool visible );

    /** Repopulates the atlas page combo box with valid items.
     */
    void updateAtlasPageComboBox( int pageCount );

    void atlasFeatureChanged( QgsFeature *feature );

    void invalidateCachedRenders();



};

#endif

