/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef CANEDA_MAINWINDOW_H
#define CANEDA_MAINWINDOW_H

#include "schematicscene.h"
#include "undocommands.h"

#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QProcess>
#include <QToolBar>
#include <QUndoView>

// Forward declarations
class QTermWidget;
class QLabel;
class QUndoGroup;

namespace Caneda
{
    // Forward declarations
    class Action;
    class ComponentsSidebar;
    class FolderBrowser;
    class Project;
    class SchematicScene;
    class SchematicWidget;
    class TabWidget;

    class MainWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        ~MainWindow();

        static MainWindow* instance();

        TabWidget* tabWidget() const;

        void saveSettings();
        void setNormalAction();
        void editFile(const QString &file);
        Action* action(const QString &name);
        QMenu* menubarMenu(const QString &name, bool createOnAbsence = true);
        QUndoGroup *m_undoGroup;

    public Q_SLOTS:
        void slotFileNew();
        void slotTextNew();
        void slotFileOpen(QString fileName = QString());
        void slotFileSave();
        void slotFileSaveAs();
        bool slotFileSaveAll();
        void slotFileClose();
        void slotFilePrint();
        void slotExportImage();
        void slotFileSettings();
        void slotApplSettings();

        void slotEditUndo();
        void slotEditRedo();
        void slotEditCut();
        void slotEditCopy();
        void slotEditPaste();
        void slotEditFind();
        void slotSelectAll();

        void slotNewProject();
        void slotOpenProject(QString fileName = 0);
        void slotAddToProject();
        void slotRemoveFromProject();
        void slotCloseProject();
        void slotBackupAndHistory();

        void slotCallFilter();
        void slotCallLine();
        void slotCallMatch();
        void slotCallAtt();
        void slotCallLibrary();
        void slotImportData();
        void slotShowConsole();

        void slotZoomIn();
        void slotZoomOut();
        void slotZoomBestFit();
        void slotZoomOriginal();
        void slotViewToolBar(bool);
        void slotViewStatusBar(bool);

        void slotSplitHorizontal();
        void slotSplitVertical();
        void slotCloseSplit();

        void slotHelpIndex();
        void slotHelpAbout();
        void slotHelpAboutQt();

        void setDocumentTitle(const QString& title);
        void updateTitle();
        void slotUpdateSettingsChanges();
        void slotStatusBarMessage(const QString& newPos);

    signals:
        void signalKillWidgets();

    protected:
        void closeEvent(QCloseEvent *closeEvent);

    private Q_SLOTS:
        void loadSettings();
        void slotProccessError(QProcess::ProcessError error);

    private:
        MainWindow(QWidget *w=0);
        void addAsDockWidget(QWidget *w, const QString &title = QString(),
                Qt::DockWidgetArea area = Qt::LeftDockWidgetArea);

        // REMOVE the below methods later.
        void removeChildWidget(QWidget *widget, bool deleteWidget = true);
        void closeAllTabs();

        void initActions();
        void initMouseActions();
        void initMenus();
        void initToolBars();
        void initStatusBar();

        void showHTML(const QString& Page);

        void createUndoView();
        void createFolderView();
        void setupSidebar();
        void setupProjectsSidebar();


        // menus contain the items of their menubar
        QMenu *fileMenu, *editMenu, *insMenu, *projMenu, *simMenu, *viewMenu,
              *docksMenu, *helpMenu, *alignMenu, *toolMenu;

        QLabel *m_statusLabel;
        QToolBar *fileToolbar, *editToolbar, *viewToolbar, *workToolbar;
        QDockWidget *sidebarDockWidget;
        QUndoView *undoView;
        ComponentsSidebar *m_componentsSidebar;
        Project *m_project;
        QDockWidget *projectDockWidget;
        FolderBrowser *m_folderBrowser;
        TabWidget *m_tabWidget;
        QTermWidget *console;
        QDockWidget *consoleDockWidget;
        QString titleText;
    };

} // namespace Caneda

#endif //CANEDA_MAINWINDOW_H