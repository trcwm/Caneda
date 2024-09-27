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

#ifndef CANEDA_IDOCUMENT_H
#define CANEDA_IDOCUMENT_H

#include <QObject>
#include <QGraphicsSceneEvent>

// Forward declarations
class QPaintDevice;
class QPrinter;
class QTextDocument;

namespace Caneda
{
    // Forward declarations
    class GraphicsScene;
    class ChartScene;
    class DocumentViewManager;
    class IContext;
    class IView;
    class TextEdit;

    /*************************************************************************
     *                    General IDocument Structure                        *
     *************************************************************************/
    /*!
     * \brief This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture,
     * serving as an interface for all documents that can be handled by
     * Caneda. This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself may be included as a pointer to
     * another class that contains all the scene specific methods (for
     * example a graphics scene). The scene, in its turn, serves as a
     * container for item objects and handles their manipulation.
     *
     * \sa IContext, IView, \ref DocumentViewFramework
     */
    class IDocument : public QObject
    {
        Q_OBJECT

    public:
        explicit IDocument(QObject *parent = nullptr);

        QString fileName() const;
        void setFileName(const QString &fileName);

        // Virtual methods.
        virtual IContext* context() = 0;

        virtual bool isModified() const = 0;

        virtual bool canUndo() const = 0;
        virtual bool canRedo() const = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual bool canCut() const = 0;
        virtual bool canCopy() const = 0;
        virtual bool canPaste() const = 0;

        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;

        virtual void selectAll() = 0;

        virtual void enterHierarchy() = 0;
        virtual void exitHierarchy() = 0;

        virtual void alignTop() = 0;
        virtual void alignBottom() = 0;
        virtual void alignLeft() = 0;
        virtual void alignRight() = 0;
        virtual void distributeHorizontal() = 0;
        virtual void distributeVertical() = 0;
        virtual void centerHorizontal() = 0;
        virtual void centerVertical() = 0;

        virtual void simulate() = 0;

        virtual bool printSupportsFitInPage() const = 0;
        virtual void print(QPrinter *printer, bool fitInPage) = 0;
        virtual void exportImage(QPaintDevice &device) = 0;
        virtual QSizeF documentSize() = 0;

        virtual bool load(QString *errorMessage = nullptr) = 0;
        virtual bool save(QString *errorMessage = nullptr) = 0;

        virtual IView* createView() = 0;
        QList<IView*> views() const;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) = 0;
        virtual void launchPropertiesDialog() = 0;

    public Q_SLOTS:
        void emitDocumentChanged();

    Q_SIGNALS:
        void documentChanged(IDocument *document);
        void statusBarMessage(const QString &text);

        // Avoid private declarations as subclasses might need direct access.
    protected:
        friend class DocumentViewManager;
        QString m_fileName;
    };


    /*************************************************************************
     *                  Specialized IDocument Implementations                *
     *************************************************************************/
    /*!
     * \brief This class represents the layout document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself is included as a pointer to
     * GraphicsScene, that contains all the scene specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa LayoutContext, LayoutView
     */
    class LayoutDocument : public IDocument
    {
        Q_OBJECT

    public:
        explicit LayoutDocument(QObject *parent = nullptr);
        ~LayoutDocument() override;

        // IDocument interface methods
        virtual IContext* context() override;

        virtual bool isModified() const override;

        virtual bool canUndo() const override;
        virtual bool canRedo() const override;

        virtual void undo() override;
        virtual void redo() override;

        virtual bool canCut() const override;
        virtual bool canCopy() const override;
        virtual bool canPaste() const override { return true; }

        virtual void cut() override;
        virtual void copy() override;
        virtual void paste() override;

        virtual void selectAll() override;

        virtual void enterHierarchy() override;
        virtual void exitHierarchy() override;

        virtual void alignTop() override;
        virtual void alignBottom() override;
        virtual void alignLeft() override;
        virtual void alignRight() override;
        virtual void distributeHorizontal() override;
        virtual void distributeVertical() override;
        virtual void centerHorizontal() override;
        virtual void centerVertical() override;

        virtual void simulate() override;

        virtual bool printSupportsFitInPage() const override { return true; }
        virtual void print(QPrinter *printer, bool fitInView) override;
        virtual void exportImage(QPaintDevice &device) override;
        virtual QSizeF documentSize() override;

        virtual bool load(QString *errorMessage = nullptr) override;
        virtual bool save(QString *errorMessage = nullptr) override;

        virtual IView* createView() override;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
        virtual void launchPropertiesDialog() override;
        // End of IDocument interface methods

        GraphicsScene* graphicsScene() const { return m_graphicsScene; }

    private:
        GraphicsScene *m_graphicsScene;

        void alignElements(Qt::Alignment alignment);
    };

    /*!
     * \brief This class represents the schematic document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself is included as a pointer to
     * GraphicsScene, that contains all the scene specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SchematicContext, SchematicView
     */
    class SchematicDocument : public IDocument
    {
        Q_OBJECT

    public:
        explicit SchematicDocument(QObject *parent = nullptr);
        ~SchematicDocument() override;

        // IDocument interface methods
        virtual IContext* context() override;

        virtual bool isModified() const override;

        virtual bool canUndo() const override;
        virtual bool canRedo() const override;

        virtual void undo() override;
        virtual void redo() override;

        virtual bool canCut() const override;
        virtual bool canCopy() const override;
        virtual bool canPaste() const override { return true; }

        virtual void cut() override;
        virtual void copy() override;
        virtual void paste() override;

        virtual void selectAll() override;

        virtual void enterHierarchy() override;
        virtual void exitHierarchy() override;

        virtual void alignTop() override;
        virtual void alignBottom() override;
        virtual void alignLeft() override;
        virtual void alignRight() override;
        virtual void distributeHorizontal() override;
        virtual void distributeVertical() override;
        virtual void centerHorizontal() override;
        virtual void centerVertical() override;

        virtual void simulate() override;

        virtual bool printSupportsFitInPage() const override { return true; }
        virtual void print(QPrinter *printer, bool fitInView) override;
        virtual void exportImage(QPaintDevice &device) override;
        virtual QSizeF documentSize() override;

        virtual bool load(QString *errorMessage = nullptr) override;
        virtual bool save(QString *errorMessage = nullptr) override;

        virtual IView* createView() override;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
        virtual void launchPropertiesDialog() override;
        // End of IDocument interface methods

        GraphicsScene* graphicsScene() const { return m_graphicsScene; }

    private Q_SLOTS:
        void simulationReady(int error);
        bool simulationError();
        void showSimulationHelp();

    private:
        GraphicsScene *m_graphicsScene;

        void alignElements(Qt::Alignment alignment);
        bool performBasicChecks();
    };

    /*!
     * \brief This class represents the simulation document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself is included as a pointer to
     * ChartScene, that contains all the scene specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SimulationContext, SimulationView
     */
    class SimulationDocument : public IDocument
    {
        Q_OBJECT

    public:
        explicit SimulationDocument(QObject *parent = nullptr);
        ~SimulationDocument() override;

        // IDocument interface methods
        virtual IContext* context() override;

        virtual bool isModified() const override { return false; }

        virtual bool canUndo() const override { return false; }
        virtual bool canRedo() const override { return false; }

        virtual void undo() override {}
        virtual void redo() override {}

        virtual bool canCut() const override { return false; }
        virtual bool canCopy() const override { return false; }
        virtual bool canPaste() const override { return false; }

        virtual void cut() override {}
        virtual void copy() override {}
        virtual void paste() override {}

        virtual void selectAll() override {}

        virtual void enterHierarchy() override {}
        virtual void exitHierarchy() override {}

        virtual void alignTop() override {}
        virtual void alignBottom() override {}
        virtual void alignLeft() override {}
        virtual void alignRight() override {}
        virtual void distributeHorizontal() override;
        virtual void distributeVertical() override;
        virtual void centerHorizontal() override;
        virtual void centerVertical() override;

        virtual void simulate() override {}

        virtual bool printSupportsFitInPage() const override { return false; }
        virtual void print(QPrinter *printer, bool fitInView) override;
        virtual void exportImage(QPaintDevice &device) override;
        virtual QSizeF documentSize() override;

        virtual bool load(QString *errorMessage = nullptr) override;
        virtual bool save(QString *errorMessage = nullptr) override { return false; }

        virtual IView* createView() override;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *) override {}
        virtual void launchPropertiesDialog() override;
        // End of IDocument interface methods

        ChartScene* chartScene() const { return m_chartScene; }

    private:
        ChartScene *m_chartScene;
    };

    /*!
     * \brief This class represents the symbol document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself is included as a pointer to
     * GraphicsScene, that contains all the scene specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SymbolContext, SymbolView
     */
    class SymbolDocument : public IDocument
    {
        Q_OBJECT

    public:
        explicit SymbolDocument(QObject *parent = nullptr);
        ~SymbolDocument() override;

        // IDocument interface methods
        virtual IContext* context() override;

        virtual bool isModified() const override;

        virtual bool canUndo() const override;
        virtual bool canRedo() const override;

        virtual void undo() override;
        virtual void redo() override;

        virtual bool canCut() const override;
        virtual bool canCopy() const override;
        virtual bool canPaste() const override { return true; }

        virtual void cut() override;
        virtual void copy() override;
        virtual void paste() override;

        virtual void selectAll() override;

        virtual void enterHierarchy() override;
        virtual void exitHierarchy() override;

        virtual void alignTop() override;
        virtual void alignBottom() override;
        virtual void alignLeft() override;
        virtual void alignRight() override;
        virtual void distributeHorizontal() override;
        virtual void distributeVertical() override;
        virtual void centerHorizontal() override;
        virtual void centerVertical() override;

        virtual void simulate() override {}

        virtual bool printSupportsFitInPage() const override { return true; }
        virtual void print(QPrinter *printer, bool fitInView) override;
        virtual void exportImage(QPaintDevice &device) override;
        virtual QSizeF documentSize() override;

        virtual bool load(QString *errorMessage = nullptr) override;
        virtual bool save(QString *errorMessage = nullptr) override;

        virtual IView* createView() override;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
        virtual void launchPropertiesDialog() override;
        // End of IDocument interface methods

        GraphicsScene* graphicsScene() const { return m_graphicsScene; }

    private:
        GraphicsScene *m_graphicsScene;

        void alignElements(Qt::Alignment alignment);
    };

    /*!
     * \brief This class represents the text document interface
     * implementation.
     *
     * This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture.
     *
     * This class manages document specific methods like saving and
     * loading, as well as containing the actual document. The document
     * itself is included as a pointer to QTextDocument, that contains all the
     * document specific methods.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa TextContext, TextView
     */
    class TextDocument : public IDocument
    {
        Q_OBJECT

    public:
        explicit TextDocument(QObject *parent = nullptr);
        ~TextDocument() override;

        // IDocument interface methods
        virtual IContext* context() override;

        virtual bool isModified() const override;

        virtual bool canUndo() const override;
        virtual bool canRedo() const override;

        virtual void undo() override;
        virtual void redo() override;

        virtual bool canCut() const override { return true; }
        virtual bool canCopy() const override { return true; }
        virtual bool canPaste() const override { return true; }

        virtual void cut() override;
        virtual void copy() override;
        virtual void paste() override;

        virtual void selectAll() override;

        virtual void enterHierarchy() override;
        virtual void exitHierarchy() override;

        virtual void alignTop() override {}
        virtual void alignBottom() override {}
        virtual void alignLeft() override {}
        virtual void alignRight() override {}
        virtual void distributeHorizontal() override {}
        virtual void distributeVertical() override {}
        virtual void centerHorizontal() override {}
        virtual void centerVertical() override {}

        virtual void simulate() override;

        virtual bool printSupportsFitInPage() const override { return false; }
        virtual void print(QPrinter *printer, bool fitInView) override;
        virtual void exportImage(QPaintDevice &) override {}
        virtual QSizeF documentSize() override;

        virtual bool load(QString *errorMessage = nullptr) override;
        virtual bool save(QString *errorMessage = nullptr) override;

        virtual IView* createView() override;

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *) override {}
        virtual void launchPropertiesDialog() override {}
        // End of IDocument interface methods

        QTextDocument* textDocument() const { return m_textDocument; }

        void pasteTemplate(const QString& text);

    private Q_SLOTS:
        void onContentsChanged();
        void simulationReady(int error);
        void simulationLog(int error);

    private:
        bool simulationErrorStatus; //! This variable is used in multistep simulations (eg. verilog) to avoid opening previously generated waveforms
        TextEdit* activeTextEdit();
        QTextDocument *m_textDocument;
    };

} // namespace Caneda

#endif //CANEDA_IDOCUMENT_H
