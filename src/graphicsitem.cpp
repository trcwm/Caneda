/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "graphicsitem.h"

#include "actionmanager.h"

#include <QGraphicsSceneEvent>
#include <QMenu>

namespace Caneda
{
    /*!
     * \brief Constructs a new graphics item.
     *
     * \param parent Parent of the item.
     */
    GraphicsItem::GraphicsItem(QGraphicsItem *parent) :
        QGraphicsItem(parent),
        m_boundingRect(0, 0, 0, 0)
    {
        m_shape.addRect(m_boundingRect);
        setFlag(ItemSendsGeometryChanges, true);
        setFlag(ItemSendsScenePositionChanges, true);
    }

    /*!
     * \brief Rotate item by 90 degrees around a pivot point
     *
     * This method rotates an item around a pivot point, allowing for complex
     * rotations for example when having multiple items selected and the
     * rotation should be around the selection center.
     *
     * To achieve the rotation, first the item is rotated around its center
     * by using the rotate(Caneda::AngleDirection dir) method. Afterwards,
     * its new position is calculated using an exploration point. This point
     * is rotated around the pivot point using a transformation, and its final
     * position is used as the destination position of the original item.
     *
     * Although the transformation could be applied directly to the original
     * item, that led to strange behaviour, for example while moving the item
     * after the rotation, the item would change its position when dropped. As
     * a solution, the temporal exploration point is used to calculate its
     * final position as explained above.
     *
     * \param dir Direction of rotation
     * \param pivotPoint Point around which the rotation is performed
     */
    void GraphicsItem::rotate(AngleDirection dir, QPointF pivotPoint)
    {
        // Rotate item
        qreal angle = (dir == Caneda::Clockwise) ? 90.0 : -90.0;
        setRotation(rotation() + angle);

        // Move to the rotated position around the pivot point
        QTransform transform;
        transform.rotate(dir == Caneda::Clockwise ? 90 : -90);

        QPointF newPos = pos() - pivotPoint;
        newPos = transform.map(newPos);
        newPos = newPos + pivotPoint;

        newPos = smartNearingGridPoint(newPos);
        setPos(newPos);
    }

    /*!
     * \brief Mirror item according to an axis around a pivot point
     *
     * This method mirrors an item around a pivot point, allowing for complex
     * mirrors for example when having multiple items selected and the
     * mirroring should be around the selection center.
     *
     * \param axis Mirror axis
     * \param pivotPoint Point around which the mirror is performed
     */
    void GraphicsItem::mirror(Qt::Axis axis, QPointF pivotPoint)
    {
        // Mirror item
        if(axis == Qt::XAxis) {
            setTransform(QTransform::fromScale(1.0, -1.0), true);
        }
        else {
            setTransform(QTransform::fromScale(-1.0, 1.0), true);
        }

        // Move to the mirrored position around the pivot point
        QPointF newPos = pos();

        if(axis == Qt::XAxis) {
            newPos.setY(2*pivotPoint.y()-newPos.y()); // pivotPoint.y() - (pos().y() - pivotPoint.y())
        }
        else {
            newPos.setX(2*pivotPoint.x()-newPos.x()); // pivotPoint.x() - (pos().x() - pivotPoint.x())
        }

        newPos = smartNearingGridPoint(newPos);
        setPos(newPos);
    }

    /*!
     * \brief Stores the item's current position for later usage.
     *
     * This method stores the current item position and is required for
     * recovering the last known position while handling the undo/redo
     * commands.
     *
     * \sa storedPos()
     */
    void GraphicsItem::storePos()
    {
        m_store = pos();
    }

    /*!
     * \brief Returns the previously stored position of this item.
     *
     * This method is required for recovering the last position while handling
     * undo/redo.
     *
     * \sa storePos()
     */
    QPointF GraphicsItem::storedPos() const
    {
        return m_store;
    }

    /*!
     * \fn GraphicsItem::copy()
     *
     * \brief Returns a copy of the current item parented to \a scene.
     *
     * This method returns a copy of the current item. It is usually used by
     * the copy/paste functionality to have an identical copy of this item
     * ready to paste.
     *
     * Subclasses of the GraphicsItem class should reimplement this method to
     * return an appropriate copy of the reimplemented item, copying in the
     * process the needed properties of the reimplementation.
     *
     * \sa copyDataTo()
     */

    /*!
     * \brief Copies the basic data of the current item to the item passed as a
     * parameter.
     *
     * This method is used by the reimplemented classes to copy the basic data
     * while a copy/paste operation is in progress. While the reimplemented
     * classes must reimplement the copy() method to handle the particular
     * properties of each kind of item, they may call this method to copy the
     * generic data as position, rotation, scale and transforms in general.
     *
     * \sa copy()
     */
    void GraphicsItem::copyDataTo(GraphicsItem *item) const
    {
        item->setShapeAndBoundRect(m_shape, m_boundingRect);
        item->setTransform(transform());
        item->setRotation(rotation());
        item->setPos(pos());
    }

    /*!
     * \brief Constructs and returns a context menu with the actions
     * corresponding to the selected object.
     */
    void GraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        ActionManager* am = ActionManager::instance();
        QMenu *_menu = new QMenu();

        // Launch context menu of item
        _menu->addAction(am->actionForName("editCut"));
        _menu->addAction(am->actionForName("editCopy"));
        _menu->addAction(am->actionForName("editDelete"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("editRotate"));
        _menu->addAction(am->actionForName("editMirrorX"));
        _menu->addAction(am->actionForName("editMirrorY"));

        _menu->addSeparator();

        _menu->addAction(am->actionForName("propertiesDialog"));

        _menu->exec(event->screenPos());
    }

    void GraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->buttons().testFlag(Qt::LeftButton)) {
            launchPropertiesDialog();
        }
    }

    /*!
     * \brief Sets the shape cache as well as boundbox cache
     *
     * This method abstracts the method of changing the geometry with support for
     * cache as well.
     *
     * \param path The path to be cached. If empty, bound rect is added.
     * \param rect The bound rect to be cached.
     * \param pw Pen width of pen used to paint outer stroke of item.
     */
    void GraphicsItem::setShapeAndBoundRect(const QPainterPath& shape,
            const QRectF& boundingRect, qreal penWidth)
    {
        // Inform scene about change in geometry.
        prepareGeometryChange();

        // Adjust the bounding rect by pen width as required by graphicsview.
        m_boundingRect = boundingRect;
        m_boundingRect.adjust(-penWidth, -penWidth, penWidth, penWidth);

        m_shape = shape;
        if(m_shape.isEmpty()) {
            // If path is empty just add the bounding rect to the path.
            m_shape.addRect(m_boundingRect);
        }
    }

} // namespace Caneda
