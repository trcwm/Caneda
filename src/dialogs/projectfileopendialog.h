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

#ifndef PROJECT_FILE_OPEN_DIALOG_H
#define PROJECT_FILE_OPEN_DIALOG_H

#include "ui_projectfileopendialog.h"

#include <QDialog>

namespace Caneda
{
    // Forward declarations
    class SidebarItemsBrowser;
    class SidebarItemsModel;

    /*!
     * \brief This class represents the dialog to open a component in a
     * project.
     */
    class ProjectFileOpenDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ProjectFileOpenDialog(QString libraryFileName = QString(),
                                       QWidget *parent = nullptr);

        QString fileName() const { return m_fileName; }

    public Q_SLOTS:
        void itemDoubleClicked(const QString& item, const QString& category);

    private:
        SidebarItemsModel *m_sidebarItems;
        SidebarItemsBrowser *m_projectsSidebar;

        QString m_fileName;
        QString m_libraryName;
        QString m_libraryFileName;

        Ui::ProjectFileOpenDialog ui;
    };

} // namespace Caneda

#endif //PROJECT_FILE_OPEN_DIALOG_H
