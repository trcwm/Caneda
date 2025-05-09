/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "icontext.h"

#include "idocument.h"
#include "library.h"
#include "quickinsert.h"
#include "settings.h"
#include "sidebarchartsbrowser.h"
#include "sidebaritemsbrowser.h"
#include "sidebartextbrowser.h"
#include "statehandler.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    /*************************************************************************
     *                             IContext                                  *
     *************************************************************************/
    //! \brief Constructor.
    IContext::IContext(QObject *parent) : QObject(parent)
    {
    }

    /*!
     * \brief Indicates if a particular file extension is managed by this
     * context.
     *
     * This method indicates if a particular file extension is managed by this
     * context. This allows to find what context is in charge of a particular
     * file type.
     */
    bool IContext::canOpen(const QFileInfo &info) const
    {
        foreach (const QString &suffix, supportedSuffixes()) {
            if (info.suffix() == suffix) {
                return true;
            }
        }

        return false;
    }

    /*!
     * \brief Returns the default suffix of the current context type.
     *
     * This method returns the default suffix of this context to be used in
     * any dialog required. It always returns the first suffix of the
     * supportedSuffixes method.
     *
     * \sa supportedSuffixes()
     */
    QString IContext::defaultSuffix() const
    {
        return supportedSuffixes().first();
    }

    /*!
     * \fn IContext::fileNameFilters()
     *
     * \brief Returns the filename extensions or filters available for this
     * context.
     *
     * Filename filters are used in open/save dialogs to allow the user to
     * filter the files displayed and ease the selection of the wanted file. In
     * this way, for example, if opening a schematic document the dialog should
     * display only schematic files. The method fileNameFilters() is used to
     * know what extensions correspond to that particular type of context.
     */

    /*!
     * \fn IContext::supportedSuffixes()
     *
     * \brief Returns the filename extensions available for this context.
     *
     * \sa defaultSuffix(), fileNameFilters()
     */

    /*!
     * \fn IContext::newDocument()
     *
     * \brief Create a new document of the current context type.
     */

    /*!
     * \fn IContext::open()
     *
     * \brief Open a document of the current context type.
     */

    /*!
     * \fn IContext::toolBar()
     *
     * \brief Returns the toolbar corresponding to this context.
     *
     * There are two type of toolbars:
     * \li Main toolbars, containing common actions as copy, cut, paste, undo,
     * etc.
     * \li Context sensitive toolbars, containing only those actions relative
     * to the current context as insert wire, rotate, etc.
     *
     * While the main toolbars are displayed in the main window for every
     * context, context sensitive toolbars are displayed inside each tab when
     * a specific type of context is opened. This method returns a pointer to
     * the current context toolbar.
     *
     * \todo Implement context sensitive toolbars and attach them inside each
     * tab.
     *
     * \sa sideBarWidget()
     */

    /*!
     * \fn IContext::sideBarWidget()
     *
     * \brief Returns the sideBarWidget corresponding to this context.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. In this sense, context sensitive sidebars are
     * changed every time a specific type of context is opened or changed to.
     * This method returns a pointer to the current context sideBarWidget.
     *
     * \sa toolBar(), updateSideBar()
     */

    /*!
     * \fn IContext::updateSideBar()
     *
     * \brief Updates sidebar contents.
     *
     * SideBarWidgets are context sensitive, containing only those items and
     * tools relative to the current context as components, painting tools,
     * code snippets, etc. Upon certain conditions, sidebars may need updating,
     * for example when inserting or removing libraries. This method allows for
     * an external event to request the update of the sidebar.
     *
     * \sa sideBarWidget()
     */

    /*!
     * \fn IContext::quickInsert()
     *
     * \brief Opens an insert dialog for items available in this context.
     *
     * Insert items are context sensitive, containing only those items relative
     * to the current context as components, painting tools, code snippets,
     * etc. This method allows for an external event to request an insert items
     * dialog.
     *
     * \sa sideBarWidget()
     */

    /*************************************************************************
     *                         Schematic Context                             *
     *************************************************************************/
    //! \brief Constructor.
    SchematicContext::SchematicContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarItems = new SidebarItemsModel(this);
        m_sidebarBrowser = new SidebarItemsBrowser(m_sidebarItems);

        connect(m_sidebarBrowser, QOverload<const QString&, const QString&>::of(&SidebarItemsBrowser::itemClicked),
                handler,          QOverload<const QString&, const QString&>::of(&StateHandler::insertItem));

        // Load schematic libraries
        LibraryManager *libraryManager = LibraryManager::instance();
        if(libraryManager->loadLibraryTree()) {

            // Get the libraries list and sort them alphabetically
            QStringList libraries(libraryManager->librariesList());
            libraries.sort();

            // Plug each library into the sidebar browser
            foreach(const QString library, libraries) {
                m_sidebarItems->plugLibrary(library, "Components");
                qDebug() << "Loaded " + library + " library";
            }

            qDebug() << "Successfully loaded libraries!";
        }
        else {
            // Invalidate entry
            qDebug() << "Error loading component libraries";
            qDebug() << "Please set the appropriate libraries through Application settings and restart the application.";
        }

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Ground"),
                QPixmap(Caneda::imageDirectory() + "ground.svg"));
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::imageDirectory() + "portsymbol.svg"));

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::imageDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::imageDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::imageDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::imageDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::imageDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::imageDirectory() + "text.svg"));

        m_sidebarItems->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));
        m_sidebarItems->plugItems(paintingItems, QObject::tr("Paint Tools"));
    }

    //! \copydoc MainWindow::instance()
    SchematicContext* SchematicContext::instance()
    {
        static SchematicContext *context = nullptr;
        if (!context) {
            context = new SchematicContext();
        }
        return context;
    }

    QStringList SchematicContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Schematic-xml (*.xsch)");

        return nameFilters;
    }

    QStringList SchematicContext::supportedSuffixes() const
    {
        // List of supported suffixes. The first suffix is the default value
        // provided by defaultSuffix() for all dialogs.
        QStringList supportedSuffixes;
        supportedSuffixes << "xsch";

        return supportedSuffixes;
    }

    IDocument* SchematicContext::newDocument()
    {
        return new SchematicDocument;
    }

    IDocument* SchematicContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SchematicDocument *document = new SchematicDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = nullptr;
        }

        return document;
    }

    QWidget* SchematicContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SchematicContext::quickInsert()
    {
        StateHandler *handler = StateHandler::instance();
        QuickInsert *quickInsert = new QuickInsert(m_sidebarItems);

        connect(quickInsert, QOverload<const QString&, const QString&>::of(&QuickInsert::itemClicked),
                handler,     QOverload<const QString&, const QString&>::of(&StateHandler::insertItem));

        quickInsert->exec(QCursor::pos());

        delete quickInsert;
    }

    /*************************************************************************
     *                        Simulation Context                             *
     *************************************************************************/
    //! \brief Constructor.
    SimulationContext::SimulationContext(QObject *parent) : IContext(parent)
    {
        m_sidebarBrowser = new SidebarChartsBrowser();
    }

    //! \copydoc MainWindow::instance()
    SimulationContext* SimulationContext::instance()
    {
        static SimulationContext *context = nullptr;
        if (!context) {
            context = new SimulationContext();
        }
        return context;
    }

    QStringList SimulationContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Raw waveform data (*.raw)");

        return nameFilters;
    }

    QStringList SimulationContext::supportedSuffixes() const
    {
        // List of supported suffixes. The first suffix is the default value
        // provided by defaultSuffix() for all dialogs.
        QStringList supportedSuffixes;
        supportedSuffixes << "raw";

        return supportedSuffixes;
    }

    IDocument* SimulationContext::newDocument()
    {
        return new SimulationDocument;
    }

    IDocument* SimulationContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SimulationDocument *document = new SimulationDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = nullptr;
        }

        return document;
    }

    QWidget *SimulationContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SimulationContext::updateSideBar()
    {
        if(m_sidebarBrowser) {
            m_sidebarBrowser->updateChartSeriesMap();
        }
    }

    /*************************************************************************
     *                          Symbol Context                               *
     *************************************************************************/
    //! \brief Constructor.
    SymbolContext::SymbolContext(QObject *parent) : IContext(parent)
    {
        StateHandler *handler = StateHandler::instance();
        m_sidebarItems = new SidebarItemsModel(this);
        m_sidebarBrowser = new SidebarItemsBrowser(m_sidebarItems);

        connect(m_sidebarBrowser, QOverload<const QString&, const QString&>::of(&SidebarItemsBrowser::itemClicked),
                handler,          QOverload<const QString&, const QString&>::of(&StateHandler::insertItem));

        QList<QPair<QString, QPixmap> > miscellaneousItems;
        miscellaneousItems << qMakePair(QObject::tr("Port Symbol"),
                QPixmap(Caneda::imageDirectory() + "portsymbol.svg"));

        QList<QPair<QString, QPixmap> > paintingItems;
        paintingItems << qMakePair(QObject::tr("Arrow"),
                QPixmap(Caneda::imageDirectory() + "arrow.svg"));
        paintingItems << qMakePair(QObject::tr("Ellipse"),
                QPixmap(Caneda::imageDirectory() + "ellipse.svg"));
        paintingItems << qMakePair(QObject::tr("Elliptic Arc"),
                QPixmap(Caneda::imageDirectory() + "ellipsearc.svg"));
        paintingItems << qMakePair(QObject::tr("Line"),
                QPixmap(Caneda::imageDirectory() + "line.svg"));
        paintingItems << qMakePair(QObject::tr("Rectangle"),
                QPixmap(Caneda::imageDirectory() + "rectangle.svg"));
        paintingItems << qMakePair(QObject::tr("Text"),
                QPixmap(Caneda::imageDirectory() + "text.svg"));

        m_sidebarItems->plugItems(miscellaneousItems, QObject::tr("Miscellaneous"));
        m_sidebarItems->plugItems(paintingItems, QObject::tr("Paint Tools"));
    }

    //! \copydoc MainWindow::instance()
    SymbolContext* SymbolContext::instance()
    {
        static SymbolContext *context = nullptr;
        if (!context) {
            context = new SymbolContext();
        }
        return context;
    }

    QStringList SymbolContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Symbol-xml (*.xsym)");

        return nameFilters;
    }

    QStringList SymbolContext::supportedSuffixes() const
    {
        // List of supported suffixes. The first suffix is the default value
        // provided by defaultSuffix() for all dialogs.
        QStringList supportedSuffixes;
        supportedSuffixes << "xsym";

        return supportedSuffixes;
    }

    IDocument* SymbolContext::newDocument()
    {
        return new SymbolDocument;
    }

    IDocument* SymbolContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SymbolDocument *document = new SymbolDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = nullptr;
        }

        return document;
    }

    QWidget* SymbolContext::sideBarWidget()
    {
        return m_sidebarBrowser;
    }

    void SymbolContext::quickInsert()
    {
        StateHandler *handler = StateHandler::instance();
        QuickInsert *quickInsert = new QuickInsert(m_sidebarItems);

        connect(quickInsert, QOverload<const QString&, const QString&>::of(&QuickInsert::itemClicked),
                handler,     QOverload<const QString&, const QString&>::of(&StateHandler::insertItem));

        quickInsert->exec(QCursor::pos());

        delete quickInsert;
    }

    /*************************************************************************
     *                           Text Context                                *
     *************************************************************************/
    //! \brief Constructor.
    TextContext::TextContext(QObject *parent) : IContext(parent)
    {
        m_sidebarTextBrowser = new SidebarTextBrowser();
    }

    //! \copydoc MainWindow::instance()
    TextContext* TextContext::instance()
    {
        static TextContext *instance = nullptr;
        if (!instance) {
            instance = new TextContext();
        }
        return instance;
    }

    QStringList TextContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Spice netlist (*.spc *.sp *.net *.cir)");
        nameFilters << QObject::tr("HDL source (*.vhdl *.vhd *.v)");
        nameFilters << QObject::tr("Text file (*.txt)");

        return nameFilters;
    }

    QStringList TextContext::supportedSuffixes() const
    {
        // List of supported suffixes. The first suffix is the default value
        // provided by defaultSuffix() for all dialogs.
        QStringList supportedSuffixes;
        supportedSuffixes << "txt";
        supportedSuffixes << "log";
        supportedSuffixes << "net";
        supportedSuffixes << "cir";
        supportedSuffixes << "spc";
        supportedSuffixes << "sp";
        supportedSuffixes << "vhd";
        supportedSuffixes << "vhdl";
        supportedSuffixes << "v";
        supportedSuffixes << "";

        return supportedSuffixes;
    }

    IDocument* TextContext::newDocument()
    {
        return new TextDocument;
    }

    IDocument* TextContext::open(const QString& fileName, QString *errorMessage)
    {
        TextDocument *document = new TextDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = nullptr;
        }

        return document;
    }

    QWidget* TextContext::sideBarWidget()
    {
        return m_sidebarTextBrowser;
    }

} // namespace Caneda
