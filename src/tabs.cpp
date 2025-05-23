/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "tabs.h"

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "global.h"
#include "icontext.h"
#include "idocument.h"
#include "iview.h"
#include "mainwindow.h"

#include <QDebug>
#include <QDockWidget>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolBar>
#include <QWheelEvent>
#include <QUndoGroup>

namespace Caneda
{
    /*************************************************************************
     *                             ViewContainer                             *
     *************************************************************************/
    //! \brief Constructor.
    ViewContainer::ViewContainer(IView *view, QWidget *parent) :
        QWidget(parent),
        m_view(nullptr),
        m_toolBar(nullptr)
    {
        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);
        setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        setView(view);
    }

    //! \brief Destructor.
    ViewContainer::~ViewContainer()
    {
        // Don't let QWidget destructor destroy toolbar as the view might still exist.
        if (m_toolBar) {
            layout()->removeWidget(m_toolBar);
            m_toolBar->setParent(nullptr);
        }
    }

    IView* ViewContainer::view() const
    {
        return m_view;
    }

    void ViewContainer::setView(IView *view)
    {
        QLayout *layout = this->layout();

        if (m_view) {
            QWidget *widget = m_view->toWidget();
            layout->removeWidget(widget);
            widget->setParent(nullptr);

            disconnect(m_view, &IView::focussedIn, this, &ViewContainer::onViewFocusChange);

            setToolBar(nullptr);
        }

        m_view = view;

        if (m_view) {
            QWidget *widget = m_view->toWidget();
            widget->setParent(this);
            layout->addWidget(widget);

            connect(m_view, &IView::focussedIn, this, &ViewContainer::onViewFocusChange);

            setToolBar(m_view->toolBar());
        }
    }

    void ViewContainer::setToolBar(QToolBar *toolbar)
    {
        QVBoxLayout *lay = qobject_cast<QVBoxLayout*>(layout());
        if (m_toolBar) {
            m_toolBar->setParent(nullptr);
            lay->removeWidget(m_toolBar);
        }

        m_toolBar = toolbar;

        if (m_toolBar) {
            m_toolBar->setParent(this);
            lay->insertWidget(0, m_toolBar);
        }
    }

    void ViewContainer::onViewFocusChange(IView *view)
    {
        Q_UNUSED(view)

        //! \todo Uncomment this line after fixing ViewContainer::paintEvent
        //update();
    }

    void ViewContainer::paintEvent(QPaintEvent *event)
    {
        QWidget::paintEvent(event);
        bool hasFocus = m_view && m_view->toWidget()->hasFocus();

        Q_UNUSED(hasFocus)
        //! \todo Draw some focus helper.
    }


    /*************************************************************************
     *                                 Tab                                   *
     *************************************************************************/
    //! \brief Constructor.
    Tab::Tab(IView *view, QWidget *parent) : QWidget(parent)
    {
        addView(view);

        QHBoxLayout *layout = new QHBoxLayout(this);

        QSplitter *splitter = new QSplitter();
        splitter->setContentsMargins(0, 0, 0, 0);
        splitter->addWidget(new ViewContainer(view));

        layout->addWidget(splitter);
        setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    IView* Tab::activeView() const
    {
        return m_views.isEmpty() ? nullptr : m_views.first();
    }

    QList<IView*> Tab::views() const
    {
        return m_views;
    }

    QString Tab::tabText() const
    {
        IView *view = activeView();
        QString title;
        if (view) {
            title = view->document()->fileName();
        }

        if (title.isEmpty()) {
            title = tr("Untitled");
        } else {
            title = QFileInfo(title).fileName();
        }

        return title;
    }

    QIcon Tab::tabIcon() const
    {
        IView *view = activeView();
        if (view) {
            return view->document()->isModified() ? modifiedIcon() : unmodifiedIcon();
        }

        return QIcon();
    }

    void Tab::splitView(IView *view, IView *newView,
            Qt::Orientation splitOrientation)
    {
        QWidget *asWidget = view->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(parentContainer->parentWidget());

        if (parentSplitter->orientation() != splitOrientation &&
                parentSplitter->count() == 1) {
            parentSplitter->setOrientation(splitOrientation);
        }

        if (parentSplitter->orientation() == splitOrientation) {
            parentSplitter->addWidget(new ViewContainer(newView));
        } else {
            int index = parentSplitter->indexOf(parentContainer);
            parentContainer->setParent(nullptr);

            QSplitter *newSplitter = new QSplitter(splitOrientation);
            newSplitter->setContentsMargins(0, 0, 0, 0);
            newSplitter->addWidget(parentContainer);
            newSplitter->addWidget(new ViewContainer(newView));
            parentSplitter->insertWidget(index, newSplitter);
        }

        addView(newView);
        newView->toWidget()->setFocus();
    }

    void Tab::closeView(IView *view)
    {
        if (!m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View doesn't exist in this tab";
            return;
        }

        QWidget *asWidget = view->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(parentContainer->parentWidget());

        parentContainer->setParent(nullptr);
        parentContainer->setView(nullptr);
        parentContainer->deleteLater();
        m_views.removeAll(view);
        // Remember, we do not delete the view itself here. It is handled in
        // DocumentViewManager.

        bool removeThisTab = false;

        // Get rid of unnecessary splitters recursively.
        while (1) {
            if (parentSplitter->count() > 0) break;

            QWidget *ancestor = parentSplitter->parentWidget();

            if (static_cast<QWidget*>(this) == ancestor) {
                removeThisTab = true;
                break;
            }

            parentSplitter->setParent(nullptr);
            parentSplitter->deleteLater();

            parentSplitter = qobject_cast<QSplitter*>(ancestor);
        }

        if (removeThisTab) {
            QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(parentWidget());

            TabWidget *tabWidget = qobject_cast<TabWidget*>(stackedWidget->parentWidget());
            tabWidget->removeTab(tabWidget->indexOf(this));
            deleteLater();
        }
    }

    void Tab::replaceView(IView *oldView, IView *newView)
    {
        if (!m_views.contains(oldView)) {
            qDebug() << Q_FUNC_INFO << "View doesn't exist in this tab";
            return;
        }

        QWidget *asWidget = oldView->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        m_views.removeAll(oldView);

        parentContainer->setView(newView);

        addView(newView);
        newView->toWidget()->setFocus();
    }

    void Tab::onViewFocussedIn(IView *view)
    {
        // Apply LRU algo by pushing the least recently used
        // view to the front of the list.
        int i = m_views.indexOf(view);
        if (i >= 0 && i < m_views.size()) {
            m_views.takeAt(i);
            m_views.insert(0, view);
        }

        emit tabInfoChanged(this);
    }

    void Tab::onDocumentChanged(IDocument *document)
    {
        Q_UNUSED(document)

        emit tabInfoChanged(this);
    }

    void Tab::onStatusBarMessage(const QString &message)
    {
        emit statusBarMessage(this, message);
    }

    void Tab::closeEvent(QCloseEvent *event)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IDocument*> documents;
        QSet<IDocument*> processedDocuments;
        foreach (IView *view, m_views) {
            IDocument *document = view->document();

            if (processedDocuments.contains(document)) {
                continue;
            }

            processedDocuments << document;

            documents << document;
        }

        if (!documents.isEmpty()) {
            bool status = manager->saveDocuments(documents);
            if (!status) {
                event->ignore();
                return;
            }
        }

        const bool askForSave = false;
        while (1) {
            if (m_views.isEmpty()) break;

            IView *view = m_views.first();
            manager->closeView(view, askForSave);
        }

        event->accept();
    }

    void Tab::addView(IView *view)
    {
        if (m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View is already added.";
            return;
        }

        m_views.insert(0, view);

        connect(view, &IView::focussedIn, this, &Tab::onViewFocussedIn);
        connect(view->document(), &IDocument::documentChanged, this, &Tab::onDocumentChanged);
        connect(view, &IView::statusBarMessage, this, &Tab::onStatusBarMessage);

        emit tabInfoChanged(this);
    }

    QIcon Tab::modifiedIcon() const
    {
        static QIcon modifiedIcon;
        if (modifiedIcon.isNull()) {
            modifiedIcon = Caneda::icon("document-save");
        }
        return modifiedIcon;
    }

    QIcon Tab::unmodifiedIcon() const
    {
        static QIcon unmodifiedIcon;
        if (unmodifiedIcon.isNull()) {
            unmodifiedIcon = Caneda::icon("unmodified.png");
        }
        return unmodifiedIcon;
    }


    /*************************************************************************
     *                             TabWidget                                 *
     *************************************************************************/
    //! \brief Constructor.
    TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
    {
        setTabBar(new QTabBar(this));

        connect(this, &TabWidget::currentChanged,    this, &TabWidget::updateTabContext);
        connect(this, &TabWidget::tabCloseRequested, this, &TabWidget::onTabCloseRequested);
    }

    QList<Tab*> TabWidget::tabs() const
    {
        QList<Tab*> tabs;
        for (int i = 0; i < count(); ++i) {
            Tab *tab = qobject_cast<Tab*>(widget(i));
            if (!tab) {
                qDebug() << Q_FUNC_INFO << "Consists of tab that isn't a Tab";
            }
            tabs << tab;
        }
        return tabs;
    }

    void TabWidget::addTab(Tab *tab)
    {
        insertTab(-1, tab);
    }

    void TabWidget::insertTab(int index, Tab *tab)
    {
        QTabWidget::insertTab(index, tab, tab->tabIcon(), tab->tabText());

        connect(tab, &Tab::tabInfoChanged,   this, &TabWidget::updateTabContext);
        connect(tab, &Tab::statusBarMessage, this, &TabWidget::onStatusBarMessage);

        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        IDocument *document = view->document();
        if (!document) {
            return;
        }
    }

    Tab* TabWidget::currentTab() const
    {
        return qobject_cast<Tab*>(currentWidget());
    }

    void TabWidget::setCurrentTab(Tab *tab)
    {
        setCurrentWidget(tab);
    }

    void TabWidget::closeAllTabs()
    {
        while(count() > 0){
            if(currentIndex() >= 0) {
                QWidget *w = widget(currentIndex());

                if(w->close()) {
                    //FIXME:emit closedWidget(w);
                    w->deleteLater();
                }
                else {
                    return;
                }

                removeTab(currentIndex());
            }
        }
    }

    void TabWidget::highlightView(IView *view)
    {
        if (!view) return;

        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = nullptr;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        setCurrentTab(parentTab);
        asWidget->setFocus();
    }

    void TabWidget::closeView(IView *view)
    {
        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = nullptr;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        parentTab->closeView(view);
    }

    void TabWidget::replaceView(IView *oldView, IView *newView)
    {
        Tab *tab = tabForView(oldView);
        tab->replaceView(oldView, newView);
    }

    Tab* TabWidget::tabForView(IView *view) const
    {
        QWidget *widget = view->toWidget();

        Tab *tab = nullptr;
        while (1) {
            if (!widget) {
                break;
            }

            tab = qobject_cast<Tab*>(widget->parentWidget());
            if (tab) {
                break;
            }

            widget = widget->parentWidget();
        }

        return tab;
    }

    void TabWidget::updateTabContext()
    {
        MainWindow *mw = MainWindow::instance();
        mw->updateWindowTitle();

        int index = currentIndex();
        if (index < 0 || index >= count()) {
            return;
        }

        Tab *tab = currentTab();
        setTabIcon(index, tab->tabIcon());
        setTabText(index, tab->tabText());

        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        QWidget *sidebar = view->context()->sideBarWidget();
        mw->sidebarDockWidget()->setWindowTitle(sidebar->windowTitle());
        mw->sidebarDockWidget()->setWidget(sidebar);
        view->context()->updateSideBar();

        IDocument *document = view->document();
        if (!document) {
            return;
        }

        ActionManager* am = ActionManager::instance();
        am->actionForName("editCut")->setEnabled(document->canCut());
        am->actionForName("editCopy")->setEnabled(document->canCopy());
        am->actionForName("editPaste")->setEnabled(document->canPaste());
        am->actionForName("editUndo")->setEnabled(document->canUndo());
        am->actionForName("editRedo")->setEnabled(document->canRedo());
    }

    void TabWidget::onStatusBarMessage(Tab *tab, const QString &message)
    {
        int index = indexOf(tab);
        if (index == currentIndex()) {
            emit statusBarMessage(message);
        }
    }

    void TabWidget::onTabCloseRequested(int index)
    {
        QWidget *w = widget(index);
        w->close();
    }

} // namespace Caneda
