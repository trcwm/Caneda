/***************************************************************************
 * Copyright 2010 Pablo Daniel Pareja Obregon                              *
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

#ifndef PROJECT_FILE_DIALOG_H
#define PROJECT_FILE_DIALOG_H

#include <QDialog>

namespace Caneda
{
    // Forward declarations
    class ComponentsSidebar;

    /*!
     * This class represents the dialog to open a component in
     * a project.
     */
    class ProjectFileDialog : public QDialog
    {
        Q_OBJECT;

    public:
        ProjectFileDialog(QString = "", QWidget * = 0);
        ~ProjectFileDialog();

        QString fileName() const { return m_fileName; }

    private Q_SLOTS:
        void slotAccept();
        void slotOnDoubleClick(const QString&, const QString&);

    private:
        ComponentsSidebar *m_projectsSidebar;
        QString m_fileName;
        QString m_libraryName;
        QString m_libraryFileName;
    };

} // namespace Caneda

#endif //PROJECT_FILE_DIALOG_H