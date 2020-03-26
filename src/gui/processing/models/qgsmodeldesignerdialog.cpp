/***************************************************************************
                             qgsmodeldesignerdialog.cpp
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldesignerdialog.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgsmessagebar.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsgui.h"
#include "qgsprocessingparametertype.h"
#include "qgsmodelundocommand.h"
#include "qgsmodelviewtoolselect.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelcomponentgraphicitem.h"

#include <QShortcut>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QFileDialog>
#include <QPrinter>
#include <QSvgGenerator>
#include <QToolButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QUndoView>

///@cond NOT_STABLE


QgsModelerToolboxModel::QgsModelerToolboxModel( QObject *parent )
  : QgsProcessingToolboxProxyModel( parent )
{

}

Qt::ItemFlags QgsModelerToolboxModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QgsProcessingToolboxProxyModel::flags( index );
  const QModelIndex sourceIndex = mapToSource( index );
  if ( toolboxModel()->isAlgorithm( sourceIndex ) )
  {
    f = f | Qt::ItemIsDragEnabled;
  }
  return f;
}

Qt::DropActions QgsModelerToolboxModel::supportedDragActions() const
{
  return Qt::CopyAction;
}



QgsModelDesignerDialog::QgsModelDesignerDialog( QWidget *parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  setAttribute( Qt::WA_DeleteOnClose );
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );
  setWindowFlags( Qt::WindowMinimizeButtonHint |
                  Qt::WindowMaximizeButtonHint |
                  Qt::WindowCloseButtonHint );

  mModel = qgis::make_unique< QgsProcessingModelAlgorithm >();
  mModel->setProvider( QgsApplication::processingRegistry()->providerById( QStringLiteral( "model" ) ) );

  mUndoStack = new QUndoStack( this );
  connect( mUndoStack, &QUndoStack::indexChanged, this, [ = ]
  {
    if ( mIgnoreUndoStackChanges )
      return;

    mBlockUndoCommands++;
    updateVariablesGui();
    mGroupEdit->setText( mModel->group() );
    mNameEdit->setText( mModel->displayName() );
    mBlockUndoCommands--;
    repaintModel();
  } );

  mPropertiesDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mInputsDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mAlgorithmsDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mVariablesDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable );

  mAlgorithmsTree->header()->setVisible( false );
  mAlgorithmSearchEdit->setShowSearchIcon( true );
  mAlgorithmSearchEdit->setPlaceholderText( tr( "Search…" ) );
  connect( mAlgorithmSearchEdit, &QgsFilterLineEdit::textChanged, mAlgorithmsTree, &QgsProcessingToolboxTreeView::setFilterString );

  mInputsTreeWidget->header()->setVisible( false );
  mInputsTreeWidget->setAlternatingRowColors( true );
  mInputsTreeWidget->setDragDropMode( QTreeWidget::DragOnly );
  mInputsTreeWidget->setDropIndicatorShown( true );

  mNameEdit->setPlaceholderText( tr( "Enter model name here" ) );
  mGroupEdit->setPlaceholderText( tr( "Enter group name here" ) );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  mainLayout->insertWidget( 0, mMessageBar );

  mView->setAcceptDrops( true );
  QgsSettings settings;

  connect( mActionClose, &QAction::triggered, this, &QWidget::close );
  connect( mActionZoomIn, &QAction::triggered, this, &QgsModelDesignerDialog::zoomIn );
  connect( mActionZoomOut, &QAction::triggered, this, &QgsModelDesignerDialog::zoomOut );
  connect( mActionZoomActual, &QAction::triggered, this, &QgsModelDesignerDialog::zoomActual );
  connect( mActionZoomToItems, &QAction::triggered, this, &QgsModelDesignerDialog::zoomFull );
  connect( mActionExportImage, &QAction::triggered, this, &QgsModelDesignerDialog::exportToImage );
  connect( mActionExportPdf, &QAction::triggered, this, &QgsModelDesignerDialog::exportToPdf );
  connect( mActionExportSvg, &QAction::triggered, this, &QgsModelDesignerDialog::exportToSvg );
  connect( mActionExportPython, &QAction::triggered, this, &QgsModelDesignerDialog::exportAsPython );
  connect( mActionSave, &QAction::triggered, this, [ = ] { saveModel( false ); } );
  connect( mActionSaveAs, &QAction::triggered, this, [ = ] { saveModel( true ); } );
  connect( mActionDeleteComponents, &QAction::triggered, this, &QgsModelDesignerDialog::deleteSelected );
  connect( mActionSnapSelected, &QAction::triggered, mView, &QgsModelGraphicsView::snapSelected );

  mActionSnappingEnabled->setChecked( settings.value( QStringLiteral( "/Processing/Modeler/enableSnapToGrid" ), false ).toBool() );
  connect( mActionSnappingEnabled, &QAction::toggled, this, [ = ]( bool enabled )
  {
    mView->snapper()->setSnapToGrid( enabled );
    QgsSettings().setValue( QStringLiteral( "/Processing/Modeler/enableSnapToGrid" ), enabled );
  } );
  mView->snapper()->setSnapToGrid( mActionSnappingEnabled->isChecked() );

  connect( mActionSelectAll, &QAction::triggered, this, [ = ]
  {
    mScene->selectAll();
  } );

  mUndoAction = mUndoStack->createUndoAction( this );
  mUndoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );
  mUndoAction->setShortcuts( QKeySequence::Undo );
  mRedoAction = mUndoStack->createRedoAction( this );
  mRedoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ) );
  mRedoAction->setShortcuts( QKeySequence::Redo );

  mMenuEdit->insertAction( mActionDeleteComponents, mRedoAction );
  mMenuEdit->insertAction( mActionDeleteComponents, mUndoAction );
  mMenuEdit->insertSeparator( mActionDeleteComponents );
  mToolbar->insertAction( mActionZoomIn, mUndoAction );
  mToolbar->insertAction( mActionZoomIn, mRedoAction );
  mToolbar->insertSeparator( mActionZoomIn );


  QgsProcessingToolboxProxyModel::Filters filters = QgsProcessingToolboxProxyModel::FilterModeler;
  if ( settings.value( QStringLiteral( "Processing/Configuration/SHOW_ALGORITHMS_KNOWN_ISSUES" ), false ).toBool() )
  {
    filters |= QgsProcessingToolboxProxyModel::FilterShowKnownIssues;
  }
  mAlgorithmsTree->setFilters( filters );
  mAlgorithmsTree->setDragDropMode( QTreeWidget::DragOnly );
  mAlgorithmsTree->setDropIndicatorShown( true );

  mAlgorithmsModel = new QgsModelerToolboxModel( this );
  mAlgorithmsTree->setToolboxProxyModel( mAlgorithmsModel );

  connect( mView, &QgsModelGraphicsView::algorithmDropped, this, [ = ]( const QString & algorithmId, const QPointF & pos )
  {
    addAlgorithm( algorithmId, pos );
  } );
  connect( mAlgorithmsTree, &QgsProcessingToolboxTreeView::doubleClicked, this, [ = ]()
  {
    if ( mAlgorithmsTree->selectedAlgorithm() )
      addAlgorithm( mAlgorithmsTree->selectedAlgorithm()->id(), QPointF() );
  } );
  connect( mInputsTreeWidget, &QgsModelDesignerInputsTreeWidget::doubleClicked, this, [ = ]( const QModelIndex & )
  {
    const QString parameterType = mInputsTreeWidget->currentItem()->data( 0, Qt::UserRole ).toString();
    addInput( parameterType, QPointF() );
  } );

  connect( mView, &QgsModelGraphicsView::inputDropped, this, &QgsModelDesignerDialog::addInput );

  // Ctrl+= should also trigger a zoom in action
  QShortcut *ctrlEquals = new QShortcut( QKeySequence( QStringLiteral( "Ctrl+=" ) ), this );
  connect( ctrlEquals, &QShortcut::activated, this, &QgsModelDesignerDialog::zoomIn );

  mUndoDock = new QgsDockWidget( tr( "Undo History" ), this );
  mUndoDock->setObjectName( QStringLiteral( "UndoDock" ) );
  mUndoView = new QUndoView( mUndoStack, this );
  mUndoDock->setWidget( mUndoView );
  mUndoDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable );
  addDockWidget( Qt::DockWidgetArea::LeftDockWidgetArea, mUndoDock );

  tabifyDockWidget( mUndoDock, mPropertiesDock );
  tabifyDockWidget( mVariablesDock, mPropertiesDock );
  mPropertiesDock->raise();
  tabifyDockWidget( mInputsDock, mAlgorithmsDock );
  mInputsDock->raise();

  connect( mVariablesEditor, &QgsVariableEditorWidget::scopeChanged, this, [ = ]
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Variables" ) );
      mModel->setVariables( mVariablesEditor->variablesInActiveScope() );
      endUndoCommand();
    }
  } );
  connect( mNameEdit, &QLineEdit::textChanged, this, [ = ]( const QString & name )
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Name" ), NameChanged );
      mModel->setName( name );
      endUndoCommand();
      updateWindowTitle();
    }
  } );
  connect( mGroupEdit, &QLineEdit::textChanged, this, [ = ]( const QString & group )
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Group" ), GroupChanged );
      mModel->setGroup( group );
      endUndoCommand();
    }
  } );

  fillInputsTree();

  QToolButton *toolbuttonExportToScript = new QToolButton();
  toolbuttonExportToScript->setPopupMode( QToolButton::InstantPopup );
  toolbuttonExportToScript->addAction( mActionExportAsScriptAlgorithm );
  toolbuttonExportToScript->setDefaultAction( mActionExportAsScriptAlgorithm );
  mToolbar->insertWidget( mActionExportImage, toolbuttonExportToScript );
  connect( mActionExportAsScriptAlgorithm, &QAction::triggered, this, &QgsModelDesignerDialog::exportAsScriptAlgorithm );

  mActionShowComments->setChecked( settings.value( QStringLiteral( "/Processing/Modeler/ShowComments" ), true ).toBool() );
  connect( mActionShowComments, &QAction::toggled, this, &QgsModelDesignerDialog::toggleComments );

  mSelectTool = new QgsModelViewToolSelect( mView );
#if 0
  mSelectTool->setAction( mActionSelectMoveItem );
#endif
  mView->setTool( mSelectTool );
  mView->setFocus();

  connect( mView, &QgsModelGraphicsView::macroCommandStarted, this, [ = ]( const QString & text )
  {
    mIgnoreUndoStackChanges++;
    mUndoStack->beginMacro( text );
    mIgnoreUndoStackChanges--;
  } );
  connect( mView, &QgsModelGraphicsView::macroCommandEnded, this, [ = ]
  {
    mIgnoreUndoStackChanges++;
    mUndoStack->endMacro();
    mIgnoreUndoStackChanges--;
  } );


  updateWindowTitle();
}

QgsModelDesignerDialog::~QgsModelDesignerDialog()
{
  mIgnoreUndoStackChanges++;
  delete mSelectTool; // delete mouse handles before everything else
}

void QgsModelDesignerDialog::closeEvent( QCloseEvent *event )
{
  if ( checkForUnsavedChanges() )
    event->accept();
  else
    event->ignore();
}

void QgsModelDesignerDialog::beginUndoCommand( const QString &text, int id )
{
  if ( mBlockUndoCommands || !mUndoStack )
    return;

  if ( mActiveCommand )
    endUndoCommand();

  mActiveCommand = qgis::make_unique< QgsModelUndoCommand >( mModel.get(), text, id );
}

void QgsModelDesignerDialog::endUndoCommand()
{
  if ( mBlockUndoCommands || !mActiveCommand || !mUndoStack )
    return;

  mActiveCommand->saveAfterState();
  mIgnoreUndoStackChanges++;
  mUndoStack->push( mActiveCommand.release() );
  mIgnoreUndoStackChanges--;
  setDirty( true );
}

QgsProcessingModelAlgorithm *QgsModelDesignerDialog::model()
{
  return mModel.get();
}

void QgsModelDesignerDialog::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel.reset( model );

  mGroupEdit->setText( mModel->group() );
  mNameEdit->setText( mModel->displayName() );
  repaintModel();
  updateVariablesGui();

  mView->centerOn( 0, 0 );
  setDirty( false );

  mIgnoreUndoStackChanges++;
  mUndoStack->clear();
  mIgnoreUndoStackChanges--;

  updateWindowTitle();
}

void QgsModelDesignerDialog::loadModel( const QString &path )
{
  std::unique_ptr< QgsProcessingModelAlgorithm > alg = qgis::make_unique< QgsProcessingModelAlgorithm >();
  if ( alg->fromFile( path ) )
  {
    alg->setProvider( QgsApplication::processingRegistry()->providerById( QStringLiteral( "model" ) ) );
    setModel( alg.release() );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Could not load model %1" ).arg( path ), tr( "Processing" ), Qgis::Critical );
    QMessageBox::critical( this, tr( "Open Model" ), tr( "The selected model could not be loaded.\n"
                           "See the log for more information." ) );
  }
}

void QgsModelDesignerDialog::setModelScene( QgsModelGraphicsScene *scene )
{
  QgsModelGraphicsScene *oldScene = mScene;

  mScene = scene;
  mScene->setParent( this );

  mView->setModelScene( mScene );

  mSelectTool->resetCache();
  mSelectTool->setScene( mScene );

  connect( mScene, &QgsModelGraphicsScene::rebuildRequired, this, [ = ]
  {
    if ( mBlockRepaints )
      return;

    repaintModel();
  } );
  connect( mScene, &QgsModelGraphicsScene::componentAboutToChange, this, [ = ]( const QString & description, int id ) { beginUndoCommand( description, id ); } );
  connect( mScene, &QgsModelGraphicsScene::componentChanged, this, [ = ] { endUndoCommand(); } );

  if ( oldScene )
    oldScene->deleteLater();
}

void QgsModelDesignerDialog::updateVariablesGui()
{
  mBlockUndoCommands++;

  std::unique_ptr< QgsExpressionContextScope > variablesScope = qgis::make_unique< QgsExpressionContextScope >( tr( "Model Variables" ) );
  const QVariantMap modelVars = mModel->variables();
  for ( auto it = modelVars.constBegin(); it != modelVars.constEnd(); ++it )
  {
    variablesScope->setVariable( it.key(), it.value() );
  }
  QgsExpressionContext variablesContext;
  variablesContext.appendScope( variablesScope.release() );
  mVariablesEditor->setContext( &variablesContext );
  mVariablesEditor->setEditableScopeIndex( 0 );

  mBlockUndoCommands--;
}

void QgsModelDesignerDialog::setDirty( bool dirty )
{
  mHasChanged = dirty;
  updateWindowTitle();
}

bool QgsModelDesignerDialog::validateSave()
{
  if ( mNameEdit->text().trimmed().isEmpty() )
  {
    mMessageBar->pushWarning( QString(), tr( "Please a enter model name before saving" ) );
    return false;
  }

  return true;
}

bool QgsModelDesignerDialog::checkForUnsavedChanges()
{
  if ( isDirty() )
  {
    QMessageBox::StandardButton ret = QMessageBox::question( this, tr( "Save Model?" ),
                                      tr( "There are unsaved changes in this model. Do you want to keep those?" ),
                                      QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard, QMessageBox::Cancel );
    switch ( ret )
    {
      case QMessageBox::Save:
        saveModel( false );
        return true;

      case QMessageBox::Discard:
        return true;

      default:
        return false;
    }
  }
  else
  {
    return true;
  }
}

void QgsModelDesignerDialog::zoomIn()
{
  mView->setTransformationAnchor( QGraphicsView::NoAnchor );
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  QgsSettings settings;
  const double factor = settings.value( QStringLiteral( "/qgis/zoom_favor" ), 2.0 ).toDouble();
  mView->scale( factor, factor );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomOut()
{
  mView->setTransformationAnchor( QGraphicsView::NoAnchor );
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  QgsSettings settings;
  const double factor = 1.0 / settings.value( QStringLiteral( "/qgis/zoom_favor" ), 2.0 ).toDouble();
  mView->scale( factor, factor );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomActual()
{
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  mView->resetTransform();
  mView->scale( QgsApplication::desktop()->logicalDpiX() / 96, QgsApplication::desktop()->logicalDpiX() / 96 );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomFull()
{
  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  mView->fitInView( totalRect, Qt::KeepAspectRatio );
}

void QgsModelDesignerDialog::exportToImage()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as Image" ), tr( "PNG files (*.png *.PNG)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "png" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF imageRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QImage img( totalRect.width(), totalRect.height(),
              QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );
  QPainter painter;
  painter.setRenderHint( QPainter::Antialiasing );
  painter.begin( &img );
  mView->scene()->render( &painter, imageRect, totalRect );
  painter.end();

  img.save( filename );

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as image to <a href=\"{}\">{}</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::Success, 5 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportToPdf()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as PDF" ), tr( "PDF files (*.pdf *.PDF)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "pdf" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF printerRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QPrinter printer;
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( filename );
  printer.setPaperSize( QSizeF( printerRect.width(), printerRect.height() ), QPrinter::DevicePixel );
  printer.setFullPage( true );

  QPainter painter( &printer );
  mView->scene()->render( &painter, printerRect, totalRect );
  painter.end();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as PDF to <a href=\"{}\">{}</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::Success, 5 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportToSvg()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as SVG" ), tr( "SVG files (*.svg *.SVG)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "svg" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF svgRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QSvgGenerator svg;
  svg.setFileName( filename );
  svg.setSize( QSize( totalRect.width(), totalRect.height() ) );
  svg.setViewBox( svgRect );
  svg.setTitle( mModel->displayName() );

  QPainter painter( &svg );
  mView->scene()->render( &painter, svgRect, totalRect );
  painter.end();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as SVG to <a href=\"{}\">{}</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::Success, 5 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportAsPython()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as Python Script" ), tr( "Processing scripts (*.py *.PY)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "py" ) );

  const QString text = mModel->asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, 4 ).join( '\n' );

  QFile outFile( filename );
  if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &outFile );
  fout << text;
  outFile.close();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as Python script to <a href=\"{}\">{}</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::Success, 5 );
}

void QgsModelDesignerDialog::toggleComments( bool show )
{
  QgsSettings().setValue( QStringLiteral( "/Processing/Modeler/ShowComments" ), show );

  repaintModel( true );
}

void QgsModelDesignerDialog::updateWindowTitle()
{
  QString title = tr( "Model Designer" );
  if ( !mModel->name().isEmpty() )
    title = QStringLiteral( "%1 - %2" ).arg( title, mModel->name() );

  if ( isDirty() )
    title.prepend( '*' );

  setWindowTitle( title );
}

void QgsModelDesignerDialog::deleteSelected()
{
  QList< QgsModelComponentGraphicItem * > items = mScene->selectedComponentItems();
  if ( items.empty() )
    return;

  if ( items.size() == 1 )
  {
    items.at( 0 )->deleteComponent();
    return;
  }

  std::sort( items.begin(), items.end(), []( QgsModelComponentGraphicItem * p1, QgsModelComponentGraphicItem * p2 )
  {
    // try to delete the easy stuff first, so comments, then outputs, as nothing will depend on these...
    if ( dynamic_cast< QgsModelCommentGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelCommentGraphicItem *>( p2 ) )
      return false;
    else if ( dynamic_cast< QgsModelOutputGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelOutputGraphicItem *>( p2 ) )
      return false;
    else if ( dynamic_cast< QgsModelChildAlgorithmGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelChildAlgorithmGraphicItem *>( p2 ) )
      return false;
    return false;
  } );


  beginUndoCommand( tr( "Delete Components" ) );

  QVariant prevState = mModel->toVariant();
  mBlockUndoCommands++;
  mBlockRepaints = true;
  bool failed = false;
  while ( !items.empty() )
  {
    QgsModelComponentGraphicItem *toDelete = nullptr;
    for ( QgsModelComponentGraphicItem *item : items )
    {
      if ( item->canDeleteComponent() )
      {
        toDelete = item;
        break;
      }
    }

    if ( !toDelete )
    {
      failed = true;
      break;
    }

    toDelete->deleteComponent();
    items.removeAll( toDelete );
  }

  if ( failed )
  {
    mModel->loadVariant( prevState );
    QMessageBox::warning( nullptr, QObject::tr( "Could not remove components" ),
                          QObject::tr( "Components depend on the selected items.\n"
                                       "Try to remove them before trying deleting these components." ) );
    mBlockUndoCommands--;
    mActiveCommand.reset();
  }
  else
  {
    mBlockUndoCommands--;
    endUndoCommand();
  }

  mBlockRepaints = false;
  repaintModel();
}

bool QgsModelDesignerDialog::isDirty() const
{
  return mHasChanged && mUndoStack->index() != -1;
}

void QgsModelDesignerDialog::fillInputsTree()
{
  const QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "mIconModelInput.svg" ) );
  std::unique_ptr< QTreeWidgetItem > parametersItem = qgis::make_unique< QTreeWidgetItem >();
  parametersItem->setText( 0, tr( "Parameters" ) );
  QList<QgsProcessingParameterType *> available = QgsApplication::processingRegistry()->parameterTypes();
  std::sort( available.begin(), available.end(), []( const QgsProcessingParameterType * a, const QgsProcessingParameterType * b ) -> bool
  {
    return QString::localeAwareCompare( a->name(), b->name() ) < 0;
  } );

  for ( QgsProcessingParameterType *param : qgis::as_const( available ) )
  {
    if ( param->flags() & QgsProcessingParameterType::ExposeToModeler )
    {
      std::unique_ptr< QTreeWidgetItem > paramItem = qgis::make_unique< QTreeWidgetItem >();
      paramItem->setText( 0, param->name() );
      paramItem->setData( 0, Qt::UserRole, param->id() );
      paramItem->setIcon( 0, icon );
      paramItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
      paramItem->setToolTip( 0, param->description() );
      parametersItem->addChild( paramItem.release() );
    }
  }
  mInputsTreeWidget->addTopLevelItem( parametersItem.release() );
  mInputsTreeWidget->topLevelItem( 0 )->setExpanded( true );
}


///@endcond

