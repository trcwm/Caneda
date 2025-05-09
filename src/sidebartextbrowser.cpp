/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "sidebartextbrowser.h"

#include "documentviewmanager.h"
#include "idocument.h"
#include "modelviewhelpers.h"
#include "settings.h"

#include <QFileSystemModel>
#include <QDebug>
#include <QLineEdit>
#include <QTextCodec>
#include <QVBoxLayout>

namespace Caneda
{
    //! \brief Constructor.
    SidebarTextBrowser::SidebarTextBrowser(QWidget *parent) : QWidget(parent)
    {
        Settings *settings = Settings::instance();

        // Load library database settings
        QString libpath = settings->currentValue("libraries/hdl").toString();
        if(QFileInfo(libpath).exists() == false) {
            qDebug() << "Error loading text libraries";
            return;
        }

        // Fill the treeview and proxy models
        QVBoxLayout *layout = new QVBoxLayout(this);

        m_filterEdit = new QLineEdit();
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        layout->addWidget(m_filterEdit);

        m_fileModel = new QFileSystemModel;
        m_fileModel->setIconProvider(new IconProvider());
        QModelIndex rootModelIndex = m_fileModel->setRootPath(libpath);

        m_proxyModel = new FileFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_fileModel);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        m_treeView = new QTreeView;
        m_treeView->setModel(m_proxyModel);
        m_treeView->setRootIndex(m_proxyModel->mapFromSource(rootModelIndex));

        m_treeView->setHeaderHidden(true);
        m_treeView->setColumnHidden(1, 1);
        m_treeView->setColumnHidden(2, 1);
        m_treeView->setColumnHidden(3, 1);
        m_treeView->setAnimated(true);
        m_treeView->setAlternatingRowColors(true);

        layout->addWidget(m_treeView);

        connect(m_filterEdit, &QLineEdit::textChanged, this, &SidebarTextBrowser::filterTextChanged);
        connect(m_fileModel,  &QFileSystemModel::modelReset, m_treeView, &QTreeView::expandAll);
        connect(m_treeView,   &QTreeView::activated,   this, &SidebarTextBrowser::slotOnDoubleClicked);

        setWindowTitle(tr("Text Templates"));
    }

    //! \brief Destructor.
    SidebarTextBrowser::~SidebarTextBrowser()
    {
        m_treeView->setModel(nullptr);
    }

    void SidebarTextBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);

        if(!text.isEmpty()) {
            m_treeView->expandAll();
        }
        else {
            m_treeView->collapseAll();
        }
    }

    void SidebarTextBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

        if(m_fileModel->isDir(sourceIndex)) {
            return;
        }

        // It is a file so we paste the template
        QFile file(m_fileModel->fileInfo(sourceIndex).absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << tr("Could not open file for reading");
            return;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString content = stream.readAll();
        file.close();

        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        TextDocument *textDoc = qobject_cast<TextDocument*>(doc);

        if (textDoc) {
            textDoc->pasteTemplate(content);
        }
    }

} // namespace Caneda
