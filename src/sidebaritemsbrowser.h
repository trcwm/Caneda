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

#ifndef SIDEBAR_ITEMS_BROWSER_H
#define SIDEBAR_ITEMS_BROWSER_H

#include <QPair>
#include <QStandardItemModel>
#include <QWidget>

// Forward declaration
class QLineEdit;
class QPixmap;
class QTreeView;

namespace Caneda
{
    // Forward declarations.
    class FilterProxyModel;

    /*!
     * \brief Model to provide the abstract interface for library tree items.
     *
     * This class derives from QStandardItemModel and provides the abstract
     * interface for library tree items. While the SidebarItemsBrowser class
     * implements the user interface, this class interacts with the data
     * itself.
     *
     * This class defines a standard interface that must used to be able to
     * interoperate with other components in Qt's model/view framework. The
     * underlying data model is exposed as a simple tree of rows and columns.
     * Each item has a unique index specified by a QModelIndex.
     *
     * \sa QStandardItemModel, SidebarItemsBrowser
     */
    class SidebarItemsModel : public QStandardItemModel
    {
        Q_OBJECT

    public:
        explicit SidebarItemsModel(QObject *parent = nullptr);

        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);
        void plugLibrary(QString libraryName, QString category);
        void unPlugLibrary(QString libraryName, QString category);
    };

    /*!
     * \brief This class implements the sidebar dockwidget with components to
     * be inserted in graphic documents.
     *
     * This class implements the sidebar dockwidget corresponding to the
     * SchematicContext, and SymbolContext classes. It allows previously
     * generated components to be inserted in these type of documents.
     *
     * The components depend on the final context class. In the
     * SchematicContext, for example, components are electronic components; and
     * in the SymbolContext, components are painting items. All these
     * components are inserted into the currently opened document upon user
     * double click.
     *
     * \sa SchematicContext, SymbolContext, SidebarTextBrowser
     * \sa QStandardItemModel
     */
    class SidebarItemsBrowser : public QWidget
    {
        Q_OBJECT

    public:
        explicit SidebarItemsBrowser(QStandardItemModel *model, QWidget *parent = nullptr);
        ~SidebarItemsBrowser() override;

    Q_SIGNALS:
        void itemClicked(const QString& item, const QString& category);
        void itemDoubleClicked(const QString& item, const QString& category);

    protected:
        bool eventFilter(QObject *object, QEvent *event) override;

    private Q_SLOTS:
        void filterTextChanged();

        void itemClicked(const QModelIndex& index);

    private:
        QStandardItemModel *m_model;
        FilterProxyModel *m_proxyModel;
        QTreeView *m_treeView;

        QLineEdit *m_filterEdit;
    };

} // namespace Caneda

#endif //SIDEBAR_ITEMS_BROWSER_H
