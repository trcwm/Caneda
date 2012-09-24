/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "propertydialog.h"

#include "cgraphicsscene.h"
#include "documentviewmanager.h"
#include "global.h"
#include "idocument.h"
#include "undocommands.h"

#include <QComboBox>
#include <QSortFilterProxyModel>

namespace Caneda
{
    //*************************************************************
    //******************** PropertyModel **************************
    //*************************************************************
    /*!
     * \brief Constructor
     *
     * \param map PropertyMap wich contains all properties to be modified.
     * \param parent This object's parent.
     *
     * \sa PropertyMap
     */
    PropertyModel::PropertyModel(PropertyMap map, QObject *parent) :
        QAbstractTableModel(parent),
        propMap(map),
        keys(map.keys())
    {
    }

    /*!
     * \brief Returns the data stored for the item referred by index.
     *
     * This class returns the item data corresponding to index position.
     * For example, if we are editing an item in the first column, the
     * data corresponds to the property name, hence the return value is
     * the property name in the form of a QString.
     *
     * \param index Item to return data from
     * \param role Role of the item (editable, checkable, etc).
     * \return data stored for given item
     */
    QVariant PropertyModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= propMap.size()) {
            return QVariant();
        }

        QString key = keys.at(index.row());

        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            switch(index.column()) {
                case 0: return key;
                case 1: return propMap[key].value();
                case 3: return propMap[key].description();
                default: ;
            }
        }
        else if(role == Qt::CheckStateRole && index.column() == 2) {
            return propMap[key].isVisible() ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    /*!
     * \brief Returns header data (text) for the given column
     *
     * This method defines column header text to be displayed on the
     * associated table view.
     */
    QVariant PropertyModel::headerData(int section, Qt::Orientation o, int role) const
    {
        if(role != Qt::DisplayRole) {
            return QVariant();
        }

        if(o == Qt::Vertical) {
            return QAbstractTableModel::headerData(section, o, role);
        }
        else {
            switch(section) {
                case 0: return tr("Name");
                case 1: return tr("Value");
                case 2: return tr("Visible");
                case 3: return tr("Description");
            }
        }
        return QVariant();
    }

    /*!
     * \brief Returns item flags according to its position. This flags
     * are responsible for the item editable or checkable state.
     *
     * \param index Item for which its flags must be returned.
     * \return Qt::ItemFlags Item's flags.
     */
    Qt::ItemFlags PropertyModel::flags(const QModelIndex& index) const
    {
        if(!index.isValid()) {
            return Qt::ItemIsEnabled;
        }

        Qt::ItemFlags flags = QAbstractTableModel::flags(index);

        // Column 2 is checkable (visibility CheckBox)
        if(index.column() == 2) {
            flags |= Qt::ItemIsUserCheckable;
        }
        // Every other column is text editable
        else {
            flags |= Qt::ItemIsEditable;
        }

        return flags;
    }

    /*!
     * \brief Sets data in a PropertyModel item.
     *
     * Sets data in a PropertyModel item. Some special data should not be editable,
     * for example property label should not change its property "name" (although
     * its property "value" can be edited). This method takes care of that.
     *
     * In the future, we can set a new field inside properties to determine if
     * some parts of a property cannot be modified. For example default property
     * "names" in the PropertyMap of a component should not be modifiable as they
     * are fixed by the spice model, nevertheless new user defined properties should
     * be modifiable.
     *
     * \param index Item to be edited.
     * \param value New value to be set.
     * \param role Role of the item. Helps identify what are we editing (editable
     * item, checkable item, etc).
     * \return True on success, false otherwise.
     *
     * \sa PropertyMap
     */
    bool PropertyModel::setData(const QModelIndex& index, const QVariant& value,
            int role)
    {
        if(index.isValid()){
            // If editing the property name, create a new property
            // and replace old one.
            if(role == Qt::EditRole && index.column() == 0) {

                // We should not overwrite an existing property
                if (keys.contains(value.toString())){
                    return false;
                }

                // Property "label" should not change name
                if (keys[index.row()] == "label") {
                    return false;
                }

                Property newProp(value.toString(),
                                 propMap[keys[index.row()]].value(),
                                 propMap[keys[index.row()]].description(),
                                 propMap[keys[index.row()]].isVisible());
                propMap.remove(keys[index.row()]);
                propMap.insert(value.toString(), newProp);
                keys.replace(index.row(), value.toString());

            }
            // If editing the property value, replace old value
            if(role == Qt::EditRole && index.column() == 1) {
                propMap[keys[index.row()]].setValue(value.toString());
            }
            // If editing the property visibility, set new visibility
            else if(role == Qt::CheckStateRole && index.column() == 2) {
                Property &prop = propMap[keys[index.row()]];
                prop.setVisible(!prop.isVisible());
            }
            // If editing the property description, replace old value
            if(role == Qt::EditRole && index.column() == 3) {
                propMap[keys[index.row()]].setDescription(value.toString());
            }

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    /*!
     * \brief Inserts a new row (Property) with a default text.
     *
     * \param position Position to insert new property.
     * \param rows Number of rows to insert.
     * \param index Unused.
     * \return True on success, false otherwise.
     */
    bool PropertyModel::insertRows(int position, int rows, const QModelIndex &index)
    {
        // Insert new property
        Q_UNUSED(index);
        beginInsertRows(QModelIndex(), position, position+rows-1);

        for (int row=0; row < rows; row++) {
            // Find first name available
            QString nameBase = tr("Property") + " ";
            int i = 1;
            QString propertyName = nameBase + QString::number(i);

            while(keys.contains(propertyName)) {
                i++;
                propertyName = nameBase + QString::number(i);
            }

            keys.insert(position, propertyName);
            Property newProp(propertyName, "Value", tr("User created property"), true);
            propMap.insert(propertyName, newProp);
        }

        endInsertRows();
        return true;
    }

    /*!
     * \brief Deletes a row (Property) from the PropertyMap.
     *
     * \param position Position from where to delete rows.
     * \param rows Number of rows to delete.
     * \param index Unused.
     * \return True on success, false otherwise.
     */
    bool PropertyModel::removeRows(int position, int rows, const QModelIndex &index)
    {
        Q_UNUSED(index);
        beginRemoveRows(QModelIndex(), position, position+rows-1);

        for (int row=0; row < rows; ++row) {
            // Remove property
            propMap.remove(keys[position]);
            keys.removeAt(position);
        }

        endRemoveRows();
        return true;
    }

    //*************************************************************
    //***************** PropertyValueDelegate *********************
    //*************************************************************
    PropertyValueDelegate::PropertyValueDelegate(QObject *parent) : QItemDelegate(parent)
    {
    }

    QWidget *PropertyValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                 const QModelIndex& index) const
    {
        QStringList valOptions;
        valOptions = index.model()->data(index, PropertyModel::OptionsRole).toStringList();

        if(index.column() != 1 || valOptions.isEmpty()) {
            return QItemDelegate::createEditor(parent, option, index);
        }

        QComboBox *editor = new QComboBox(parent);
        editor->addItems(valOptions);
        return editor;
    }

    void PropertyValueDelegate::setEditorData(QWidget *editor, const QModelIndex& index) const
    {
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        if(!comboBox) {
            QItemDelegate::setEditorData(editor, index);
            return;
        }

        QString value = index.model()->data(index, Qt::DisplayRole).toString();
        QStringList options = index.model()->data(index, PropertyModel::OptionsRole).toStringList();
        int comboIndex = options.indexOf(value);

        comboBox->setCurrentIndex(comboIndex);
    }

    void PropertyValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                             const QModelIndex& index) const
    {
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        if(!comboBox) {
            QItemDelegate::setModelData(editor, model, index);
            return;
        }
        model->setData(index, comboBox->currentText());
    }

    //*************************************************************
    //******************** PropertyDialog *************************
    //*************************************************************
    /*!
     * \brief Constructor
     *
     * \param propGroup The PropertyGroup being modified by this dialog.
     */
    PropertyDialog::PropertyDialog(PropertyGroup *propGroup, QWidget *parent) :
        QDialog(parent), m_propertyGroup(propGroup)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        // Set button properties
        ui.m_clearButton->setIcon(Caneda::icon("edit-clear-locationbar-rtl"));
        ui.m_clearButton->setShortcut(Qt::ALT + Qt::Key_C);
        ui.m_clearButton->setStatusTip(tr("Clear the filter text"));
        ui.m_clearButton->setWhatsThis(
                    tr("Clear Filter Text\n\nClears the filter text thus reshowing all properties"));

        ui.m_addButton->setIcon(Caneda::icon("list-add"));
        ui.m_addButton->setStatusTip(tr("Add a new property to the list"));
        ui.m_addButton->setWhatsThis(
                    tr("Add New Property\n\nAdds a new property to the list"));

        ui.m_removeButton->setIcon(Caneda::icon("list-remove"));
        ui.m_removeButton->setStatusTip(tr("Remove selected property from the list"));
        ui.m_removeButton->setWhatsThis(
                    tr("Remove Property\n\nRemoves selected property from the list"));

        // Create new table model
        m_model = new PropertyModel(m_propertyGroup->propertyMap(), this);

        // Create proxy model and set its properties
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        // Apply table properties and set proxy model
        ui.tableView->setModel(m_proxyModel);
        ui.tableView->setItemDelegate(new PropertyValueDelegate(this));
        ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui.tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
        ui.tableView->resizeColumnsToContents();

        connect(ui.m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));
        connect(ui.m_clearButton, SIGNAL(clicked()), ui.m_filterEdit, SLOT(clear()));
        connect(ui.m_addButton, SIGNAL(clicked()), SLOT(addProperty()));
        connect(ui.m_removeButton, SIGNAL(clicked()), SLOT(removeProperty()));
    }

    /*!
     * \brief Accept dialog
     *
     * Accept dialog and set new property's values according to
     * the user input.
     */
    void PropertyDialog::accept()
    {
        PropertyMapCmd *cmd = new PropertyMapCmd(m_propertyGroup, m_propertyGroup->propertyMap(),
                m_model->propMap);

        DocumentViewManager *manager = DocumentViewManager::instance();
        manager->currentDocument()->undoStack()->push(cmd);

        QDialog::accept();
    }

    //! \brief Filters properties according to user input on a QLineEdit.
    void PropertyDialog::filterTextChanged()
    {
        QString text = ui.m_filterEdit->text();
        ui.m_clearButton->setEnabled(!text.isEmpty());
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    //! \brief Add a new property to the model.
    void PropertyDialog::addProperty()
    {
        m_model->insertRows(m_model->rowCount(), 1);
        ui.tableView->resizeColumnsToContents();
        ui.tableView->horizontalHeader()->setStretchLastSection(true);
    }

    //! \brief Remove a property from the model.
    void PropertyDialog::removeProperty()
    {
        QItemSelectionModel *selectionModel = ui.tableView->selectionModel();
        QModelIndexList indexes = selectionModel->selectedRows();
        QModelIndex index;

        foreach (index, indexes) {
            int row = m_proxyModel->mapToSource(index).row();
            m_model->removeRows(row, 1);
        }

        ui.tableView->resizeColumnsToContents();
        ui.tableView->horizontalHeader()->setStretchLastSection(true);
    }

} // namespace Caneda
