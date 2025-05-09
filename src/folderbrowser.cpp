/***************************************************************************
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "folderbrowser.h"

#include "global.h"
#include "modelviewhelpers.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructs a folder browser widget.
     *
     * \param parent Parent of the widget.
     */
    FolderBrowser::FolderBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Create the toolbar
        QToolBar *toolbar = new QToolBar(this);

        QToolButton *buttonUp = new QToolButton(this);
        buttonUp->setIcon(Caneda::icon("go-up"));
        buttonUp->setShortcut(Qt::ALT + Qt::Key_Up);
        buttonUp->setStatusTip(tr("Go up one folder"));
        buttonUp->setToolTip(tr("Go up one folder"));
        buttonUp->setWhatsThis(tr("Go up one folder"));

        buttonBack = new QToolButton(this);
        buttonBack->setIcon(Caneda::icon("go-previous"));
        buttonBack->setShortcut(Qt::ALT + Qt::Key_Left);
        buttonBack->setStatusTip(tr("Go previous folder"));
        buttonBack->setToolTip(tr("Go previous folder"));
        buttonBack->setWhatsThis(tr("Go previous folder"));
        buttonBack->setEnabled(false);

        buttonForward = new QToolButton(this);
        buttonForward->setIcon(Caneda::icon("go-next"));
        buttonForward->setShortcut(Qt::ALT + Qt::Key_Right);
        buttonForward->setStatusTip(tr("Go next folder"));
        buttonForward->setToolTip(tr("Go next folder"));
        buttonForward->setWhatsThis(tr("Go next folder"));
        buttonForward->setEnabled(false);

        QToolButton *buttonHome = new QToolButton(this);
        buttonHome->setIcon(Caneda::icon("go-home"));
        buttonHome->setShortcut(Qt::ALT + Qt::Key_Home);
        buttonHome->setStatusTip(tr("Go to the home folder"));
        buttonHome->setToolTip(tr("Go to the home folder"));
        buttonHome->setWhatsThis(tr("Go to the home folder"));

        QToolButton *buttonNewFolder = new QToolButton(this);
        buttonNewFolder->setIcon(Caneda::icon("folder-new"));
        buttonNewFolder->setStatusTip(tr("Create new folder"));
        buttonNewFolder->setToolTip(tr("Create new folder"));
        buttonNewFolder->setWhatsThis(tr("Create new folder"));

        QToolButton *buttonDeleteFile = new QToolButton(this);
        buttonDeleteFile->setIcon(Caneda::icon("archive-remove"));
        buttonDeleteFile->setStatusTip(tr("Delete file/folder"));
        buttonDeleteFile->setToolTip(tr("Delete file/folder"));
        buttonDeleteFile->setWhatsThis(tr("Delete file/folder"));

        connect(buttonUp,         &QToolButton::clicked, this, &FolderBrowser::slotUpFolder);
        connect(buttonBack,       &QToolButton::clicked, this, &FolderBrowser::slotBackFolder);
        connect(buttonForward,    &QToolButton::clicked, this, &FolderBrowser::slotForwardFolder);
        connect(buttonHome,       &QToolButton::clicked, this, &FolderBrowser::slotHomeFolder);
        connect(buttonNewFolder,  &QToolButton::clicked, this, &FolderBrowser::slotNewFolder);
        connect(buttonDeleteFile, &QToolButton::clicked, this, &FolderBrowser::slotDeleteFile);

        toolbar->addWidget(buttonUp);
        toolbar->addWidget(buttonBack);
        toolbar->addWidget(buttonForward);
        toolbar->addWidget(buttonHome);
        toolbar->addWidget(buttonNewFolder);
        toolbar->addWidget(buttonDeleteFile);
        layout->addWidget(toolbar);

        // Create a new filesystem model
        m_model = new QFileSystemModel(this);
        m_model->setIconProvider(new IconProvider());
        m_model->setRootPath(QDir::rootPath());

        // Create a list view and set its properties
        m_listView = new QListView(this);
        m_listView->setModel(m_model);
        layout->addWidget(m_listView);

        connect(m_listView, &QListView::activated, this, &FolderBrowser::slotOnDoubleClicked);

        setWindowTitle(tr("Folder Browser"));
    }

    //! \brief Set the current folder to \a path.
    void FolderBrowser::setCurrentFolder(const QString& path)
    {
        m_listView->setRootIndex(m_model->index(path));

        previousPages.clear();
        nextPages.clear();

        buttonBack->setEnabled(false);
        buttonForward->setEnabled(false);
    }

    //! \brief Open the selected item.
    void FolderBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        if(m_model->isDir(index)) {
            previousPages << m_listView->rootIndex();
            m_listView->setRootIndex(index);
            nextPages.clear();

            buttonBack->setEnabled(true);
            buttonForward->setEnabled(false);
        }
        else {
            emit itemDoubleClicked(m_model->fileInfo(index).absoluteFilePath());
        }
    }

    //! \brief Go up one folder in the filesystem.
    void FolderBrowser::slotUpFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_listView->rootIndex().parent());
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Go the the previous folder.
    void FolderBrowser::slotBackFolder()
    {
        if(!previousPages.isEmpty()) {
            nextPages << m_listView->rootIndex();
            m_listView->setRootIndex(previousPages.last());
            previousPages.removeLast();

            buttonForward->setEnabled(true);
            if(previousPages.isEmpty()) {
                buttonBack->setEnabled(false);
            }
        }
    }

    //! \brief Go the the next folder.
    void FolderBrowser::slotForwardFolder()
    {
        if(!nextPages.isEmpty()) {
            previousPages << m_listView->rootIndex();
            m_listView->setRootIndex(nextPages.last());
            nextPages.removeLast();

            buttonBack->setEnabled(true);
            if(nextPages.isEmpty()) {
                buttonForward->setEnabled(false);
            }
        }
    }

    //! \brief Go the the home folder.
    void FolderBrowser::slotHomeFolder()
    {
        previousPages << m_listView->rootIndex();
        m_listView->setRootIndex(m_model->index(QDir::homePath()));
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Create a new folder.
    void FolderBrowser::slotNewFolder()
    {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             tr("New Folder"),
                                             tr("Please enter new folder name:"),
                                             QLineEdit::Normal,
                                             QString(),
                                             &ok);

        if(ok && !text.isEmpty()) {
            m_model->mkdir(m_listView->rootIndex(), text);
        }
    }

    //! \brief Delete currently selected folder/file.
    void FolderBrowser::slotDeleteFile()
    {
        int ret = QMessageBox::critical(this, tr("Delete File/Folder"),
                tr("You're about to delete one file/folder. This action can't be undone.\n"
                    "Do you want to continue?"),
                QMessageBox::Ok | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Ok:
                if(m_model->isDir(m_listView->currentIndex())) {
                    bool result = m_model->rmdir(m_listView->currentIndex());
                    if(!result) {
                        QMessageBox::warning(this, tr("Delete File/Folder"),
                                tr("Folder not empty. Skipping."),
                                QMessageBox::Ok);
                    }
                }
                else {
                    m_model->remove(m_listView->currentIndex());
                }
                break;

            case QMessageBox::Cancel:
                break;

            default:
                break;
        }
    }

} // namespace Caneda
