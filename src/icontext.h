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

#ifndef CANEDA_ICONTEXT_H
#define CANEDA_ICONTEXT_H

#include <QObject>

// Forward declaration
class QFileInfo;
class QToolBar;
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IDocument;
    class SidebarChartsBrowser;
    class SidebarItemsBrowser;
    class SidebarItemsModel;
    class SidebarTextBrowser;

    /*************************************************************************
     *                     General IContext Structure                        *
     *************************************************************************/
    /*!
     * \brief This class provides an interface for a context which is
     * used by IDocument and IView. Only one instance of this class per
     * document type is used during the whole life span of the program.
     * This class answers the general questions about each document type,
     * like which file suffixes it can handle, points to the appropiate
     * methods to create new documents of its type, etc. This class also
     * provides objects like the toolbar, statusbar, etc, which are specific
     * to the particular context. The context class can also be used to host
     * functionalites shared by all views and documents of same type.
     *
     * Each inherited class must be a singleton class and thier only static
     * instance (returned by instance()) must be used.
     *
     * \sa IDocument, IView, \ref DocumentViewFramework
     */
    class IContext : public QObject
    {
        Q_OBJECT

    public:
        bool canOpen(const QFileInfo &info) const;

        virtual QStringList fileNameFilters() const = 0;
        virtual QStringList supportedSuffixes() const = 0;
        virtual QString defaultSuffix() const;

        virtual IDocument* newDocument() = 0;
        virtual IDocument* open(const QString& filename, QString *errorMessage = nullptr) = 0;

        virtual QToolBar* toolBar() = 0;
        virtual QWidget* sideBarWidget() = 0;
        virtual void updateSideBar() = 0;
        virtual void quickInsert() = 0;

    protected:
        explicit IContext(QObject *parent = nullptr);
    };


    /*************************************************************************
     *                Specialized IContext Implementations                   *
     *************************************************************************/
    /*!
     * \brief This class represents the schematic context interface
     * implementation.
     *
     * Only one instance of this class is used during the whole life span of
     * the program. This class answers the general questions like which file
     * suffixes it can handle, points to the appropiate methods to create new
     * documents of its type, etc.
     *
     * This class also provides objects like the toolbar, statusbar, etc, which
     * are specific to this particular context.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SchematicDocument, SchematicView
     */
    class SchematicContext : public IContext
    {
        Q_OBJECT

    public:
        static SchematicContext* instance();

        // IContext interface methods
        virtual QStringList fileNameFilters() const override;
        virtual QStringList supportedSuffixes() const override;

        virtual IDocument* newDocument() override;
        virtual IDocument* open(const QString &fileName, QString *errorMessage = nullptr) override;

        virtual QToolBar* toolBar() override { return nullptr; }
        virtual QWidget* sideBarWidget() override;
        virtual void updateSideBar() override {}
        virtual void quickInsert() override;
        // End of IContext interface methods

    private:
        explicit SchematicContext(QObject *parent = nullptr);

        SidebarItemsModel *m_sidebarItems;
        SidebarItemsBrowser *m_sidebarBrowser;
    };

    /*!
     * \brief This class represents the simulation context interface
     * implementation.
     *
     * Only one instance of this class is used during the whole life span of
     * the program. This class answers the general questions like which file
     * suffixes it can handle, points to the appropiate methods to create new
     * documents of its type, etc.
     *
     * This class also provides objects like the toolbar, statusbar, etc, which
     * are specific to this particular context.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SimulationDocument, SimulationView
     */
    class SimulationContext : public IContext
    {
        Q_OBJECT

    public:
        static SimulationContext* instance();

        // IContext interface methods
        virtual QStringList fileNameFilters() const override;
        virtual QStringList supportedSuffixes() const override;

        virtual IDocument* newDocument() override;
        virtual IDocument* open(const QString &fileName, QString *errorMessage = nullptr) override;

        virtual QToolBar* toolBar() override { return nullptr; }
        virtual QWidget* sideBarWidget() override;
        virtual void updateSideBar() override;
        virtual void quickInsert() override {}
        // End of IContext interface methods

    private:
        explicit SimulationContext(QObject *parent = nullptr);

        SidebarChartsBrowser *m_sidebarBrowser;
    };

    /*!
     * \brief This class represents the symbol context interface
     * implementation.
     *
     * Only one instance of this class is used during the whole life span of
     * the program. This class answers the general questions like which file
     * suffixes it can handle, points to the appropiate methods to create new
     * documents of its type, etc.
     *
     * This class also provides objects like the toolbar, statusbar, etc, which
     * are specific to this particular context.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SymbolDocument, SymbolView
     */
    class SymbolContext : public IContext
    {
        Q_OBJECT

    public:
        static SymbolContext* instance();

        // IContext interface methods
        virtual QStringList fileNameFilters() const override;
        virtual QStringList supportedSuffixes() const override;

        virtual IDocument* newDocument() override;
        virtual IDocument* open(const QString &fileName, QString *errorMessage = nullptr) override;

        virtual QToolBar* toolBar() override { return nullptr; }
        virtual QWidget* sideBarWidget() override;
        virtual void updateSideBar() override {}
        virtual void quickInsert() override;
        // End of IContext interface methods

    private:
        explicit SymbolContext(QObject *parent = nullptr);

        SidebarItemsModel *m_sidebarItems;
        SidebarItemsBrowser *m_sidebarBrowser;
    };

    /*!
     * \brief This class represents the text context interface implementation.
     *
     * Only one instance of this class is used during the whole life span of
     * the program. This class answers the general questions like which file
     * suffixes it can handle, points to the appropiate methods to create new
     * documents of its type, etc.
     *
     * This class also provides objects like the toolbar, statusbar, etc, which
     * are specific to this particular context.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa TextDocument, TextView
     */
    class TextContext : public IContext
    {
        Q_OBJECT

    public:
        static TextContext* instance();

        // IContext interface methods
        virtual QStringList fileNameFilters() const override;
        virtual QStringList supportedSuffixes() const override;

        virtual IDocument* newDocument() override;
        virtual IDocument* open(const QString& filename, QString *errorMessage = nullptr) override;

        virtual QToolBar* toolBar() override { return nullptr; }
        virtual QWidget* sideBarWidget() override;
        virtual void updateSideBar() override {}
        virtual void quickInsert() override {}
        // End of IContext interface methods

    private:
        explicit TextContext(QObject *parent = nullptr);

        SidebarTextBrowser *m_sidebarTextBrowser;
    };

} // namespace Caneda

#endif //CANEDA_ICONTEXT_H
