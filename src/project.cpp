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

#include "project.h"

#include "actionmanager.h"
#include "fileformats.h"
#include "gitmanager.h"
#include "global.h"
#include "icontext.h"
#include "idocument.h"
#include "library.h"
#include "projectfilenewdialog.h"
#include "sidebaritemsbrowser.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructs a project manager widget.
     *
     * \param parent Parent of the widget.
     */
    Project::Project(QWidget *parent) :
        QWidget(parent),
        m_libraryName(),
        m_libraryFileName()
    {
        projectLibrary = nullptr;

        QVBoxLayout *layout = new QVBoxLayout(this);

        QToolBar *toolbar = new QToolBar(this);

        //! \todo Reenable these menus once project and tools reimplemented.
        //        ActionManager* am = ActionManager::instance();
        //        toolbar->addAction(am->actionForName("projNew"));
        //        toolbar->addAction(am->actionForName("projOpen"));
        //        toolbar->addAction(am->actionForName("addToProj"));
        //        toolbar->addAction(am->actionForName("projDel"));
        //        toolbar->addAction(am->actionForName("projClose"));

        m_sidebarItems = new SidebarItemsModel(this);
        m_projectsSidebar = new SidebarItemsBrowser(m_sidebarItems, this);

        connect(m_projectsSidebar, QOverload<const QString&, const QString&>::of(&SidebarItemsBrowser::itemClicked),      this, &Project::itemClicked);
        connect(m_projectsSidebar,                                               &SidebarItemsBrowser::itemDoubleClicked, this, &Project::slotOnDoubleClicked);

        layout->addWidget(toolbar);
        layout->addWidget(m_projectsSidebar);

        setWindowTitle(tr("Project View"));
    }

    bool Project::isValid()
    {
        return (projectLibrary == nullptr ? false : true);
    }

    void Project::slotNewProject()
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                        QString(), tr("Caneda Projects (*.xpro)"));
        if(fileName.isEmpty()) {
            return;
        }

        if(QFileInfo(fileName).suffix().isEmpty()) {
            fileName = fileName + ".xpro";
        }

        //First we create the folder structure where files are to be placed
        QFileInfo fileInfo = QFileInfo(fileName);
        QDir filePath = QDir(fileInfo.path() + "/" + fileInfo.baseName());
        if(!filePath.exists()) {
            filePath.mkpath(filePath.absolutePath());
        }
        fileName = filePath.absolutePath() + "/" + fileInfo.fileName();

        //Then we create the library/project
        LibraryManager *library = LibraryManager::instance();
        if(library->newLibrary(fileName)) {
            slotCloseProject();
            setCurrentLibrary(fileName);
            projectLibrary = library->library(m_libraryName);
            qDebug() << "Successfully created library!";
            m_sidebarItems->plugLibrary(m_libraryName, "root");
        }
    }

    void Project::slotOpenProject(QString fileName)
    {
        //If no name is provided, we open a dialog asking the user for a project to be opened
        if(fileName.isEmpty()) {
            fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    QString(), tr("Caneda Projects (*.xpro)"));
        }

        if(fileName.isEmpty()) {
            return;
        }

        LibraryManager *library = LibraryManager::instance();
        if(library->load(fileName)) {
            slotCloseProject();
            setCurrentLibrary(fileName);
            projectLibrary = library->library(m_libraryName);
            qDebug() << "Successfully loaded library!";
            m_sidebarItems->plugLibrary(m_libraryName, "root");
        }
    }

    void Project::slotAddToProject()
    {
        if(projectLibrary) {
            ProjectFileNewDialog *dialog = new ProjectFileNewDialog(this);
            int status = dialog->exec();

            if(status == QDialog::Accepted) {
                if(dialog->userChoice() == Caneda::ExistingComponent) {
                    addExistingComponent();
                }
                else if(dialog->userChoice() == Caneda::NewComponent) {
                    addNewComponent(dialog->fileName());
                }
                else {
                    importFromProject();
                }
            }

            delete dialog;
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                    tr("Invalid project!"));
            return;
        }
    }

    //! \todo Reimplement or remove this
    void Project::slotRemoveFromProject()
    {
//        if(projectLibrary) {
//            if(!m_projectsSidebar->currentComponent().isEmpty()) {

//                int ret = QMessageBox::warning(this, tr("Delete component"),
//                                  tr("You're about to delete one component. This action can't be undone.\n"
//                                     "Do you want to continue?"),
//                                  QMessageBox::Ok | QMessageBox::Cancel);

//                switch (ret) {
//                    case QMessageBox::Ok: {

//                        QString fileName = projectLibrary->component(m_projectsSidebar->currentComponent())->filename;
//                        fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + QFileInfo(fileName).baseName();

//                        QFile::remove(fileName + ".xsch");
//                        QFile::remove(fileName + ".xsym");

//                        projectLibrary->removeComponent(m_projectsSidebar->currentComponent());
//                        m_projectsSidebar->unPlugLibrary(m_libraryName, "root");
//                        m_projectsSidebar->plugLibrary(m_libraryName, "root");
//                        break;
//                    }
//                    case QMessageBox::Cancel:
//                        break;

//                    default:
//                        break;
//                }
//            }
//        }
//        else {
//            QMessageBox::critical(this, tr("Error"),
//                    tr("Invalid project!"));
//            return;
//        }
    }

    void Project::slotCloseProject()
    {
        if(projectLibrary) {
            m_sidebarItems->unPlugLibrary(m_libraryName, "root");
            LibraryManager *library = LibraryManager::instance();
            library->unload(m_libraryName);

            projectLibrary = nullptr;
            m_libraryFileName = QString();
            m_libraryName = QString();
        }
    }

    void Project::slotBackupAndHistory()
    {
        if(projectLibrary) {
            GitManager *dialog = new GitManager(QFileInfo(m_libraryFileName).absolutePath(), this);
            dialog->exec();
            delete dialog;
        }
    }

    void Project::slotOnDoubleClicked(const QString& item, const QString& category)
    {
        Q_UNUSED(category)

        emit itemDoubleClicked(QFileInfo(m_libraryFileName).absolutePath() + "/" + item + ".xsch");
    }

    void Project::setCurrentLibrary(const QString& libFileName)
    {
        m_libraryFileName = libFileName;

        //The library name is created from the basename with the first letter in uppercase
        m_libraryName = QFileInfo(libFileName).baseName();
        m_libraryName.replace(0, 1, m_libraryName.left(1).toUpper()); // First letter in uppercase
    }

    void Project::addExistingComponent()
    {
        QString sourceFileName = QFileDialog::getOpenFileName(this, tr("Add File to Project"),
                                                              QString(), tr("Component-xml (*.xsch)"));

        if(sourceFileName.isEmpty()) {
            return;
        }

        SchematicContext *context = SchematicContext::instance();
        QString errorMessage;
        QScopedPointer<IDocument> document(context->open(sourceFileName, &errorMessage));
        if (!document) {
            QMessageBox::critical(this, tr("Error opening file"), errorMessage);
            return;
        }

        //We copy the selected file to current project folder
        QString fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + QFileInfo(sourceFileName).fileName();
        if(!QFile::copy(sourceFileName, fileName)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Component %1 already exists in project!").arg(QFileInfo(fileName).baseName()));
            return;
        }

        //We generate the corresponding symbol
        generateSymbol(fileName);

        //Finally we open the component
        emit itemDoubleClicked(fileName);
    }

    void Project::addNewComponent(const QString& componentName)
    {
        QString fileName = QFileInfo(m_libraryFileName).absolutePath() + "/" + componentName +".xsch";

        //When the component is already created, we return.
        if(QFileInfo(fileName).exists()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Component already created!"));
            return;
        }

        SchematicContext *context = SchematicContext::instance();
        QScopedPointer<IDocument> document(context->newDocument());
        if (!document) {
            qWarning() << Q_FUNC_INFO << "newDocument() failed";
            return;
        }
        document->setFileName(fileName);
        QString errorMessage;
        if(!document->save(&errorMessage)) {
            QMessageBox::critical(this, tr("Error saving file"), errorMessage);
            return;
        }

        //We generate the corresponding symbol
        generateSymbol(fileName);

        //Finally we open the component
        emit itemDoubleClicked(fileName);
    }

    void Project::importFromProject()
    {
        /*!
         * \todo In case of adding a component from another project, we
         * should copy the component as well as all its dependencies.
         */
    }

    /*!
     * \brief Generates the symbol corresponding to a schematic file.
     */
    void Project::generateSymbol(const QString& fileName)
    {
//        //PORT:
//        QString symbolFileName = fileName;

//        // First, we open the symbol from corresponding schematic
//        SchematicContext *context = SchematicContext::instance();
//        QScopedPointer<SchematicDocument> document(qobject_cast<SchematicDocument*>(context->newDocument()));
//        document->setFileName(symbolFileName);
//        document->graphicsScene()->setMode(Caneda::SymbolMode);
//        document->load();

//        // Then we save the symbol in a xsym file
//        symbolFileName.replace(".xsch",".xsym");
//        document->setFileName(symbolFileName);
//        FormatXmlSymbol *symbol = new FormatXmlSymbol(document.data());
//        symbol->save();

//        // Finally we load the new component in the library
//        projectLibrary->parseExternalComponent(symbolFileName);
//        projectLibrary->saveLibrary();
//        m_projectsSidebar->unPlugLibrary(m_libraryName, "root");
//        m_projectsSidebar->plugLibrary(m_libraryName, "root");
    }

} // namespace Caneda
