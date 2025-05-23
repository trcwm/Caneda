/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#include "quicklauncher.h"

#include "actionmanager.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

namespace Caneda
{
    //*************************************************************
    //******************* QuickLauncherModel **********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param actions List of available application actions.
     * \param parent Parent of this object.
     */
    QuickLauncherModel::QuickLauncherModel(QList<QAction *> actions, QObject *parent) :
        QAbstractTableModel(parent),
        m_actions(actions)
    {
    }

    /*!
     * \brief Returns the data stored for the item referred by index.
     *
     * This class returns the item data corresponding to index position.
     * For example, if we are editing an item in the first column, the
     * data corresponds to the action name, hence the return value is
     * the object name of the action in the form of a QString.
     *
     * \param index Item to return data from
     * \param role Role of the item (editable, checkable, etc).
     * \return data stored for given item
     */
    QVariant QuickLauncherModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= m_actions.size()) {
            return QVariant();
        }

        if(role == Qt::DisplayRole) {
            return m_actions.at(index.row())->text().remove(QChar('&'));
        }

        if(role == Qt::DecorationRole) {
            return QVariant(m_actions.at(index.row())->icon());
        }

        if(role == Qt::SizeHintRole) {
            return QSize(150, 32);
        }

        return QVariant();
    }

    /*!
     * \brief Returns item flags according to its position. These flags
     * are responsible for the item editable or checkable state.
     *
     * \param index Item for which its flags must be returned.
     * \return Qt::ItemFlags Item's flags.
     */
    Qt::ItemFlags QuickLauncherModel::flags(const QModelIndex& index) const
    {
        if(!index.isValid()) {
            return Qt::ItemIsEnabled;
        }

        Qt::ItemFlags flags = QAbstractTableModel::flags(index);

        if(index.column() == 0) {
            flags |= Qt::ItemIsEnabled;
        }

        return flags;
    }


    //*************************************************************
    //********************* QuickLauncher *************************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object.
     */
    QuickLauncher::QuickLauncher(QWidget *parent) : QMenu(parent)
    {
        // Set window geometry
        setMinimumSize(300,300);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Set lineEdit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        m_filterEdit->installEventFilter(this);
        layout->addWidget(m_filterEdit);

        // Get the list of actions
        ActionManager *am = ActionManager::instance();
        m_actions = am->actions();

        // Create a new table model
        m_model = new QuickLauncherModel(m_actions, this);

        // Create proxy model and set its properties.
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->sort(0);

        // Create table view, set properties and proxy model
        m_listView = new QListView(this);
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_listView->setModel(m_proxyModel);
        layout->addWidget(m_listView);

        // Signals and slots connections
        connect(m_filterEdit, &QLineEdit::textChanged,   this, &QuickLauncher::filterTextChanged);
        connect(m_filterEdit, &QLineEdit::returnPressed, this, &QuickLauncher::triggerAction);
        connect(m_listView,   &QListView::activated,     this, &QuickLauncher::triggerAction);

        // Start with the focus on the filter
        m_filterEdit->setFocus();
    }

    //! \brief Filter event to select the listView on down arrow key event
    bool QuickLauncher::eventFilter(QObject *object, QEvent *event)
    {
        if(object == m_filterEdit) {
            if(event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if(keyEvent->key() == Qt::Key_Down) {
                    // Set the row next to the currently selected one
                    int index = m_listView->currentIndex().row() + 1;
                    m_listView->setCurrentIndex(m_proxyModel->index(index,0));
                    m_listView->setFocus();

                    return true;
                }
            }

            return false;
        }

        return QMenu::eventFilter(object, event);
    }

    //! \brief Filters actions according to user input on a QLineEdit.
    void QuickLauncher::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
        m_listView->setCurrentIndex(m_proxyModel->index(0,0));
    }

    //! \brief Accept the dialog and run the selected action.
    void QuickLauncher::triggerAction()
    {
        if(m_listView->currentIndex().isValid()) {
            int currentIndex = m_proxyModel->mapToSource(m_listView->currentIndex()).row();
            m_actions.at(currentIndex)->trigger();
        }

        hide();
    }

} // namespace Caneda
