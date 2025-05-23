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

#ifndef FOLDERBROWSER_H
#define FOLDERBROWSER_H

#include <QWidget>

// Forward declarations
class QFileSystemModel;
class QListView;
class QToolButton;

namespace Caneda
{
    /*!
     * \brief This class implements a simple folder browser widget to be used
     * as a toolbar for easy access to the file system.
     *
     * This class handles user interaction to allow direct opening of files, as
     * well as basic file operations (as deletion). This also handles the mouse
     * and keyboad events, and sends, when appropiate, the file names to be
     * opened by the parent.
     */
    class FolderBrowser : public QWidget
    {
        Q_OBJECT

    public:
        explicit FolderBrowser(QWidget *parent = nullptr);

        void setCurrentFolder(const QString& path);

    signals:
        void itemDoubleClicked(const QString& filename);

    private Q_SLOTS:
        void slotOnDoubleClicked(const QModelIndex& index);

        void slotUpFolder();
        void slotBackFolder();
        void slotForwardFolder();
        void slotHomeFolder();
        void slotNewFolder();
        void slotDeleteFile();

    private:
        QFileSystemModel *m_model;
        QListView *m_listView;

        QList<QModelIndex> previousPages;
        QList<QModelIndex> nextPages;

        QToolButton *buttonBack, *buttonForward;
    };

} // namespace Caneda

#endif //FOLDERBROWSER_H
