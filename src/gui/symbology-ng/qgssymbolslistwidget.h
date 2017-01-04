/***************************************************************************
 qgssymbolslistwidget.h
 ---------------------
 begin                : June 2012
 copyright            : (C) 2012 by Arunmozhi
 email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLSLISTWIDGET_H
#define QGSSYMBOLSLISTWIDGET_H

#include "ui_widget_symbolslist.h"

#include "qgssymbolwidgetcontext.h"

#include <QWidget>
#include "qgis_gui.h"

class QgsSymbol;
class QgsStyle;

class QMenu;

/** \ingroup gui
 * \class QgsSymbolsListWidget
 */
class GUI_EXPORT QgsSymbolsListWidget : public QWidget, private Ui::SymbolsListWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsSymbolsListWidget( QgsSymbol* symbol, QgsStyle* style, QMenu* menu, QWidget* parent, const QgsVectorLayer * layer = nullptr );


    virtual ~QgsSymbolsListWidget();

    /** Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * @param context symbol widget context
     * @see context()
     * @note added in QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext& context );

    /** Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * @see setContext()
     * @note added in QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* layer() const { return mLayer; }

  public slots:

    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor( const QColor& color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void saveSymbol();

    void symbolAddedToStyle( const QString& name, QgsSymbol* symbol );

    void on_mSymbolUnitWidget_changed();
    void on_mTransparencySlider_valueChanged( int value );

    //! Pupulates the groups combo box with available tags and smartgroups
    void populateGroups();
    void on_groupsCombo_currentIndexChanged( int index );

    void openStyleManager();
    void clipFeaturesToggled( bool checked );

    void updateDataDefinedMarkerSize();
    void updateDataDefinedMarkerAngle();
    void updateDataDefinedLineWidth();

  signals:
    void changed();

  private:
    QgsSymbol* mSymbol;
    QgsStyle* mStyle;
    QMenu* mAdvancedMenu;
    QAction* mClipFeaturesAction;
    const QgsVectorLayer* mLayer;
    QgsMapCanvas* mMapCanvas;

    void populateSymbolView();
    void populateSymbols( const QStringList& symbols );
    void updateSymbolColor();
    void updateSymbolInfo();

    //! Displays alpha value as transparency in mTransparencyLabel
    void displayTransparency( double alpha );

    QgsSymbolWidgetContext mContext;

    QgsExpressionContext createExpressionContext() const override;
};

#endif //QGSSYMBOLSLISTWIDGET_H



