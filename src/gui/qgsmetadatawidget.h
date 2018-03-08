/***************************************************************************
                          qgsmetadatawidget.h  -  description
                             -------------------
    begin                : 17/05/2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMETADATAWIDGET_H
#define QGSMETADATAWIDGET_H

#include "QStandardItemModel"
#include "QStyledItemDelegate"

#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsmaplayer.h"
#include "qgslayermetadata.h"
#include "ui_qgsmetadatawidget.h"

/**
 * \ingroup gui
 * \class QgsMetadataWidget
 * \brief A wizard to edit metadata on a map layer.
 *
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsMetadataWidget : public QWidget, private Ui::QgsMetadataWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for the wizard.
     *
     * If \a layer is set, then this constructor automatically sets the widget's metadata() to match
     * the layer's metadata..

     * \see setMetadata()
     */
    QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer = nullptr );

    /**
     * Sets the \a metadata to display in the widget.
     *
     * This method can be called after constructing a QgsMetadataWidget in order
     * to set the displayed metadata to custom, non-layer based metadata.
     *
     * \see metadata()
     */
    void setMetadata( const QgsLayerMetadata &metadata );

    /**
     * Returns a QgsLayerMetadata object representing the current state of the widget.
     * \see saveMetadata()
     */
    QgsLayerMetadata metadata();

    /**
     * Save all fields in a QgsLayerMetadata object.
     * \see metadata()
     * \see acceptMetadata()
     * \see checkMetadata()
     */
    void saveMetadata( QgsLayerMetadata &layerMetadata );

    /**
     * Check if values in the wizard are correct.
     * \see saveMetadata()
     */
    bool checkMetadata();

    /**
     * If the CRS is updated.
     */
    void crsChanged();

    /**
     * Saves the metadata to the layer.
     */
    void acceptMetadata();

    /**
     * Returns a list of languages available by default in the wizard.
     */
    static QMap<QString, QString> parseLanguages();

    /**
     * Returns a list of licences available by default in the wizard.
     */
    static QStringList parseLicenses();

    /**
     * Returns a list of link types available by default in the wizard.
     * \see https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
     */
    static QStringList parseLinkTypes();

    /**
     * Returns a list of MIME types available by default in the wizard.
     * \see https://fr.wikipedia.org/wiki/Type_MIME
     */
    static QStringList parseMimeTypes();

    /**
     * Returns a list of types available by default in the wizard.
     */
    static QMap<QString, QString> parseTypes();

    /**
     * Sets a map \a canvas associated with the widget.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  private slots:
    void removeSelectedCategories();
    void updatePanel();
    void fillSourceFromLayer();
    void fillCrsFromLayer();
    void fillCrsFromProvider();
    void addDefaultCategories();
    void addNewCategory();
    void addVocabulary();
    void removeSelectedVocabulary();
    void addLicence();
    void removeSelectedLicence();
    void addRight();
    void removeSelectedRight();
    void addConstraint();
    void removeSelectedConstraint();
    void addAddress();
    void removeSelectedAddress();
    void addLink();
    void removeSelectedLink();
    void addHistory();
    void removeSelectedHistory();

  private:

    void fillComboBox();
    void setPropertiesFromLayer();
    void syncFromCategoriesTabToKeywordsTab();
    QStringList mDefaultCategories;
    QgsMapLayer *mLayer = nullptr;
    QgsCoordinateReferenceSystem mCrs;
    QgsLayerMetadata mMetadata;
    QStandardItemModel *mConstraintsModel = nullptr;
    QStandardItemModel *mLinksModel = nullptr;
    QStringListModel *mCategoriesModel = nullptr;
    QStringListModel *mDefaultCategoriesModel = nullptr;
    QStringListModel *mRightsModel = nullptr;
    QStringListModel *mHistoryModel = nullptr;
};

#ifndef SIP_RUN


/**
 \\\@cond PRIVATE
 * \ingroup gui
 * \class LinkItemDelegate
 * \brief Special delegate for the link view in the metadata wizard.
 *
 * \since QGIS 3.0
 */
class LinkItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:

    /**
     * \brief LinkItemDelegate constructor
     * \param parent
     */
    explicit LinkItemDelegate( QObject *parent = nullptr );

    /**
     * Create a special editor with a QCombobox in the link view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 * \ingroup gui
 * \class ConstraintItemDelegate
 * \brief Special delegate for the constraint view in the metadata wizard.
 *
 * \since QGIS 3.0
 */
class ConstraintItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:

    /**
     * \brief ConstraintItemDelegate constructor
     * \param parent
     */
    explicit ConstraintItemDelegate( QObject *parent = nullptr );

    /**
     * Create a special editor with a QCombobox in the constraint view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 \\\@endcond
 */

#endif
#endif
