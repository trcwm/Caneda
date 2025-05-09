/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "painting.h"

#include "settings.h"

#include "arrow.h"
#include "ellipse.h"
#include "ellipsearc.h"
#include "graphicline.h"
#include "graphicsscene.h"
#include "graphictext.h"
#include "rectangle.h"

#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs a painting item with default pen and default brush.
     *
     * \param parent Parent of the Painting item.
     */
    Painting::Painting(QGraphicsItem *parent) :
        GraphicsItem(parent),
        m_brush(Qt::NoBrush),
        m_resizeHandles(Caneda::NoHandle),
        m_activeHandle(Caneda::NoHandle)
    {
        Settings *settings = Settings::instance();
        m_pen = QPen(settings->currentValue("gui/foregroundColor").value<QColor>());

        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);
    }

    /*!
     * \brief Returns a pointer to new Painting object created appropriately
     * according to name.
     */
    Painting* Painting::fromName(const QString& name)
    {
        static QRectF rect(-30, -30, 90, 60);

        // Check if name begins with capital letter and if so use the following.
        // This happens when painting is placed by selecting in sidebar.
        if(name.at(0).isUpper()) {
            if(name == QObject::tr("Line")) {
                return new GraphicLine(QLineF(rect.bottomLeft(), rect.topRight()));
            }
            else if(name == QObject::tr("Arrow")) {
                return new Arrow(QLineF(rect.bottomLeft(), rect.topRight()));
            }
            else if(name == QObject::tr("Ellipse")) {
                return new Ellipse(rect);
            }
            else if(name == QObject::tr("Rectangle")) {
                return new Rectangle(rect);
            }
            else if(name == QObject::tr("Elliptic Arc")) {
                return new EllipseArc(rect, 100, 300);
            }
            else if(name == QObject::tr("Text")) {
                return new GraphicText;
            }

        }

        // This is true usually when painting is being read from xml file.
        // To allow i18n interoperation, saved tag should not depend on language settings.
        else {
            if(name == QLatin1String("line")) {
                return new GraphicLine(QLineF(rect.bottomLeft(), rect.topRight()));
            }
            else if(name == QLatin1String("arrow")) {
                return new Arrow(QLineF(rect.bottomLeft(), rect.topRight()));
            }
            else if(name == QLatin1String("ellipse")) {
                return new Ellipse(rect);
            }
            else if(name == QLatin1String("rectangle")) {
                return new Rectangle(rect);
            }
            else if(name == QLatin1String("ellipseArc")) {
                return new EllipseArc(rect, 100, 300);
            }
            else if(name == QLatin1String("text")) {
                return new GraphicText;
            }

        }
        return nullptr;
    }

    /*!
     * \brief Sets the painting rect to \a rect.
     *
     * \copydoc Painting::m_paintingRect
     */
    void Painting::setPaintingRect(const QRectF& rect)
    {
        if(rect == m_paintingRect) {
            return;
        }

        prepareGeometryChange();

        m_paintingRect = rect;
        geometryChange();

        adjustGeometry();
    }

    /*!
     * \brief Returns shape of item for given painting rect.
     *
     * Subclasses can use this to customize their shapes.
     */
    QPainterPath Painting::shapeForRect(const QRectF& rect) const
    {
        QPainterPath path;
        path.addRect(rect);
        return path;
    }

    //! \brief Sets item's pen to \a _pen.
    void Painting::setPen(const QPen& _pen)
    {
        if(m_pen == _pen) {
            return;
        }

        prepareGeometryChange();
        m_pen = _pen;

        adjustGeometry();
    }

    //! \brief Sets item's brush to \a _brush.
    void Painting::setBrush(const QBrush& _brush)
    {
        if(m_brush == _brush) {
            return;
        }

        prepareGeometryChange();
        m_brush = _brush;

        //no need to adjust geometry as brush doesn't alter geometry
        update();
    }

    /*!
     * \brief Draws the handles if the item is selected.
     *
     * Subclasses should reimplement to draw itself with using paintingRect
     * as hint to draw. The subclassed paint method should also call this
     * base method in the end to get the resize handles drawn.
     */
    void Painting::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *)
    {
        if(option->state & QStyle::State_Selected) {
            drawResizeHandles(m_resizeHandles, m_paintingRect, painter);
        }
    }

    //! \brief Indicate the resize handles to be shown.
    void Painting::setResizeHandles(Caneda::ResizeHandles handles)
    {
        if(m_resizeHandles == handles) {
            return;
        }

        prepareGeometryChange();
        m_resizeHandles = handles;

        adjustGeometry();
    }

    //! \copydoc GraphicsItem::copyDataTo()
    void Painting::copyDataTo(Painting *painting) const
    {
        painting->setPen(pen());
        painting->setBrush(brush());
        GraphicsItem::copyDataTo(painting);
    }

    //! Adjust geometry of item to accommodate resize handles.
    void Painting::adjustGeometry()
    {
        // First obtain the shape of the painting item
        QRectF boundRect = m_paintingRect;
        QPainterPath _shape = shapeForRect(m_paintingRect);

        // Now determine how to adjust the bounding rect based on the resize
        // handles being used.
        if(m_resizeHandles.testFlag(Caneda::TopLeftHandle)) {
            QRectF rect = handleRect().translated(m_paintingRect.topLeft());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        if(m_resizeHandles.testFlag(Caneda::TopRightHandle)) {
            QRectF rect = handleRect().translated(m_paintingRect.topRight());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        if(m_resizeHandles.testFlag(Caneda::BottomLeftHandle)) {
            QRectF rect = handleRect().translated(m_paintingRect.bottomLeft());
            boundRect |= rect;
            _shape.addRect(rect);

        }

        if(m_resizeHandles.testFlag(Caneda::BottomRightHandle)) {
            QRectF rect = handleRect().translated(m_paintingRect.bottomRight());
            boundRect |= rect;
            _shape.addRect(rect);
        }

        // Extend the shape to allow for a thick selection band. This is
        // specially usefull when trying to select diagonal lines or arrows.
        QPainterPathStroker stroker;
        stroker.setWidth(10);
        _shape = stroker.createStroke(_shape);

        setShapeAndBoundRect(_shape, boundRect, m_pen.widthF());
        update();
    }

    //! Takes care of handle resizing on mouse press event.
    void Painting::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        m_activeHandle = Caneda::NoHandle;

        if(event->buttons().testFlag(Qt::LeftButton)) {
            m_activeHandle = handleHitTest(event->pos(), m_resizeHandles, m_paintingRect);
        }

        //call base method to get move behaviour as no handle is pressed
        if(m_activeHandle == Caneda::NoHandle) {
            GraphicsItem::mousePressEvent(event);
        }
        else {
            storePaintingRect();
        }
    }

    //! Takes care of handle resizing on mouse move event.
    void Painting::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if(m_activeHandle == Caneda::NoHandle) {
            GraphicsItem::mouseMoveEvent(event);
            Q_ASSERT(scene()->mouseGrabberItem() == this);
            return;
        }

        if(event->buttons().testFlag(Qt::LeftButton)) {
            QRectF rect = m_paintingRect;
            QPointF point = event->pos();

            switch(m_activeHandle) {
            case Caneda::TopLeftHandle:
                rect.setTopLeft(point);
                break;

            case Caneda::TopRightHandle:
                rect.setTopRight(point);
                break;

            case Caneda::BottomLeftHandle:
                rect.setBottomLeft(point);
                break;

            case Caneda::BottomRightHandle:
                rect.setBottomRight(point);
                break;

            case Caneda::NoHandle:
                break;
            }

            setPaintingRect(rect);
        }
    }

    /*!
     * \brief Takes care of resizing upon mouse release event.
     *
     * This method takes care of creating the necessary undo commands
     * upon mouse release event. In this way, when resizing this item
     * (painting) the undo commands are passed to the undostack making
     * them available should an undo/redo command be issued.
     */
    void Painting::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        GraphicsItem::mouseReleaseEvent(event);
        if(m_activeHandle != Caneda::NoHandle && m_paintingRect != m_store) {
            ChangePaintingRectCmd *cmd = new ChangePaintingRectCmd(this, storedPaintingRect(), m_paintingRect);
            GraphicsScene *scene = qobject_cast<GraphicsScene*>(this->scene());
            scene->undoStack()->push(cmd);
        }
        m_activeHandle = Caneda::NoHandle;
    }

    /*!
     * \brief Draw resize handle for painting items.
     *
     * \param centrePos The center point of the handle.
     * \param painter   The painter used to draw the handle.
     */
    void Painting::drawResizeHandle(const QPointF &centrePos, QPainter *painter)
    {
        QPen savedPen = painter->pen();
        QBrush savedBrush = painter->brush();

        Settings *settings = Settings::instance();
        painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>()));
        painter->setBrush(Qt::NoBrush);

        // handleRect is defined as QRectF(-w/2, -h/2, w, h)
        painter->drawRect(handleRect().translated(centrePos));

        painter->setPen(savedPen);
        painter->setBrush(savedBrush);
    }

    /*!
     * \brief Draw four resize handles along the corners of the rectangle passed.
     *
     * \param handles  The handle areas where handles need to be drawn.
     * \param rect     The rectangle around which resize handles are to be drawn.
     * \param painter  The painter used to draw resize handles.
     */
    void Painting::drawResizeHandles(Caneda::ResizeHandles handles, const QRectF& rect, QPainter *painter)
    {
        if(handles.testFlag(Caneda::TopLeftHandle)) {
            drawResizeHandle(rect.topLeft(), painter);
        }

        if(handles.testFlag(Caneda::TopRightHandle)) {
            drawResizeHandle(rect.topRight(), painter);
        }

        if(handles.testFlag(Caneda::BottomLeftHandle)) {
            drawResizeHandle(rect.bottomLeft(), painter);
        }

        if(handles.testFlag(Caneda::BottomRightHandle)) {
            drawResizeHandle(rect.bottomRight(), painter);
        }
    }

    /*!
     * \brief Returns the resize handle corresponding to a given point.
     *
     * \param point   The point to be tested for collision with handles.
     * \param handles The mask indicating the handle areas to be tested.
     * \param rect    The rectangle around which resize handles are to be tested.
     */
    Caneda::ResizeHandle Painting::handleHitTest(const QPointF& point, Caneda::ResizeHandles handles,
            const QRectF& rect)
    {
        if(handles == Caneda::NoHandle) {
            return Caneda::NoHandle;
        }

        if(handles.testFlag(Caneda::TopLeftHandle)) {
            if(handleRect().translated(rect.topLeft()).contains(point)) {
                return Caneda::TopLeftHandle;
            }
        }

        if(handles.testFlag(Caneda::TopRightHandle)) {
            if(handleRect().translated(rect.topRight()).contains(point)) {
                return Caneda::TopRightHandle;
            }
        }

        if(handles.testFlag(Caneda::BottomLeftHandle)){
            if(handleRect().translated(rect.bottomLeft()).contains(point)) {
                return Caneda::BottomLeftHandle;
            }
        }

        if(handles.testFlag(Caneda::BottomRightHandle)){
            if(handleRect().translated(rect.bottomRight()).contains(point)) {
                return Caneda::BottomRightHandle;
            }
        }

        return Caneda::NoHandle;
    }

} // namespace Caneda
