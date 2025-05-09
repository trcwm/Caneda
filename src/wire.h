/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef WIRE_H
#define WIRE_H

#include "port.h"

namespace Caneda
{
    // Forward declarations
    class GraphicsItem;

    /*!
     * \brief The Wire class forms part of one of the GraphicsItem
     * derived classes available on Caneda. It represents a wire on schematic,
     * and connects different components throught the use of ports. One port
     * can have one or multiple wire connections, allowing multiple components
     * to be connected together.
     *
     * \sa GraphicsItem, Component, Port
     */
    class Wire : public GraphicsItem
    {
    public:
        explicit Wire(const QPointF &startPos, const QPointF &endPos,
                      QGraphicsItem *parent = nullptr);

        ~Wire() override;

        //! \copydoc GraphicsItem::Type
        enum { Type = GraphicsItem::WireType };
        //! \copydoc GraphicsItem::type()
        int type() const override { return Type; }

        //! Return's the list's first member.
        Port* port1() const { return m_ports[0]; }
        //! Returns the list's second member.
        Port* port2() const { return m_ports[1]; }

        void movePort1(const QPointF& newScenePos);
        void movePort2(const QPointF& newScenePos);

        //! Return true if wire is horizontal
        bool isHorizontal() const { return int(port1()->pos().y()) == int(port2()->pos().y()); }
        //! Return true if wire is vertical
        bool isVertical() const { return int(port1()->pos().x()) == int(port2()->pos().x()); }
        //! Check if port 1 and 2 overlap
        bool isNull() const { return port1()->scenePos() == port2()->scenePos(); }

        void updateGeometry();
        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                   QWidget *widget = nullptr) override;

        Wire* copy() const override;

        void saveData(Caneda::XmlWriter *writer) const override;
        void loadData(Caneda::XmlReader *reader) override;

        //! \copydoc GraphicsItem::launchPropertiesDialog()
        void launchPropertiesDialog() override {}

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    };

} // namespace Caneda

#endif //WIRE_H
