# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleInputDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import range
from builtins import basestring

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import QgsSettings
from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtCore import QByteArray
from qgis.PyQt.QtWidgets import QDialog, QAbstractItemView, QPushButton, QDialogButtonBox
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgMultipleSelection.ui'))


class MultipleInputDialog(BASE, WIDGET):

    def __init__(self, options, selectedoptions=None):
        super(MultipleInputDialog, self).__init__(None)
        self.setupUi(self)

        self.lstLayers.setSelectionMode(QAbstractItemView.NoSelection)

        self.options = []
        for i, option in enumerate(options):
            if option is None or isinstance(option, basestring):
                self.options.append((i, option))
            else:
                self.options.append((option[0], option[1]))

        self.selectedoptions = selectedoptions or []

        # Additional buttons
        self.btnSelectAll = QPushButton(self.tr('Select all'))
        self.buttonBox.addButton(self.btnSelectAll,
                                 QDialogButtonBox.ActionRole)
        self.btnClearSelection = QPushButton(self.tr('Clear selection'))
        self.buttonBox.addButton(self.btnClearSelection,
                                 QDialogButtonBox.ActionRole)
        self.btnToggleSelection = QPushButton(self.tr('Toggle selection'))
        self.buttonBox.addButton(self.btnToggleSelection,
                                 QDialogButtonBox.ActionRole)

        self.btnSelectAll.clicked.connect(lambda: self.selectAll(True))
        self.btnClearSelection.clicked.connect(lambda: self.selectAll(False))
        self.btnToggleSelection.clicked.connect(self.toggleSelection)

        self.settings = QgsSettings()
        self.restoreGeometry(self.settings.value("/Processing/multipleInputDialogGeometry", QByteArray()))

        self.populateList()
        self.finished.connect(self.saveWindowGeometry)

    def saveWindowGeometry(self):
        self.settings.setValue("/Processing/multipleInputDialogGeometry", self.saveGeometry())

    def populateList(self):
        model = QStandardItemModel()
        for value, text in self.options:
            item = QStandardItem(text)
            item.setData(value, Qt.UserRole)
            item.setCheckState(Qt.Checked if value in self.selectedoptions else Qt.Unchecked)
            item.setCheckable(True)
            model.appendRow(item)

        self.lstLayers.setModel(model)

    def accept(self):
        self.selectedoptions = []
        model = self.lstLayers.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            if item.checkState() == Qt.Checked:
                self.selectedoptions.append(item.data(Qt.UserRole))
        QDialog.accept(self)

    def reject(self):
        self.selectedoptions = None
        QDialog.reject(self)

    def selectAll(self, value):
        model = self.lstLayers.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            item.setCheckState(Qt.Checked if value else Qt.Unchecked)

    def toggleSelection(self):
        model = self.lstLayers.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            checked = item.checkState() == Qt.Checked
            item.setCheckState(Qt.Unchecked if checked else Qt.Checked)
