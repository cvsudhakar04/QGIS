/***************************************************************************
                             qgsprocessingoutputdestinationwidget.h
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

#ifndef QGSPROCESSINGOUTPUTDESTINATIONWIDGET_H
#define QGSPROCESSINGOUTPUTDESTINATIONWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingdestinationwidgetbase.h"
#include "qgsprocessingwidgetwrapper.h"
#include <QWidget>

class QgsProcessingDestinationParameter;
class QgsBrowserGuiModel;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A widget which allows users to select the destination path for an output style Processing parameter.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingLayerOutputDestinationWidget : public QWidget, private Ui::QgsProcessingDestinationWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingLayerOutputDestinationWidget, associated with the specified \a parameter.
     */
    QgsProcessingLayerOutputDestinationWidget( const QgsProcessingDestinationParameter *parameter, bool defaultSelection, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns TRUE if the output is set to be skipped.
     */
    bool outputIsSkipped() const;

    /**
     * Sets the \a value to show in the widget.
     */
    void setValue( const QVariant &value );

    /**
     * Returns the widgets current value.
     */
    QVariant value() const;

    /**
     * Sets the \a context in which the widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

  signals:

    /**
     * Emitted whenever the "skip output" option is toggled in the widget.
     */
    void skipOutputChanged( bool skipped );

    /**
     * Emitted whenever the destination value is changed in the widget.
     */
    void destinationChanged();
  protected:

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:

    void menuAboutToShow();
    void skipOutput();
    void saveToTemporary();
    void selectDirectory();
    void selectFile();
    void saveToGeopackage();
    void saveToDatabase();
    void selectEncoding();
    void textChanged( const QString &text );

  private:

    QString mimeDataToPath( const QMimeData *data );

    const QgsProcessingDestinationParameter *mParameter = nullptr;
    QMenu *mMenu = nullptr;

    bool mUseTemporary = true;
    bool mDefaultSelection = false;
    QString mEncoding;
    QgsBrowserGuiModel *mBrowserModel = nullptr;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGOUTPUTDESTINATIONWIDGET_H
