/***************************************************************************
                             qgsmodelcomponentgraphicitem.cpp
                             ----------------------------------
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

#include "qgsmodelcomponentgraphicitem.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingmodeloutput.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsapplication.h"
#include "qgsmodelgraphicitem.h"
#include "qgsprocessingmodelalgorithm.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QMenu>

///@cond NOT_STABLE

QgsModelComponentGraphicItem::QgsModelComponentGraphicItem( QgsProcessingModelComponent *component, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QGraphicsObject( parent )
  , mComponent( component )
  , mModel( model )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsMovable, true );
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );
  setZValue( QgsModelGraphicsScene::ZValues::ModelComponent );

  mFont.setPixelSize( 12 );

  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mActionEditModelComponent.svg" ) ) );
  QPicture editPicture;
  QPainter painter( &editPicture );
  svg.render( &painter );
  painter.end();
  mEditButton = new QgsModelDesignerFlatButtonGraphicItem( this, editPicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               component->size().height() / 2.0 - mButtonSize.height() / 2.0 ) );
  connect( mEditButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::editComponent );

  QSvgRenderer svg2( QgsApplication::iconPath( QStringLiteral( "mActionDeleteModelComponent.svg" ) ) );
  QPicture deletePicture;
  painter.begin( &deletePicture );
  svg2.render( &painter );
  painter.end();
  mDeleteButton = new QgsModelDesignerFlatButtonGraphicItem( this, deletePicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               mButtonSize.height() / 2.0 - component->size().height() / 2.0 ) );
  connect( mDeleteButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::deleteComponent );
}

QgsModelComponentGraphicItem::~QgsModelComponentGraphicItem() = default;

QgsProcessingModelComponent *QgsModelComponentGraphicItem::component()
{
  return mComponent.get();
}

const QgsProcessingModelComponent *QgsModelComponentGraphicItem::component() const
{
  return mComponent.get();
}

QgsProcessingModelAlgorithm *QgsModelComponentGraphicItem::model()
{
  return mModel;
}

QFont QgsModelComponentGraphicItem::font() const
{
  return mFont;
}

void QgsModelComponentGraphicItem::setFont( const QFont &font )
{
  mFont = font;
  update();
}

void QgsModelComponentGraphicItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
  editComponent();
}

void QgsModelComponentGraphicItem::hoverEnterEvent( QGraphicsSceneHoverEvent *event )
{
  updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
  updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * )
{
  setToolTip( QString() );
  if ( mIsHovering )
  {
    mIsHovering = false;
    update();
    emit repaintArrows();
  }
}

QVariant QgsModelComponentGraphicItem::itemChange( QGraphicsItem::GraphicsItemChange change, const QVariant &value )
{
  switch ( change )
  {
    case QGraphicsItem::ItemPositionHasChanged:
    {
      emit updateArrowPaths();
      mComponent->setPosition( pos() );

      // also need to update the model's stored component's position
      // TODO - this is not so nice, consider moving this to model class
      if ( QgsProcessingModelChildAlgorithm *child = dynamic_cast< QgsProcessingModelChildAlgorithm * >( mComponent.get() ) )
        mModel->childAlgorithm( child->childId() ).setPosition( pos() );
      else if ( QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( mComponent.get() ) )
        mModel->parameterComponent( param->parameterName() ).setPosition( pos() );
      else if ( QgsProcessingModelOutput *output = dynamic_cast< QgsProcessingModelOutput * >( mComponent.get() ) )
        mModel->childAlgorithm( output->childId() ).modelOutput( output->name() ).setPosition( pos() );

      break;
    }
    case QGraphicsItem::ItemSelectedChange:
    {
      emit repaintArrows();
      break;
    }

    case QGraphicsItem::ItemSceneChange:
    {
      if ( !mInitialized )
      {
        // ideally would be in constructor, but cannot call virtual methods from that...
        if ( linkPointCount( Qt::TopEdge ) )
        {
          QPointF pt = linkPoint( Qt::TopEdge, -1 );
          pt = QPointF( 0, pt.y() );
          mExpandTopButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::TopEdge ), pt );
          connect( mExpandTopButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::TopEdge, folded ); } );
        }
        if ( linkPointCount( Qt::BottomEdge ) )
        {
          QPointF pt = linkPoint( Qt::BottomEdge, -1 );
          pt = QPointF( 0, pt.y() );
          mExpandBottomButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::BottomEdge ), pt );
          connect( mExpandBottomButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::BottomEdge, folded ); } );
        }
      }
      break;
    }

    default:
      break;
  }

  return QGraphicsObject::itemChange( change, value );
}

QRectF QgsModelComponentGraphicItem::boundingRect() const
{
  QFontMetricsF fm( mFont );
  const int linksAbove = mComponent->linksCollapsed( Qt::TopEdge ) ? 0 : linkPointCount( Qt::TopEdge );
  const int linksBelow = mComponent->linksCollapsed( Qt::BottomEdge ) ? 0 : linkPointCount( Qt::BottomEdge );

  const double hUp = fm.height() * 1.2 * ( linksAbove + 2 );
  const double hDown = fm.height() * 1.2 * ( linksBelow + 2 );
  return QRectF( -( mComponent->size().width() + 2 ) / 2,
                 -( mComponent->size().height() + 2 ) / 2 - hUp,
                 mComponent->size().width() + 2,
                 mComponent->size().height() + hDown + hUp );
}

void QgsModelComponentGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  const QRectF rect = itemRect();
  QColor color = fillColor( state() );
  QColor stroke = strokeColor( state() );

  painter->setPen( QPen( stroke, 0 ) ); // 0 width "cosmetic" pen
  painter->setBrush( QBrush( color, Qt::SolidPattern ) );
  painter->drawRect( rect );
  painter->setFont( font() );
  painter->setPen( QPen( textColor( state() ) ) );

  QString text = truncatedTextForItem( label() );

  const QSizeF componentSize = mComponent->size();

  QFontMetricsF fm( font() );
  double h = fm.ascent();
  QPointF pt( -componentSize.width() / 2 + 25, componentSize.height() / 2.0 - h + 1 );
  painter->drawText( pt, text );
  painter->setPen( QPen( QApplication::palette().color( QPalette::WindowText ) ) );

  if ( linkPointCount( Qt::TopEdge ) || linkPointCount( Qt::BottomEdge ) )
  {
    h = -( fm.height() * 1.2 );
    h = h - componentSize.height() / 2.0 + 5;
    pt = QPointF( -componentSize.width() / 2 + 25, h );
    painter->drawText( pt, QObject::tr( "In" ) );
    int i = 1;
    if ( !mComponent->linksCollapsed( Qt::TopEdge ) )
    {
      for ( int idx = 0; idx < linkPointCount( Qt::TopEdge ); ++idx )
      {
        text = linkPointText( Qt::TopEdge, idx );
        h = -( fm.height() * 1.2 ) * ( i + 1 );
        h = h - componentSize.height() / 2.0 + 5;
        pt = QPointF( -componentSize.width() / 2 + 33, h );
        painter->drawText( pt, text );
        i += 1;
      }
    }

    h = fm.height() * 1.1;
    h = h + componentSize.height() / 2.0;
    pt = QPointF( -componentSize.width() / 2 + 25, h );
    painter->drawText( pt, QObject::tr( "Out" ) );
    if ( !mComponent->linksCollapsed( Qt::BottomEdge ) )
    {
      for ( int idx = 0; idx < linkPointCount( Qt::BottomEdge ); ++idx )
      {
        text = linkPointText( Qt::BottomEdge, idx );
        h = fm.height() * 1.2 * ( idx + 2 );
        h = h + componentSize.height() / 2.0;
        pt = QPointF( -componentSize.width() / 2 + 33, h );
        painter->drawText( pt, text );
      }
    }
  }

  const QPixmap px = iconPixmap();
  if ( !px.isNull() )
  {
    painter->drawPixmap( -( componentSize.width() / 2.0 ) + 3, -8, px );
  }
  else
  {
    const QPicture pic = iconPicture();
    if ( !pic.isNull() )
    {
      painter->drawPicture( -( componentSize.width() / 2.0 ) + 3, -8, pic );
    }
  }
}

QRectF QgsModelComponentGraphicItem::itemRect() const
{
  return QRectF( -( mComponent->size().width() + 2 ) / 2.0,
                 -( mComponent->size().height() + 2 ) / 2.0,
                 mComponent->size().width() + 2,
                 mComponent->size().height() + 2 );
}

QString QgsModelComponentGraphicItem::truncatedTextForItem( const QString &text ) const
{
  QFontMetricsF fm( mFont );
  double width = fm.boundingRect( text ).width();
  if ( width < mComponent->size().width() - 25 - mButtonSize.width() )
    return text;

  QString t = text;
  t = t.left( t.length() - 3 ) + QChar( 0x2026 );
  width = fm.boundingRect( t ).width();
  while ( width > mComponent->size().width() - 25 - mButtonSize.width() )
  {
    t = t.left( t.length() - 4 ) + QChar( 0x2026 );
    width = fm.boundingRect( t ).width();
  }
  return t;
}

QPicture QgsModelComponentGraphicItem::iconPicture() const
{
  return QPicture();
}

QPixmap QgsModelComponentGraphicItem::iconPixmap() const
{
  return QPixmap();
}

void QgsModelComponentGraphicItem::updateToolTip( const QPointF &pos )
{
  const bool prevHoverStatus = mIsHovering;
  if ( itemRect().contains( pos ) )
  {
    setToolTip( mLabel );
    mIsHovering = true;
  }
  else
  {
    setToolTip( QString() );
    mIsHovering = false;
  }
  if ( mIsHovering != prevHoverStatus )
  {
    update();
    emit repaintArrows();
  }
}

void QgsModelComponentGraphicItem::fold( Qt::Edge edge, bool folded )
{
  mComponent->setLinksCollapsed( edge, folded );
  // also need to update the model's stored component

  // TODO - this is not so nice, consider moving this to model class
  if ( QgsProcessingModelChildAlgorithm *child = dynamic_cast< QgsProcessingModelChildAlgorithm * >( mComponent.get() ) )
    mModel->childAlgorithm( child->childId() ).setLinksCollapsed( edge, folded );
  else if ( QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( mComponent.get() ) )
    mModel->parameterComponent( param->parameterName() ).setLinksCollapsed( edge, folded );
  else if ( QgsProcessingModelOutput *output = dynamic_cast< QgsProcessingModelOutput * >( mComponent.get() ) )
    mModel->childAlgorithm( output->childId() ).modelOutput( output->name() ).setLinksCollapsed( edge, folded );

  prepareGeometryChange();
  emit updateArrowPaths();
  update();
}

QString QgsModelComponentGraphicItem::label() const
{
  return mLabel;
}

void QgsModelComponentGraphicItem::setLabel( const QString &label )
{
  mLabel = label;
  update();
}

QgsModelComponentGraphicItem::State QgsModelComponentGraphicItem::state() const
{
  if ( isSelected() )
    return Selected;
  else if ( mIsHovering )
    return Hover;
  else
    return Normal;
}

int QgsModelComponentGraphicItem::linkPointCount( Qt::Edge ) const
{
  return 0;
}

QString QgsModelComponentGraphicItem::linkPointText( Qt::Edge, int ) const
{
  return QString();
}

QPointF QgsModelComponentGraphicItem::linkPoint( Qt::Edge edge, int index ) const
{
  switch ( edge )
  {
    case Qt::BottomEdge:
    {
      if ( linkPointCount( Qt::BottomEdge ) )
      {
        const int pointIndex = !mComponent->linksCollapsed( Qt::BottomEdge ) ? index : -1;
        const QString text = truncatedTextForItem( linkPointText( Qt::BottomEdge, index ) );
        QFontMetricsF fm( mFont );
        const double w = fm.boundingRect( text ).width();
        const double h = fm.height() * 1.2 * ( pointIndex + 1 ) + fm.height() / 2.0;
        const double y = h + mComponent->size().height() / 2.0 + 5;
        const double x = !mComponent->linksCollapsed( Qt::BottomEdge ) ? ( -mComponent->size().width() / 2 + 33 + w + 5 ) : 10;
        return QPointF( x, y );
      }
      break;
    }

    case Qt::TopEdge:
    {
      if ( linkPointCount( Qt::TopEdge ) )
      {
        double offsetX = 25;
        int paramIndex = index;
        if ( mComponent->linksCollapsed( Qt::TopEdge ) )
        {
          paramIndex = -1;
          offsetX = 17;
        }
        QFontMetricsF fm( mFont );
        double h = -( fm.height() * 1.2 ) * ( paramIndex + 2 ) - fm.height() / 2.0 + 8;
        h = h - mComponent->size().height() / 2.0;
        return QPointF( -mComponent->size().width() / 2 + offsetX, h );
      }
      break;
    }
    case Qt::LeftEdge:
    case Qt::RightEdge:
      break;
  }

  return QPointF();
}

QPointF QgsModelComponentGraphicItem::calculateAutomaticLinkPoint( QgsModelComponentGraphicItem *other, Qt::Edge &edge ) const
{
  // find closest edge to other item
  const QgsRectangle otherRect( other->itemRect().translated( other->pos() ) );

  const QPointF leftPoint = pos() + QPointF( -mComponent->size().width() / 2.0, 0 );
  const double distLeft = otherRect.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( mComponent->size().width() / 2.0, 0 );
  const double distRight = otherRect.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -mComponent->size().height() / 2.0 );
  const double distTop = otherRect.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, mComponent->size().height() / 2.0 );
  const double distBottom = otherRect.distance( QgsPointXY( bottomPoint ) );

  if ( distLeft <= distRight && distLeft <= distTop && distLeft <= distBottom )
  {
    edge = Qt::LeftEdge;
    return leftPoint;
  }
  else if ( distRight <= distTop && distRight <= distBottom )
  {
    edge = Qt::RightEdge;
    return rightPoint;
  }
  else if ( distBottom <= distTop )
  {
    edge = Qt::BottomEdge;
    return bottomPoint;
  }
  else
  {
    edge = Qt::TopEdge;
    return topPoint;
  }
}

QPointF QgsModelComponentGraphicItem::calculateAutomaticLinkPoint( const QPointF &point, Qt::Edge &edge ) const
{
  // find closest edge to other point
  const QgsPointXY otherPt( point );
  const QPointF leftPoint = pos() + QPointF( -mComponent->size().width() / 2.0, 0 );
  const double distLeft = otherPt.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( mComponent->size().width() / 2.0, 0 );
  const double distRight = otherPt.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -mComponent->size().height() / 2.0 );
  const double distTop = otherPt.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, mComponent->size().height() / 2.0 );
  const double distBottom = otherPt.distance( QgsPointXY( bottomPoint ) );

  if ( distLeft <= distRight && distLeft <= distTop && distLeft <= distBottom )
  {
    edge = Qt::LeftEdge;
    return leftPoint;
  }
  else if ( distRight <= distTop && distRight <= distBottom )
  {
    edge = Qt::RightEdge;
    return rightPoint;
  }
  else if ( distBottom <= distTop )
  {
    edge = Qt::BottomEdge;
    return bottomPoint;
  }
  else
  {
    edge = Qt::TopEdge;
    return topPoint;
  }
}

QgsModelParameterGraphicItem::QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( parameter, model, parent )
{
  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mIconModelInput.svg" ) ) );
  QPainter painter( &mPicture );
  svg.render( &painter );
  painter.end();

  if ( const QgsProcessingParameterDefinition *paramDef = model->parameterDefinition( parameter->parameterName() ) )
    setLabel( paramDef->description() );
  else
    setLabel( QObject::tr( "Error (%1)" ).arg( parameter->parameterName() ) );
}

void QgsModelParameterGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::editComponent );
  popupmenu->exec( event->screenPos() );
}

QColor QgsModelParameterGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 238, 242, 131 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelParameterGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 116, 113, 68 );
    case Hover:
    case Normal:
      return QColor( 234, 226, 118 );
  }
  return QColor();
}

QColor QgsModelParameterGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return Qt::black;
}

QPicture QgsModelParameterGraphicItem::iconPicture() const
{
  return mPicture;
}

void QgsModelParameterGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelParameter *param = dynamic_cast< const QgsProcessingModelParameter * >( component() ) )
  {
    if ( model()->childAlgorithmsDependOnParameter( param->parameterName() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove input" ),
                            QObject::tr( "Algorithms depend on the selected input.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else if ( model()->otherParametersDependOnParameter( param->parameterName() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove input" ),
                            QObject::tr( "Other inputs depend on the selected input.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else
    {
      model()->removeModelParameter( param->parameterName() );
      emit changed();
      emit requestModelRepaint();
    }
  }
}



QgsModelChildAlgorithmGraphicItem::QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( child, model, parent )
{
  if ( !child->algorithm()->svgIconPath().isEmpty() )
  {
    QSvgRenderer svg( child->algorithm()->svgIconPath() );
    const QSizeF size = svg.defaultSize();
    QPainter painter( &mPicture );
    painter.scale( 16.0 / size.width(), 16.0 / size.width() );
    svg.render( &painter );
    painter.end();
  }
  else
  {
    mPixmap = child->algorithm()->icon().pixmap( 15, 15 );
  }

  setLabel( child->description() );
}

void QgsModelChildAlgorithmGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::editComponent );

  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( !child->isActive() )
    {
      QAction *activateAction = popupmenu->addAction( QObject::tr( "Activate" ) );
      connect( activateAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::activateAlgorithm );
    }
    else
    {
      QAction *deactivateAction = popupmenu->addAction( QObject::tr( "Deactivate" ) );
      connect( deactivateAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::deactivateAlgorithm );
    }
  }

  popupmenu->exec( event->screenPos() );
}

QColor QgsModelChildAlgorithmGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 255, 255, 255 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelChildAlgorithmGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 50, 50, 50 );
    case Hover:
    case Normal:
      return Qt::gray;
  }
  return QColor();
}

QColor QgsModelChildAlgorithmGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() )->isActive() ? Qt::black : Qt::gray;
}

QPixmap QgsModelChildAlgorithmGraphicItem::iconPixmap() const
{
  return mPixmap;
}

QPicture QgsModelChildAlgorithmGraphicItem::iconPicture() const
{
  return mPicture;
}

int QgsModelChildAlgorithmGraphicItem::linkPointCount( Qt::Edge edge ) const
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    switch ( edge )
    {
      case Qt::BottomEdge:
        return child->algorithm()->outputDefinitions().size();
      case Qt::TopEdge:
      {
        QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();
        params.erase( std::remove_if( params.begin(), params.end(), []( const QgsProcessingParameterDefinition * param )
        {
          return param->flags() & QgsProcessingParameterDefinition::FlagHidden || param->isDestination();
        } ), params.end() );
        return params.size();
      }

      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }
  return 0;
}

QString QgsModelChildAlgorithmGraphicItem::linkPointText( Qt::Edge edge, int index ) const
{
  if ( index < 0 )
    return QString();

  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    switch ( edge )
    {
      case Qt::BottomEdge:
        return truncatedTextForItem( child->algorithm()->outputDefinitions().at( index )->description() );

      case Qt::TopEdge:
      {
        QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();
        params.erase( std::remove_if( params.begin(), params.end(), []( const QgsProcessingParameterDefinition * param )
        {
          return param->flags() & QgsProcessingParameterDefinition::FlagHidden || param->isDestination();
        } ), params.end() );

        return truncatedTextForItem( params.at( index )->description() );
      }

      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }
  return QString();
}

void QgsModelChildAlgorithmGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( !model()->removeChildAlgorithm( child->childId() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove algorithm" ),
                            QObject::tr( "Other algorithms depend on the selected one.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else
    {
      emit changed();
      emit requestModelRepaint();
    }
  }
}

void QgsModelChildAlgorithmGraphicItem::deactivateAlgorithm()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    model()->deactivateChildAlgorithm( child->childId() );
    emit requestModelRepaint();
  }
}

void QgsModelChildAlgorithmGraphicItem::activateAlgorithm()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( model()->activateChildAlgorithm( child->childId() ) )
    {
      emit requestModelRepaint();
    }
    else
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not activate algorithm" ),
                            QObject::tr( "The selected algorithm depends on other currently non-active algorithms.\n"
                                         "Activate them them before trying to activate it.." ) );
    }
  }
}


QgsModelOutputGraphicItem::QgsModelOutputGraphicItem( QgsProcessingModelOutput *output, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( output, model, parent )
{
  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mIconModelOutput.svg" ) ) );
  QPainter painter( &mPicture );
  svg.render( &painter );
  painter.end();
  setLabel( output->name() );
}


QColor QgsModelOutputGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 172, 196, 114 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelOutputGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 42, 65, 42 );
    case Hover:
    case Normal:
      return QColor( 90, 140, 90 );
  }
  return QColor();
}

QColor QgsModelOutputGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return Qt::black;
}

QPicture QgsModelOutputGraphicItem::iconPicture() const
{
  return mPicture;
}

void QgsModelOutputGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelOutput *output = dynamic_cast< const QgsProcessingModelOutput * >( component() ) )
  {
    model()->childAlgorithm( output->childId() ).removeModelOutput( output->name() );
    model()->updateDestinationParameters();
    emit changed();
    emit requestModelRepaint();
  }
}


///@endcond
