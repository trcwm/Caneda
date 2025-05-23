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

#include "projectfileopendialog.h"

#include "sidebaritemsbrowser.h"

#include <QDir>
#include <QFileInfo>

namespace Caneda
{
    //! \brief Constructor.
    ProjectFileOpenDialog::ProjectFileOpenDialog(QString libraryFileName,
                                                 QWidget *parent) :
        QDialog(parent)
    {
        ui.setupUi(this);

        //Add components browser
        m_sidebarItems = new SidebarItemsModel(this);
        m_projectsSidebar = new SidebarItemsBrowser(m_sidebarItems, this);

        if(!libraryFileName.isEmpty()) {
            m_libraryFileName = libraryFileName;
            m_libraryName = QFileInfo(libraryFileName).baseName();
            m_libraryName.replace(0, 1, m_libraryName.left(1).toUpper()); // First letter in uppercase

            m_sidebarItems->plugLibrary(m_libraryName, "root");
        }

        ui.layout->addWidget(m_projectsSidebar);

        connect(m_projectsSidebar, &SidebarItemsBrowser::itemDoubleClicked,
                this, &ProjectFileOpenDialog::itemDoubleClicked);

        m_fileName = QString();
    }

    void ProjectFileOpenDialog::itemDoubleClicked(const QString& item, const QString& category)
    {
        Q_UNUSED(category)

        QString baseName = item;
        if(!baseName.isEmpty()) {
            QString path = QFileInfo(m_libraryFileName).path();
            m_fileName = QDir::toNativeSeparators(path + "/" + baseName + ".xsch");
        }
        else {
            return;
        }

        QDialog::accept();
    }

} // namespace Caneda
