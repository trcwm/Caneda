/***************************************************************************
 * Copyright (C) 2006 Gopala Krishna A <krishna.ggk@gmail.com>             *
 * Copyright (C) 2008 Bastien Roucaries <roucaries.bastien@gmail.com>      *
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

#include "graphicsscene.h"

#include "actionmanager.h"
#include "component.h"
#include "documentviewmanager.h"
#include "ellipsearc.h"
#include "graphicsview.h"
#include "graphictextdialog.h"
#include "idocument.h"
#include "iview.h"
#include "portsymbol.h"
#include "property.h"
#include "settings.h"
#include "wire.h"
#include "xmlutilities.h"

#include <QApplication>
#include <QClipboard>
#include <QGraphicsSceneEvent>
#include <QKeySequence>
#include <QMenu>
#include <QPainter>
#include <QShortcutEvent>
#include <QtMath>

namespace Caneda
{
    /*!
     * \brief Constructs a new graphics scene.
     *
     * \param parent Parent of the scene.
     */
    GraphicsScene::GraphicsScene(QObject *parent) :
        QGraphicsScene(QRectF(-2500, -2500, 5000, 5000), parent)
    {
        // Initialize m_mouseAction before anything else to avoid event
        // comparisions with an uninitialized variable.
        m_mouseAction = Normal;

        // Setup spice/electric related scene properties
        m_properties = new PropertyGroup();
        m_properties->setUserPropertiesEnabled(true);
        addItem(m_properties);

        // Setup undo stack
        m_undoStack = new QUndoStack(this);

        // Setup grid
        m_backgroundVisible = true;

        m_areItemsMoving = false;
        m_shortcutsBlocked = false;

        // Wire state machine
        m_currentlyWiring = false;
        m_currentWiringWire = nullptr;

        m_paintingDrawItem = nullptr;
        m_paintingDrawClicks = 0;

        Settings *settings = Settings::instance();
        QColor zoomBandColor =
            settings->currentValue("gui/foregroundColor").value<QColor>();
        m_zoomBand = new QGraphicsRectItem();
        m_zoomBand->setPen(QPen(zoomBandColor));
        zoomBandColor.setAlpha(25);
        m_zoomBand->setBrush(QBrush(zoomBandColor));
        m_zoomBand->hide();
        addItem(m_zoomBand);

        m_zoomBandClicks = 0;

        connect(undoStack(), &QUndoStack::cleanChanged, this, &GraphicsScene::changed);
    }

    /**********************************************************************
     *
     *                             Edit actions
     *
     **********************************************************************/
    //! \brief Cut items
    void GraphicsScene::cutItems(QList<GraphicsItem*> &items)
    {
        copyItems(items);
        deleteItems(items);
    }

    //! \brief Copy items
    void GraphicsScene::copyItems(QList<GraphicsItem*> &items)
    {
        if(items.isEmpty()) {
            return;
        }

        QString clipText;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&clipText);
        writer->setAutoFormatting(true);
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        foreach(GraphicsItem *item, items) {
            item->saveData(writer);
        }

        writer->writeEndDocument();

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(clipText);
    }

    //! \brief Delete items
    void GraphicsScene::deleteItems(QList<GraphicsItem*> &items)
    {
        m_undoStack->beginMacro(tr("Delete items"));
        m_undoStack->push(new RemoveItemsCmd(items, this));
        m_undoStack->endMacro();
    }

    /*!
     * \brief Mirror an item list
     *
     * \param items: item to mirror
     * \param axis: mirror axis
     */
    void GraphicsScene::mirrorItems(QList<GraphicsItem*> &items, const Qt::Axis axis)
    {
        m_undoStack->beginMacro(tr("Mirror items"));
        m_undoStack->push(new MirrorItemsCmd(items, axis, this));
        m_undoStack->endMacro();
    }

    /*!
     * \brief Rotate an item list
     *
     * \param items: item list
     * \param dir: rotation direction
     */
    void GraphicsScene::rotateItems(QList<GraphicsItem*> &items, const Caneda::AngleDirection dir)
    {
        m_undoStack->beginMacro(tr("Rotate items"));
        m_undoStack->push(new RotateItemsCmd(items, dir, this));
        m_undoStack->endMacro();
    }

    /*!
     * \brief Align elements
     *
     * \param alignment: alignement used
     * \todo use smart alignment ie: port alignement
     * \todo string of undo
     * \todo filter wires ???
     */
    bool GraphicsScene::alignElements(const Qt::Alignment alignment)
    {
        QList<QGraphicsItem*> gItems = selectedItems();
        QList<GraphicsItem*> items = filterItems<GraphicsItem>(gItems);

        // Could not align less than two elements
        if(items.size() < 2) {
            return false;
        }

        // Setup undo
        m_undoStack->beginMacro(tr("Align items"));

        // Disconnect
        disconnectItems(items);

        // Compute bounding rectangle
        QRectF rect = items.first()->sceneBoundingRect();
        QList<GraphicsItem*>::iterator it = items.begin()+1;
        while(it != items.end()) {
            rect |= (*it)->sceneBoundingRect();
            ++it;
        }

        it = items.begin();
        while(it != items.end()) {
            if((*it)->type() == GraphicsItem::WireType) {
                ++it;
                continue;
            }

            QRectF itemRect = (*it)->sceneBoundingRect();
            QPointF delta;

            switch(alignment) {
                case Qt::AlignLeft :
                    delta.rx() =  rect.left() - itemRect.left();
                    break;
                case Qt::AlignRight :
                    delta.rx() = rect.right() - itemRect.right();
                    break;
                case Qt::AlignTop :
                    delta.ry() = rect.top() - itemRect.top();
                    break;
                case Qt::AlignBottom :
                    delta.ry() = rect.bottom() - itemRect.bottom();
                    break;
                case Qt::AlignHCenter :
                    delta.rx() = rect.center().x() - itemRect.center().x();
                    break;
                case Qt::AlignVCenter :
                    delta.ry() = rect.center().y() - itemRect.center().y();
                    break;
                case Qt::AlignCenter:
                    delta.rx() = rect.center().x() - itemRect.center().x();
                    delta.ry() = rect.center().y() - itemRect.center().y();
                    break;
                default:
                    break;
            }

            // Move item
            QPointF itemPos = (*it)->pos();
            m_undoStack->push(new MoveItemCmd(*it, itemPos, itemPos + delta));
            ++it;
        }

        // Reconnect items
        connectItems(items);
        splitAndCreateNodes(items);

        // Finish undo
        m_undoStack->endMacro();
        return true;
    }

    /*!
     * \brief Distribute elements
     *
     * Distribute elements ie each element is equally spaced
     *
     * \param orientation: distribute according to orientation
     * \todo filter wire ??? Do not distribute wire ??
     */
    bool GraphicsScene::distributeElements(const Qt::Orientation orientation)
    {
        QList<QGraphicsItem*> gItems = selectedItems();
        QList<GraphicsItem*> items = filterItems<GraphicsItem>(gItems);

        // Could not distribute single items
        if(items.size() < 2) {
            return false;
        }

        if(orientation == Qt::Horizontal) {
            distributeElementsHorizontally(items);
        }
        else {
            distributeElementsVertically(items);
        }
        return true;
    }

    /*!
     * \brief Distribute elements horizontally
     *
     * This method distributes elements horizontally. While all elements are
     * distributed, wires are filtered because they need special care, wires
     * do not have a single x and y coordinate (think of several segments of
     * wires which form single path between two components), therefore their
     * distribution would need a separate check for these kind of segments.
     *
     * \param items: items to distribute
     */
    void GraphicsScene::distributeElementsHorizontally(QList<GraphicsItem*> items)
    {
        m_undoStack->beginMacro(tr("Distribute items"));

        disconnectItems(items);

        // Sort items
        std::sort(items.begin(), items.end(), pointCmpFunction_X);
        qreal x1 = items.first()->pos().x();
        qreal x2 = items.last()->pos().x();

        // Compute steps
        qreal dx = (x2 - x1) / (items.size() - 1);
        qreal x = x1;

        foreach(GraphicsItem *item, items) {
            if(item->type() == GraphicsItem::WireType) {
                continue;
            }

            // Compute the new item position
            QPointF newPos = item->pos();
            newPos.setX(x);
            x += dx;

            // Move the item to the new position
            m_undoStack->push(new MoveItemCmd(item, item->pos(), newPos));
        }

        connectItems(items);
        splitAndCreateNodes(items);

        m_undoStack->endMacro();
    }

    /*!
     * \brief Distribute elements vertically
     *
     * \copydoc distributeElementsHorizontally()
     */
    void GraphicsScene::distributeElementsVertically(QList<GraphicsItem*> items)
    {
        m_undoStack->beginMacro(tr("Distribute items"));

        disconnectItems(items);

        // Sort items
        std::sort(items.begin(), items.end(), pointCmpFunction_Y);
        qreal y1 = items.first()->pos().y();
        qreal y2 = items.last()->pos().y();

        // Compute steps
        qreal dy = (y2 - y1) / (items.size() - 1);
        qreal y = y1;

        foreach(GraphicsItem *item, items) {
            if(item->type() == GraphicsItem::WireType) {
                continue;
            }

            // Compute the new item position
            QPointF newPos = item->pos();
            newPos.setY(y);
            y += dy;

            // Move the item to the new position
            m_undoStack->push(new MoveItemCmd(item, item->pos(), newPos));
        }

        connectItems(items);
        splitAndCreateNodes(items);

        m_undoStack->endMacro();
    }

    /**********************************************************************
     *
     *                      Document properties
     *
     **********************************************************************/
    /*!
     * \brief Change the background color visibility.
     *
     * \param visible Set true of false to show or hide the background color.
     */
    void GraphicsScene::setBackgroundVisible(bool visible)
    {
        m_backgroundVisible = visible;
        update();
    }

    /*!
     * \brief Prints the current scene to device
     *
     * The device to print the scene on can be a physical printer,
     * a postscript (ps) file or a portable document format (pdf)
     * file.
     */
    void GraphicsScene::print(QPrinter *printer, bool fitInView)
    {
        QPainter p(printer);
        p.setRenderHints(Caneda::DefaulRenderHints);

        const bool viewGridStatus = Settings::instance()->currentValue("gui/gridVisible").value<bool>();
        Settings::instance()->setCurrentValue("gui/gridVisible", false);

        const QRectF diagramRect = itemsBoundingRect();

        if(fitInView) {
            render(&p, QRectF(), diagramRect, Qt::KeepAspectRatio);
        }
        else {
            //Printing on one or more pages
            QRectF printedArea = printer->pageLayout().fullRect();

            const int horizontalPages =
                qCeil(diagramRect.width() / printedArea.width());
            const int verticalPages =
                qCeil(diagramRect.height() / printedArea.height());

            QList<QRectF> pagesToPrint;

            //The schematic is printed on a grid of sheets running from top-bottom, left-right.
            qreal yOffset = 0;
            for(int y = 0; y < verticalPages; ++y) {
                //Runs through the sheets of the line
                qreal xOffset = 0;
                for(int x = 0; x < horizontalPages; ++x) {
                    const qreal width = qMin(printedArea.width(), diagramRect.width() - xOffset);
                    const qreal height = qMin(printedArea.height(), diagramRect.height() - yOffset);
                    pagesToPrint << QRectF(xOffset, yOffset, width, height);
                    xOffset += printedArea.width();
                }

                yOffset += printedArea.height();
            }

            for (int i = 0; i < pagesToPrint.size(); ++i) {
                const QRectF rect = pagesToPrint.at(i);
                render(&p,
                       rect.translated(-rect.topLeft()), // dest - topleft at (0, 0)
                       rect.translated(diagramRect.topLeft()), // src
                       Qt::KeepAspectRatio);

                if(i != (pagesToPrint.size() - 1)) {
                    printer->newPage();
                }
            }
        }

        Settings::instance()->setCurrentValue("gui/gridVisible", viewGridStatus);
    }

    /*!
     * \brief Export the scene to an image.
     *
     * This method exports the scene to a user selected image. This method is
     * used in the ExportDialog class, to generate the image itself into a
     * QPaintDevice. The image will be later saved by the ExportImage class.
     *
     * The image itself can be a raster image (bmp, png, etc) or a vector image
     * (svg). The desired size of the destination (final) image must be set in
     * the QPaintDevice where the image is to be rendered. This size can be a
     * 1:1 ratio or any other size.
     *
     * \param pix QPaintDevice where the image is to be rendered
     * \return bool True on success, false otherwise
     * \sa ExportDialog, IDocument::exportImage()
     */
    bool GraphicsScene::exportImage(QPaintDevice &pix)
    {
        // Calculate the source area
        QRectF source_area = itemsBoundingRect();

        // Make the source_area a little bit bigger that dest_area to avoid
        // expanding the image due to floating point precision (this is useful
        // in svg images to avoid generating a raster, non-expandable image)
        source_area.setBottom(source_area.bottom()+1);
        source_area.setRight(source_area.right()+1);

        // Calculate the destination area, acording to the user settings
        QRectF dest_area = QRectF(0, 0, pix.width(), pix.height());

        // Prepare the device
        QPainter p;
        if(!p.begin(&pix)) {
            return(false);
        }

        // Deselect the elements
        QList<QGraphicsItem *> selected_elmts = selectedItems();
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(false);
        }

        // Perform the rendering itself (without background for svg images)
        // As the size is specified, there is no need to keep the aspect ratio
        // (it will be kept if the dimensions of the source and destination areas
        // are proportional.
        setBackgroundVisible(false);
        render(&p, dest_area, source_area, Qt::IgnoreAspectRatio);
        setBackgroundVisible(true);
        p.end();

        // Restore the selected items
        foreach(QGraphicsItem *qgi, selected_elmts) {
            qgi->setSelected(true);
        }

        return(true);
    }

    /**********************************************************************
     *
     *                             Mouse actions
     *
     **********************************************************************/
    /*!
     * \brief Set mouse action
     * This method takes care to disable the shortcuts while items are being added
     * to the scene thus preventing side effects. It also sets the appropriate
     * drag mode for all the views associated with this scene.
     * Finally the state variables are reset.
     *
     * \param MouseAction: mouse action to set
     */
    void GraphicsScene::setMouseAction(const Caneda::MouseAction action)
    {
        if(m_mouseAction == action) {
            return;
        }

        // Remove the shortcut blocking if the current action uptil now was InsertItems
        if(m_mouseAction == InsertingItems) {
            blockShortcuts(false);
        }

        // Blocks shortcut if the new action to be set is InsertingItems
        if(action == InsertingItems) {
            blockShortcuts(true);
        }

        m_areItemsMoving = false;
        m_mouseAction = action;

        emit mouseActionChanged(m_mouseAction);

        resetState();

        //! \todo Implemement this for all mouse actions
    }

    /*!
     * \brief Starts insertItem mode.
     *
     * This is the mode which is used while pasting components or inserting
     * components after selecting it from the sidebar. This initiates the process
     * by filling the internal m_insertibles list whose contents will be moved on
     * mouse events.
     * Meanwhile it also prepares for this process by hiding component's properties
     * which should not be shown while responding to mouse events in this mode.
     *
     * \todo create a insert canedacomponents property in order to avoid ugly cast
     * \todo gpk: why two loop??
     *
     * \note Follow up for the above question:
     * Actually there are 3 loops involved here one encapsulated in centerOfItems
     * method.
     * The first loop prepares the items for insertion by either hiding/showing
     * based on cursor position.
     * Then we have to calculate center of these items with respect to which the
     * items have to be moved. (encapsulated in centerOfItems method)
     * Finally, the third loop actually moves the items.
     * Now the second implicit loop is very much required to run completely as
     * we have to parse each item's bounding rect to calcuate final center.
     * So best approach would be to call centerOfItems first to find delta.
     * Then combine the first and the third loop.
     * Bastein can you look into that ?
     *
     * \note Regarding ugly cast:
     * I think a virtual member function - prepareForInsertion() should be added
     * to GraphicsItem which does nothing. Then classes like component can specialize
     * this method to do necessary operation like hiding properties.
     * Then in the loop, there is no need for cast. Just call that prepare method
     * on all items.
     */
    void GraphicsScene::beginInsertingItems(const QList<GraphicsItem*> &items)
    {
        Q_ASSERT(m_mouseAction == Caneda::InsertingItems);

        // Delete all previous insertibles
        qDeleteAll(m_insertibles);
        // Add to insert list
        m_insertibles = items;

        // Add items
        foreach(GraphicsItem *item, m_insertibles) {
            // Set the item as selected
            item->setSelected(true);
            // Hide all items here, they are made visible in ::insertingItemsEvent
            item->hide();
            // Replace by item->prepareForInsertion()
            if(item->type() == GraphicsItem::ComponentType) {
                Component *comp = canedaitem_cast<Component*>(item);
                comp->properties()->hide();
            }
            // Finally add the item to the scene
            addItem(item);
        }
    }

    /*!
     * \brief Starts beginPaintingDraw mode.
     *
     * This is the mode which is used while inserting painting items.
     */
    void GraphicsScene::beginPaintingDraw(Painting *item)
    {
        Q_ASSERT(m_mouseAction == Caneda::PaintingDrawEvent);

        m_paintingDrawClicks = 0;
        delete m_paintingDrawItem;
        m_paintingDrawItem = item->copy();
    }

    /**********************************************************************
     *
     *                    Connect/disconnect methods
     *
     **********************************************************************/
    /*!
     * \brief Calculates the center of the items given as a parameter.
     *
     * It actually unites the boundingRect of the items sent as parameters
     * and then returns the center of the united rectangle. This center may be
     * used as a reference point for several actions, for example, rotation,
     * mirroring, and copy/paste/inserting items on the scene.
     *
     * \param items The items which geometric center has to be calculated.
     * \return The geometric center of the items.
     */
    QPointF GraphicsScene::centerOfItems(const QList<GraphicsItem*> &items)
    {
        QRectF rect = items.isEmpty() ? QRectF() :
            items.first()->sceneBoundingRect();

        foreach(GraphicsItem *item, items) {
            rect |= item->sceneBoundingRect();
        }

        return rect.center();
    }

    /*!
     * \brief Check for overlapping ports around the scene, and connect the
     * coinciding ports.
     *
     * This method checks for overlapping ports around the scene, and connects
     * the coinciding ports. Although previously this method was included in
     * the GraphicsItem class, later was moved to GraphicsScene to give more
     * flexibility and to avoid infinite recursions when calling this method
     * from inside a newly created or deleted item.
     *
     * \param item: items to connect
     *
     * \sa splitAndCreateNodes()
     */
    void GraphicsScene::connectItems(GraphicsItem *item)
    {
        // Find existing intersecting ports and connect
        foreach(Port *port, item->ports()) {
            Port *other = port->findCoincidingPort();
            if(other) {
                port->connectTo(other);
            }
        }
    }

    //! \copydoc connectItems(GraphicsItem *item)
    void GraphicsScene::connectItems(QList<GraphicsItem*> &items)
    {
        // Check and connect each item
        foreach (GraphicsItem *item, items) {
            connectItems(item);
        }
    }

    /*!
     * \brief Disconnect an item from any wire or other components
     *
     * \param item: item to disconnect
     */
    void GraphicsScene::disconnectItems(GraphicsItem *item)
    {
        QList<Port*> ports = item->ports();
        foreach(Port *p, ports) {
            p->disconnect();
        }
    }

    //! \copydoc disconnectItems(GraphicsItem *item)
    void GraphicsScene::disconnectItems(QList<GraphicsItem*> &items)
    {
        foreach(GraphicsItem *item, items) {
            disconnectItems(item);
        }
    }

    /*!
     * \brief Search wire collisions and if found split the wire.
     *
     * This method searches for wire collisions, and if a collision is present,
     * splits the wire in two, creating a new node. This is done, for example,
     * when wiring the schematic and a wire ends in the middle of another wire.
     * In that case, a connection must be made, thus the need to split the
     * colliding wire.
     *
     * \return Returns true if new node was created.
     *
     * \sa connectItems()
     */
    void GraphicsScene::splitAndCreateNodes(GraphicsItem *item)
    {
        // Check for collisions in each port, otherwise the items intersect
        // but no node should be created.
        foreach(Port *port, item->ports()) {

            // List of wires to delete after collision and creation of new wires
            QList<Wire*> markedForDeletion;

            // Detect all colliding items
            QList<QGraphicsItem*> collisions = port->collidingItems(Qt::IntersectsItemBoundingRect);

            // Filter colliding wires only
            foreach(QGraphicsItem *collidingItem, collisions) {
                Wire* collidingWire = canedaitem_cast<Wire*>(collidingItem);
                if(collidingWire) {

                    // If already connected, the collision is the result of the connection,
                    // otherwise there is a potential new node.
                    bool alreadyConnected = false;
                    foreach(Port *portIterator, item->ports()) {
                        alreadyConnected |=
                                portIterator->isConnectedTo(collidingWire->port1()) ||
                                portIterator->isConnectedTo(collidingWire->port2());
                    }

                    if(!alreadyConnected){
                        // Calculate the start, middle and end points. As the ports are mapped in the parent's
                        // coordinate system, we must calculate the positions (via the mapToScene method) in
                        // the global (scene) coordinate system.
                        QPointF startPoint  = collidingWire->port1()->scenePos();
                        QPointF middlePoint = port->scenePos();
                        QPointF endPoint    = collidingWire->port2()->scenePos();

                        // Mark old wire for deletion. The deletion is performed in a second
                        // stage to avoid referencing null pointers inside the foreach loop.
                        markedForDeletion << collidingWire;

                        // Create two new wires
                        Wire *wire1 = new Wire(startPoint, middlePoint);
                        Wire *wire2 = new Wire(middlePoint, endPoint);
                        addItem(wire1);
                        addItem(wire2);

                        // Create new node (connections to the colliding wire)
                        port->connectTo(wire1->port2());
                        port->connectTo(wire2->port1());

                        wire1->updateGeometry();
                        wire2->updateGeometry();

                        // Restore old wire connections
                        connectItems(wire1);
                        connectItems(wire2);
                    }
                }
            }

            // Delete all wires marked for deletion. The deletion is performed
            // in a second stage to avoid referencing null pointers inside the
            // foreach loop.
            foreach(Wire *w, markedForDeletion) {
                delete w;
            }

            // Clear the list to avoid dereferencing deleted wires
            markedForDeletion.clear();
        }
    }

    //! \copydoc splitAndCreateNodes(GraphicsItem *item)
    void GraphicsScene::splitAndCreateNodes(QList<GraphicsItem *> &items)
    {
        foreach (GraphicsItem *item, items) {
            splitAndCreateNodes(item);
        }
    }

    /**********************************************************************
     *
     *               Spice/electric related scene properties
     *
     **********************************************************************/
    /*!
     * \brief Adds a new property to the scene.
     *
     * This method adds a new property to the scene, by calling the
     * PropertyGroup::addProperty() method of the m_properties stored in this
     * scene.
     *
     * \param property New property to add to the PropertyGroup
     * \sa PropertyGroup::addProperty()
     */
    void GraphicsScene::addProperty(Property property)
    {
        m_properties->addProperty(property.name(), property);
    }

    /*!
     * \brief Draw background of scene including grid
     *
     * \param painter: Where to draw
     * \param rect: Visible area
     * \todo Finish visual representation
     */
    void GraphicsScene::drawBackground(QPainter *painter, const QRectF& rect)
    {
        QPen savedpen = painter->pen();

        // Disable anti aliasing
        painter->setRenderHint(QPainter::Antialiasing, false);

        if(isBackgroundVisible()) {
            const QColor backgroundColor =
                Settings::instance()->currentValue("gui/backgroundColor").value<QColor>();
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(backgroundColor));
            painter->drawRect(rect);
        }

        // Configure pen
        const QColor foregroundColor =
            Settings::instance()->currentValue("gui/foregroundColor").value<QColor>();
        painter->setPen(QPen(foregroundColor, 0));
        painter->setBrush(Qt::NoBrush);

        // Draw origin (if visible in the view)
        if(rect.contains(QPointF(0, 0))) {
            painter->drawLine(QLineF(-3.0, 0.0, 3.0, 0.0));
            painter->drawLine(QLineF(0.0, -3.0, 0.0, 3.0));
        }

        // Draw grid
        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {

            int drawingGridWidth = Caneda::DefaultGridSpace;
            int drawingGridHeight = Caneda::DefaultGridSpace;

            //Make grid size display dinamic, depending on zoom level
            DocumentViewManager *manager = DocumentViewManager::instance();
            IView *v = manager->currentView();
            GraphicsView *sv = qobject_cast<GraphicsView*>(v->toWidget());

            if(sv) {
                if(sv->currentZoom() < 1) {
                    // While drawing, choose spacing to be multiple times the actual grid size.
                    if(sv->currentZoom() > 0.5) {
                        drawingGridWidth *= 4;
                        drawingGridHeight *= 4;
                    }
                    else {
                        drawingGridWidth *= 16;
                        drawingGridHeight *= 16;
                    }
                }
            }

            // Extrema grid points
            int left = int(rect.left()) - (int(rect.left()) % drawingGridWidth);
            int top = int(rect.top()) - (int(rect.top()) % drawingGridHeight);
            int right = int(rect.right()) - (int(rect.right()) % drawingGridWidth);
            int bottom = int(rect.bottom()) - (int(rect.bottom()) % drawingGridHeight);
            int x, y;

            // Draw grid
            painter->setBrush(Qt::NoBrush);
            for(x = left; x <= right; x += drawingGridWidth) {
                for(y = top; y <=bottom; y += drawingGridHeight) {
                    painter->drawPoint(QPointF(x, y));
                }
            }
        }

        // Restore painter
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(savedpen);
    }

    /**********************************************************************
     *
     *                       Custom event handlers
     *
     **********************************************************************/
    /*!
     * Handle some events at lower level. This callback is called before the
     * specialized event handler methods (like mousePressEvent) are called.
     *
     * Here this callback is mainly reimplemented to handle the QEvent::Enter and
     * QEvent::Leave event while the current mouse actions is InsertingItems.
     * When the mouse cursor goes out of the scene, this hides the items to be inserted
     * and the items are shown back once the cursor enters the scene.
     * This actually is used to optimize by not causing much changes on scene when
     * cursor is moved outside the scene.
     * Hint: Hidden items don't result in any changes to the scene's states.
     */
    bool GraphicsScene::event(QEvent *event)
    {
        if(m_mouseAction == InsertingItems) {
            if(event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
                bool visible = (event->type() == QEvent::Enter);
                foreach(GraphicsItem *item, m_insertibles) {
                    item->setVisible(visible);
                }
            }
        }

        return QGraphicsScene::event(event);
    }

    /*!
     * \brief Event called when mouse is pressed
     */
    void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        lastPos = smartNearingGridPoint(event->scenePos());

        // This is not to lose grid snaping when moving objects
        event->setScenePos(lastPos);
        event->setPos(lastPos);

        sendMouseActionEvent(event);
    }

    /*!
     * \brief Mouse move event
     */
    void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        QPointF point = smartNearingGridPoint(event->scenePos());
        if(point == lastPos) {
            event->accept();
            return;
        }

        // Implement grid snap by changing event parameters with new grid position
        event->setScenePos(point);
        event->setPos(point);
        event->setLastScenePos(lastPos);
        event->setLastPos(lastPos);

        // Now cache this point for next move
        lastPos = point;

        sendMouseActionEvent(event);
    }

    /*!
     * \brief Release mouse event
     */
    void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        sendMouseActionEvent(event);
    }

    /*!
     * \brief Mouse double click event
     *
     * Encapsulates the mouseDoubleClickEvent as one of MouseAction and calls
     * corresponding callback.
     */
    void GraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        sendMouseActionEvent(event);
    }

    void GraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        QGraphicsView *v = static_cast<QGraphicsView *>(event->widget()->parent());
        GraphicsView *sv = qobject_cast<GraphicsView*>(v);
        if(!sv) {
            return;
        }

        if(event->modifiers() & Qt::ControlModifier){

            if(event->delta() > 0) {
                sv->translate(0,50);
            }
            else {
                sv->translate(0,-50);
            }

        }
        else if(event->modifiers() & Qt::ShiftModifier){

            if(event->delta() > 0) {
                sv->translate(-50,0);
            }
            else {
                sv->translate(50,0);
            }
        }
        else{

            sv->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Set transform to zoom into mouse position

            if(event->delta() > 0) {
                sv->zoomIn();
            }
            else {
                sv->zoomOut();
            }

        }

        event->accept();
    }

    /*!
     * \brief Constructs and returns a context menu with the actions
     * corresponding to the selected object.
     */
    void GraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        if(m_mouseAction == Normal) {

            IDocument *document = DocumentViewManager::instance()->currentDocument();

            switch(selectedItems().size()) {
            case 0:
                // Launch the context of the current document
                if (document) {
                    document->contextMenuEvent(event);
                }
                break;

            case 1:
                // Launch the context menu of an item.
                QGraphicsScene::contextMenuEvent(event);
                break;

            default:
                // Launch the context menu of multiple items selected.
                QMenu *menu = new QMenu();
                ActionManager* am = ActionManager::instance();

                menu->addAction(am->actionForName("editCut"));
                menu->addAction(am->actionForName("editCopy"));
                menu->addAction(am->actionForName("editDelete"));

                menu->addSeparator();

                menu->addAction(am->actionForName("editRotate"));
                menu->addAction(am->actionForName("editMirrorX"));
                menu->addAction(am->actionForName("editMirrorY"));

                menu->addSeparator();

                menu->addAction(am->actionForName("centerHor"));
                menu->addAction(am->actionForName("centerVert"));

                menu->addSeparator();

                menu->addAction(am->actionForName("alignTop"));
                menu->addAction(am->actionForName("alignBottom"));
                menu->addAction(am->actionForName("alignLeft"));
                menu->addAction(am->actionForName("alignRight"));

                menu->addSeparator();

                menu->addAction(am->actionForName("distrHor"));
                menu->addAction(am->actionForName("distrVert"));

                menu->addSeparator();

                menu->addAction(am->actionForName("propertiesDialog"));

                menu->exec(event->screenPos());
            }
        }
    }

    /*!
     * \brief Call the appropriate mouseAction event based on the current mouse action
     */
    void GraphicsScene::sendMouseActionEvent(QGraphicsSceneMouseEvent *event)
    {
        switch(m_mouseAction) {
            case Wiring:
                wiringEvent(event);
                break;

            case Deleting:
                deletingEvent(event);
                break;

            case Rotating:
                rotatingEvent(event);
                break;

            case MirroringX:
                mirroringXEvent(event);
                break;

            case MirroringY:
                mirroringYEvent(event);
                break;

            case ZoomingAreaEvent:
                zoomingAreaEvent(event);
                break;

            case PaintingDrawEvent:
                paintingDrawEvent(event);
                break;

            case InsertingItems:
                insertingItemsEvent(event);
                break;

            case Normal:
                normalEvent(event);
                break;
        }
    }

    /*!
     * \brief Handle events other than the specilized mouse actions.
     *
     * This involves moving items in a special way so that wires disconnect
     * from unselected components, and unselected wires change their geometry
     * to accomodate item movements.
     *
     * \sa disconnectDisconnectibles(), processForSpecialMove(),
     * specialMove(), endSpecialMove()
     */
    void GraphicsScene::normalEvent(QGraphicsSceneMouseEvent *event)
    {
        switch(event->type()) {
            case QEvent::GraphicsSceneMousePress:
                {
                    QGraphicsScene::mousePressEvent(event);
                    processForSpecialMove();
                }
                break;

            case QEvent::GraphicsSceneMouseMove:
                {
                    if(!m_areItemsMoving) {
                        if((event->buttons() & Qt::LeftButton) && !selectedItems().isEmpty()) {
                            // Items are selected and we are begining a new move operation
                            m_areItemsMoving = true;
                            m_undoStack->beginMacro(tr("Move items"));

                            disconnectDisconnectibles();
                            QGraphicsScene::mouseMoveEvent(event);
                            specialMove();
                        }
                        else {
                            // No items are selected and we are begining a select operation
                            QGraphicsScene::mouseMoveEvent(event);
                        }
                    }
                    else {
                        // We were already moving items
                        QGraphicsScene::mouseMoveEvent(event);
                        specialMove();
                    }
                }
                break;

            case QEvent::GraphicsSceneMouseRelease:
                {
                    if(m_areItemsMoving) {
                        m_areItemsMoving = false;
                        endSpecialMove();
                        m_undoStack->endMacro();
                    }
                    QGraphicsScene::mouseReleaseEvent(event);
                }
                break;

            case QEvent::GraphicsSceneMouseDoubleClick:
                {
                    if(selectedItems().size() == 0) {

                        IDocument *document = DocumentViewManager::instance()->currentDocument();
                        if (document) {
                            document->launchPropertiesDialog();
                        }

                    }

                    QGraphicsScene::mouseDoubleClickEvent(event);
                }
                break;

            default:
                qDebug() << "GraphicsScene::normalEvent() :  Unknown event type";
        }
    }

    /*!
     * \brief This event corresponds to placing/pasting items on scene.
     *
     * When the mouse is moved without pressing, then feed back of all
     * m_insertibles items moving is done here.
     * On mouse press, these items are placed on the scene and a duplicate is
     * retained to support further placing/insertion/paste.
     */
    void GraphicsScene::insertingItemsEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->type() == QEvent::GraphicsSceneMousePress) {

            if(event->button() == Qt::LeftButton) {

                // First temporarily remove the item from the scene. This item
                // is the one the user is grabbing with the mouse and about to
                // insert into the scene. If this "moving" item is not removed
                // there is a collision, and a temporal connection between its
                // ports is made (as the ports of the inserting items collides
                // with the ports of the new item created.
                clearSelection();
                foreach(GraphicsItem *item, m_insertibles) {
                    removeItem(item);
                }

                // Create a new item and copy the properties of the inserting
                // item.
                m_undoStack->beginMacro(tr("Insert items"));
                foreach(GraphicsItem *item, m_insertibles) {
                    GraphicsItem *copied = item->copy();
                    placeItem(copied, smartNearingGridPoint(item->pos()));
                }
                m_undoStack->endMacro();

                // Re-add the inserting items into the scene, to be able to
                // insert more items of the same kind.
                foreach(GraphicsItem *item, m_insertibles) {
                    addItem(item);
                    item->setSelected(true);
                }

            }
            else if(event->button() == Qt::RightButton) {

                // Rotate
                QPointF rotationCenter = centerOfItems(m_insertibles);

                foreach(GraphicsItem *item, m_insertibles) {
                    item->rotate(Caneda::Clockwise, rotationCenter);
                }

            }

        }
        else if(event->type() == QEvent::GraphicsSceneMouseMove) {

            // Move snapping each item to the grid
            QPointF delta = event->scenePos() - centerOfItems(m_insertibles);

            foreach(GraphicsItem *item, m_insertibles) {
                item->show();
                item->setPos(smartNearingGridPoint(item->pos() + delta));
            }

        }
    }

    void GraphicsScene::paintingDrawEvent(QGraphicsSceneMouseEvent *event)
    {
        if(!m_paintingDrawItem) {
            return;
        }

        EllipseArc *arc = nullptr;
        GraphicText *text = nullptr;
        QPointF dest = event->scenePos();
        dest += m_paintingDrawItem->paintingRect().topLeft();
        dest = smartNearingGridPoint(dest);

        if(m_paintingDrawItem->type() == EllipseArc::Type) {
            arc = static_cast<EllipseArc*>(m_paintingDrawItem);
        }

        if(m_paintingDrawItem->type() == GraphicText::Type) {
            text = static_cast<GraphicText*>(m_paintingDrawItem);
        }


        if(event->type() == QEvent::GraphicsSceneMousePress) {
            clearSelection();
            ++m_paintingDrawClicks;

            // First handle special painting items
            if(arc && m_paintingDrawClicks < 4) {
                if(m_paintingDrawClicks == 1) {
                    arc->setStartAngle(0);
                    arc->setSpanAngle(360);
                    arc->setPos(dest);
                    addItem(arc);
                }
                else if(m_paintingDrawClicks == 2) {
                    arc->setSpanAngle(180);
                }

                return;
            }
            else if(text) {
                Q_ASSERT(m_paintingDrawClicks == 1);

                GraphicTextDialog dialog(text, false);
                if(dialog.exec() == QDialog::Accepted) {
                    // Place the text item
                    placeItem(m_paintingDrawItem, dest);

                    // Make an empty copy of the item for the next item insertion
                    m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
                    m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
                    static_cast<GraphicText*>(m_paintingDrawItem)->setText(QString());
                }

                // This means the text was set through the text dialog
                m_paintingDrawClicks = 0;
                return;
            }

            // This is the generic case
            if(m_paintingDrawClicks == 1) {
                m_paintingDrawItem->setPos(dest);
                addItem(m_paintingDrawItem);
            }
            else {
                m_paintingDrawClicks = 0;

                // Place the painting item
                dest = m_paintingDrawItem->pos();
                placeItem(m_paintingDrawItem, dest);

                // Make an empty copy of the item for the next item insertion
                m_paintingDrawItem = static_cast<Painting*>(m_paintingDrawItem->copy());
                m_paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
            }
        }

        else if(event->type() == QEvent::GraphicsSceneMouseMove) {
            if(arc && m_paintingDrawClicks > 1) {
                QPointF delta = event->scenePos() - arc->scenePos();
                int angle = int(180/M_PI * qAtan2(-delta.y(), delta.x()));

                if(m_paintingDrawClicks == 2) {
                    while(angle < 0) {
                        angle += 360;
                    }
                    arc->setStartAngle(int(angle));
                }

                else if(m_paintingDrawClicks == 3) {
                    int span = angle - arc->startAngle();
                    while(span < 0) {
                        span += 360;
                    }
                    arc->setSpanAngle(span);
                }
            }

            else if(m_paintingDrawClicks == 1) {
                QRectF rect = m_paintingDrawItem->paintingRect();
                const QPointF gridifiedPos = smartNearingGridPoint(event->scenePos());
                rect.setBottomRight(m_paintingDrawItem->mapFromScene(gridifiedPos));
                m_paintingDrawItem->setPaintingRect(rect);
            }
        }
    }

    /*************************************************************
     *
     *          DELETING
     *
     *************************************************************/
    /*!
     * \brief Delete action
     *
     * Delete action: left click delete, right click disconnect item
     */
    void GraphicsScene::deletingEvent(const QGraphicsSceneMouseEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton) {
            return deletingEventLeftMouseClick(event->scenePos());
        }

        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return deletingEventRightMouseClick(event->scenePos());
        }

        return;
    }

    /*!
     * \brief Left button deleting event: delete items
     *
     * \param pos: pos clicked
     */
    void GraphicsScene::deletingEventLeftMouseClick(const QPointF &pos)
    {
        // Create a list of items
        QList<QGraphicsItem*> list = items(pos);

        if(!list.isEmpty()) {
            QList<GraphicsItem*> items = filterItems<GraphicsItem>(list);

            if(!items.isEmpty()) {
                deleteItems(QList<GraphicsItem*>() << items.first());
            }
        }
    }

    /*!
     * \brief Left button deleting event: delete items
     *
     * \param pos: pos clicked
     */
    void GraphicsScene::deletingEventRightMouseClick(const QPointF &pos)
    {
        // Create a list of items
        QList<QGraphicsItem*> list = items(pos);

        if(!list.isEmpty()) {
            QList<GraphicsItem*> items = filterItems<GraphicsItem>(list);

            if(!items.isEmpty()) {
                disconnectItems(QList<GraphicsItem*>() << items.first());
            }
        }
    }

    /*********************************************************************
     *
     *            WIRING
     *
     *********************************************************************/
    //! \brief Wiring event
    void GraphicsScene::wiringEvent(QGraphicsSceneMouseEvent *event)
    {
        QPointF pos = smartNearingGridPoint(event->scenePos());

        // Press mouse event
        if(event->type() == QEvent::GraphicsSceneMousePress)  {
            return wiringEventMouseClick(event, pos);
        }
        // Move mouse event
        else if(event->type() == QEvent::GraphicsSceneMouseMove)  {
            return wiringEventMouseMove(pos);
        }
    }

    /*!
     * \brief Mouse click wire event
     *
     * \param Event: mouse event
     * \param pos: coordinate of mouse action point
     */
    void GraphicsScene::wiringEventMouseClick(const QGraphicsSceneMouseEvent *event, const QPointF &pos)
    {
        // Left click - Add control point
        if((event->buttons() & Qt::LeftButton) == Qt::LeftButton)  {
            return wiringEventLeftMouseClick(pos);
        }
        // Right click - Finish wire
        if((event->buttons() & Qt::RightButton) == Qt::RightButton) {
            return wiringEventRightMouseClick();
        }
    }

    /*!
     * \brief Left mouse click wire event
     *
     * \param pos: coordinate of mouse action point
     */
    void GraphicsScene::wiringEventLeftMouseClick(const QPointF &pos)
    {
        if(!m_currentlyWiring) {
            // Create a new wire
            m_currentWiringWire = new Wire(pos, pos);
            addItem(m_currentWiringWire);
            m_currentlyWiring = true;
            return;
        }

        if(m_currentlyWiring) {
            // Check if port 1 and 2 overlap
            if(m_currentWiringWire->isNull())  {
                return;
            }

            // Connect ports to any coinciding port in the scene
            connectItems(m_currentWiringWire);
            splitAndCreateNodes(m_currentWiringWire);

            if(m_currentWiringWire->port2()->hasAnyConnection()) {
                // If a connection was made, detach current wire and finalize
                m_currentWiringWire = nullptr;
                m_currentlyWiring = false;
            }
            else  {
                // Add a wire segment
                QPointF refPos = m_currentWiringWire->port2()->pos() + m_currentWiringWire->pos();
                m_currentWiringWire = new Wire(refPos, refPos);
                addItem(m_currentWiringWire);
            }

            return;
        }

    }

    //! \brief Right mouse click wire event, ie finish wire event
    void GraphicsScene::wiringEventRightMouseClick()
    {
        if(m_currentlyWiring) {
            // Check if port 1 and 2 overlap
            if(m_currentWiringWire->isNull()) {
                return;
            }

            connectItems(m_currentWiringWire);
            splitAndCreateNodes(m_currentWiringWire);

            // Detach current wire and finalize
            m_currentWiringWire = nullptr;
            m_currentlyWiring = false;

            return;
        }
    }

    /*!
     * \brief Mouse move wire event
     *
     * \param newPos: coordinate of mouse action point
     */
    void GraphicsScene::wiringEventMouseMove(const QPointF &newPos)
    {
        if(m_currentlyWiring) {

            QPointF refPos = m_currentWiringWire->port1()->scenePos();

            if( qAbs(refPos.x()-newPos.x()) > qAbs(refPos.y()-newPos.y()) ) {
                m_currentWiringWire->movePort2(QPointF(newPos.x(), refPos.y()));
            }
            else {
                m_currentWiringWire->movePort2(QPointF(refPos.x(), newPos.y()));
            }

        }
    }

    /******************************************************************
     *
     *                         Rotate Event
     *
     *****************************************************************/
    /*!
     * \brief Rotate item
     * \note right anticlockwise
     */
    void GraphicsScene::rotatingEvent(QGraphicsSceneMouseEvent *event)
    {
        Caneda::AngleDirection angle;

        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        // left == clock wise
        if(event->buttons() == Qt::LeftButton) {
            angle = Caneda::Clockwise;
        }
        // right == anticlock wise
        else if(event->buttons() == Qt::RightButton) {
            angle = Caneda::AntiClockwise;
        }
        // Avoid angle unitialized
        else {
            return;
        }

        // Get items
        QList<QGraphicsItem*> list = items(event->scenePos());
        // Filter item
        QList<GraphicsItem*> items = filterItems<GraphicsItem>(list);
        if(!items.isEmpty()) {
            rotateItems(QList<GraphicsItem*>() << items.first(), angle);
        }
    }

    /**********************************************************************
     *
     *                           Mirror
     *
     **********************************************************************/
    /*!
     * \brief Mirror event
     *
     * \param event: event
     * \param axis: mirror axis
     */
    void GraphicsScene::mirroringEvent(const QGraphicsSceneMouseEvent *event,
            const Qt::Axis axis)
    {
        // Select item and filter items
        QList<QGraphicsItem*> list = items(event->scenePos());
        QList<GraphicsItem*> items = filterItems<GraphicsItem>(list);

        if(!items.isEmpty()) {
            mirrorItems(QList<GraphicsItem*>() << items.first(), axis);
        }
    }

    //! \brief Mirror X event
    void GraphicsScene::mirroringXEvent(const QGraphicsSceneMouseEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if(event->buttons() == Qt::LeftButton) {
            mirroringEvent(event, Qt::XAxis);
        }
    }

    //! \brief Mirror Y event
    void GraphicsScene::mirroringYEvent(const QGraphicsSceneMouseEvent *event)
    {
        if(event->type() != QEvent::GraphicsSceneMousePress) {
            return;
        }

        if(event->buttons() == Qt::LeftButton) {
            mirroringEvent(event, Qt::YAxis);
        }
    }

    /******************************************************************
     *
     *                       Zooming Area Event
     *
     *****************************************************************/
    /*!
     * \brief Zoom in event handles zooming of the view based on mouse signals.
     *
     * If just a point is clicked(mouse press + release) then, an ordinary zoomIn
     * is done (similar to selecting from menu)
     *
     * On the otherhand if mouse is pressed and dragged and then release,
     * corresponding feedback (zoom band) is shown which indiates area that will
     * be zoomed. On mouse release, the area (rect) selected is zoomed.
     */
    void GraphicsScene::zoomingAreaEvent(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsView *view = static_cast<QGraphicsView *>(event->widget()->parent());
        GraphicsView *cView = qobject_cast<GraphicsView*>(view);
        if(!cView) {
            return;
        }

        QPointF dest = smartNearingGridPoint(event->scenePos());

        if(event->type() == QEvent::GraphicsSceneMousePress) {
            clearSelection();
            ++m_zoomBandClicks;

            // This is the generic case
            if(m_zoomBandClicks == 1) {
                m_zoomRect.setRect(dest.x(), dest.y(), 0, 0);
                m_zoomBand->setRect(m_zoomRect.normalized());
                m_zoomBand->show();
            }
            else {
                m_zoomBandClicks = 0;
                m_zoomBand->hide();
                cView->zoomFitRect(m_zoomRect.normalized());
                m_zoomRect.setRect(0, 0, 0, 0);
            }
        }

        else if(event->type() == QEvent::GraphicsSceneMouseMove) {
            if(m_zoomBandClicks == 1) {
                m_zoomRect.setBottomRight(dest);
                m_zoomBand->setRect(m_zoomRect.normalized());
            }
        }
    }

    /**********************************************************************
     *
     *                           Place item
     *
     **********************************************************************/
    /*!
     * \brief Place an item on the scene
     *
     * \param item item to place
     * \param pos position of the item
     * \warning pos is not rounded (grid snapping)
     */
    void GraphicsScene::placeItem(GraphicsItem *item, const QPointF &pos)
    {
        if(item->type() == GraphicsItem::ComponentType) {
            Component *component = canedaitem_cast<Component*>(item);

            int labelSuffix = componentLabelSuffix(component->labelPrefix());
            QString label = QString("%1%2").
                arg(component->labelPrefix()).
                arg(labelSuffix);

            component->setLabel(label);
        }

        m_undoStack->beginMacro(tr("Place items"));
        m_undoStack->push(new InsertItemCmd(item, pos, this));
        m_undoStack->endMacro();
    }

    /*!
     * \brief Returns an appropriate label suffix as 1 and 2 in R1, R2
     *
     * This method walks through all the items on the scene matching the
     * labelprefix and uses the highest of these suffixes + 1 as the new
     * suffix candidate.
     */
    int GraphicsScene::componentLabelSuffix(const QString& prefix) const
    {
        int max = 1;

        foreach(QGraphicsItem *item, items()) {
            Component *comp = canedaitem_cast<Component*>(item);
            if(comp && comp->labelPrefix() == prefix) {
                bool ok;
                int suffix = comp->labelSuffix().toInt(&ok);
                if(ok) {
                    max = qMax(max, suffix+1);
                }
            }
        }

        return max;
    }

    /******************************************************************
     *
     *                   Moving Events
     *
     *****************************************************************/
    /*!
     * \brief Check which items should be moved in a special way, to allow
     * proper wire and component movements.
     *
     * This method decides which tipe of movement each item must perform. In
     * general, moving wires should disconnect from unselected components, and
     * unselected wires should change their geometry to accomodate item
     * movements. This is acomplished by generating two lists:
     *
     * \li A list of items to disconnect
     * \li A list of wires whose geometry must be updated
     *
     * The action of this function is observed, for example, when moving an
     * item (a wire, a component, etc) connected to other components. By
     * processing if the item is a component or a wire and deciding if the
     * items must remain together (in the case of wires or when moving only
     * a component connected to wires) or separated from the connections
     * (when moving a wire away from a component), expected movements are
     * performed.
     *
     * \sa normalEvent(), specialMove()
     */
    void GraphicsScene::processForSpecialMove()
    {
        disconnectibles.clear();
        specialMoveItems.clear();

        foreach(QGraphicsItem *qItem, selectedItems()) {
            GraphicsItem *item = canedaitem_cast<GraphicsItem*>(qItem);

            if(item) {
                // Save item's position for later use in undo/redo.
                item->storePos();

                // Check for disconnections and wire resizing
                foreach(Port *port, item->ports()) {

                    foreach(Port *other, *(port->connections())) {
                        // If the item connected is a component, determine whether it should
                        // be disconnected or not.
                        if(other->parentItem()->type() == GraphicsItem::ComponentType &&
                                !other->parentItem()->isSelected()) {
                            disconnectibles << item;
                        }
                        // If the item connected is a wire, determine whether it should be
                        // resized or moved.
                        if(other->parentItem()->type() == GraphicsItem::WireType &&
                                !other->parentItem()->isSelected()) {
                            specialMoveItems << other->parentItem();
                        }
                        // If the item connected is a port, determine whether it should be
                        // moved or not.
                        if(other->parentItem()->type() == GraphicsItem::PortSymbolType &&
                                !other->parentItem()->isSelected()) {
                            specialMoveItems << other->parentItem();
                        }
                    }

                }
            }
        }
    }

    /*!
     * \brief Move the unselected items in a special way to allow proper wire
     * movements.
     *
     * This method accomodates the geometry of all wires which must be resized
     * due to the current wire movement, but are not selected (and moving)
     * themselves. It also moves some special items which must move along with
     * the selected wires.
     *
     * The action of this function is observed, for example, when moving a wire
     * connected to other wires. Thanks to this function, the connected ports
     * of all wires stay together. If this function was to be removed, after
     * a wire movement action, the connected wires would remain untouched, and
     * a gap would appear between the moved wire and the connected wires (which
     * would remain in their original place).
     *
     * \sa normalEvent(), processForSpecialMove()
     */
    void GraphicsScene::specialMove()
    {
        foreach(GraphicsItem *item, specialMoveItems) {

            // The wires in specialMoveItems are those wires that are not selected
            // but whose geometry must acommodate to the current moving wire.
            if(item->type() == GraphicsItem::WireType) {

                // Check both ports (port1 and port2) of the unselected wire for
                // possible ports movement.
                Wire *wire = canedaitem_cast<Wire*>(item);

                // First check port1
                foreach(Port *other, *(wire->port1()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving wire and this port must copy that port position.
                    if(other->scenePos() != wire->port1()->scenePos()) {
                        wire->movePort1(other->scenePos());
                        break;
                    }
                }

                // Then check port2
                foreach(Port *other, *(wire->port2()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving wire and this port must copy that port position.
                    if(other->scenePos() != wire->port2()->scenePos()) {
                        wire->movePort2(other->scenePos());
                        break;
                    }
                }

            }

            // The ports in specialMoveItems must be moved along the selected
            // (and moving) wires.
            if(item->type() == GraphicsItem::PortSymbolType) {

                PortSymbol *portSymbol = canedaitem_cast<PortSymbol*>(item);

                foreach(Port *other, *(portSymbol->port()->connections())) {
                    // If some of the connected ports has moved, we have found the
                    // moving item and this port must copy that port position.
                    if(other->scenePos() != portSymbol->scenePos()) {
                        portSymbol->setPos(other->scenePos());
                        break;
                    }
                }

            }

        }
    }

    /*!
     * \brief End the special move and finalize wire's segements.
     *
     * This method ends the special move by pushing the necessary UndoCommands
     * relative to position changes of items on a scene. Also finalize wire's
     * segments.
     *
     * \sa normalEvent()
     */
    void GraphicsScene::endSpecialMove()
    {
        foreach(QGraphicsItem *qItem, selectedItems()) {
            GraphicsItem *item = canedaitem_cast<GraphicsItem*>(qItem);

            if(item) {
                m_undoStack->push(new MoveItemCmd(item, item->storedPos(),
                            smartNearingGridPoint(item->pos())));

                connectItems(item);
                splitAndCreateNodes(item);
            }
        }

        specialMoveItems.clear();
        disconnectibles.clear();
    }

    /*!
     * \brief Disconnect the items in the disconnectibles list.
     *
     * This method disconnects the ports in the disconnectibles list, generated
     * by the processForSpecialMove() method. The disconnection should happen
     * when two (or more) components are connected and one of them is clicked
     * and dragged, or when a wire is moved away from a (unselected) component.
     *
     * \sa normalEvent(), processForSpecialMove()
     */
    void GraphicsScene::disconnectDisconnectibles()
    {
        QSet<GraphicsItem*> remove;

        foreach(GraphicsItem *item, disconnectibles) {

            int disconnections = 0;
            foreach(Port *port, item->ports()) {

                foreach(Port *other, *(port->connections())) {
                    if(other->parentItem()->type() == GraphicsItem::ComponentType &&
                            other->parentItem() != item &&
                            !other->parentItem()->isSelected()) {

                        m_undoStack->push(new DisconnectCmd(port, other));
                        ++disconnections;

                        break;
                    }
                }

            }

            if(disconnections) {
                remove << item;
            }
        }

        foreach(GraphicsItem *item, remove) {
            disconnectibles.removeAll(item);
        }
    }

    /*!
     * \brief Reset the state
     *
     * This callback is called when for instance you press esc key
     */
    void GraphicsScene::resetState()
    {
        // Clear focus on any item on this scene.
        setFocusItem(nullptr);
        // Clear selection.
        clearSelection();

        // Clear the list holding items to be pasted/placed on graphics scene.
        qDeleteAll(m_insertibles);
        m_insertibles.clear();

        // If current state is wiring, delete last attempt
        if(m_currentlyWiring){
            Q_ASSERT(m_currentWiringWire != NULL);
            delete m_currentWiringWire;
            m_currentlyWiring = false;
        }

        // Reset drawing item
        delete m_paintingDrawItem;
        m_paintingDrawItem = nullptr;
        m_paintingDrawClicks = 0;

        // Clear zoom
        m_zoomRect = QRectF();
        m_zoomBand->hide();
        m_zoomBandClicks = 0;
    }

    /*!
     * \brief Event filter filter's out some events on the watched object.
     *
     * This filter is used to install on QApplication object to filter our
     * shortcut events.
     * This filter is installed by \a setMouseAction method if the new action
     * is InsertingItems and removed if the new action is different, thus blocking
     * shortcuts on InsertItems and unblocking for other mouse actions
     * \sa GraphicsScene::setMouseAction, GraphicsScene::blockShortcuts
     * \sa QObject::eventFilter
     *
     * \todo Take care if multiple scenes install event filters.
     */
    bool GraphicsScene::eventFilter(QObject *watched, QEvent *event)
    {
        if(event->type() != QEvent::Shortcut && event->type() != QEvent::ShortcutOverride) {
            return QGraphicsScene::eventFilter(watched, event);
        }

        QKeySequence key;

        if(event->type() == QEvent::Shortcut) {
            key = static_cast<QShortcutEvent*>(event)->key();
        }
        else {
            key = static_cast<QKeyEvent*>(event)->key();
        }

        if(key == QKeySequence(Qt::Key_Escape)) {
            return false;
        }
        else {
            return true;
        }
    }

    /*!
     * \brief Blocks/unblocks the shortcuts on the QApplication.
     *
     * \param block True blocks while false unblocks the shortcuts.
     * \sa GraphicsScene::eventFilter
     */
    void GraphicsScene::blockShortcuts(bool block)
    {
        if(block) {
            if(!m_shortcutsBlocked) {
                qApp->installEventFilter(this);
                m_shortcutsBlocked = true;
            }
        }
        else {
            if(m_shortcutsBlocked) {
                qApp->removeEventFilter(this);
                m_shortcutsBlocked = false;
            }
        }
    }

} // namespace Caneda
