/***************************************************************************
 * Copyright (C) 2013-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef PORTSYMBOL_H
#define PORTSYMBOL_H

#include "port.h"

namespace Caneda
{
    // Forward declarations
    class GraphicsItem;

    /*!
     * \brief Represents the port symbol on component symbols and schematics.
     *
     * This item is used as a "bridge" between symbols and schematics. When
     * used in a symbol, the symbol's port is used as a connection when
     * instantiated into larger schematics. The connection bounds the connected
     * net or wire to the inside circuit (subcircuit in spice terminology) of
     * the component. The inside circuit is defined by a schematic with the
     * same name of the symbol.
     *
     * For the bridge to work, the must be ports in the schematic with the same
     * names as the ports in the symbol.
     *
     */
    class PortSymbol : public GraphicsItem
    {
    public:
        explicit PortSymbol(QGraphicsItem *parent = nullptr);
        ~PortSymbol() override;

        //! \copydoc GraphicsItem::Type
        enum { Type = GraphicsItem::PortSymbolType };
        //! \copydoc GraphicsItem::type()
        int type() const override { return Type; }

        //! Return's the symbol's port
        Port* port() const { return m_ports[0]; }

        //! Return's the symbol's label
        QString label() const { return m_label->text();}
        bool setLabel(const QString &newLabel);

        void updateGeometry();

        void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

        PortSymbol* copy() const override;

        void saveData(Caneda::XmlWriter *writer) const override;
        void loadData(Caneda::XmlReader *reader) override;

        void launchPropertiesDialog() override;

    private:
        QGraphicsSimpleTextItem *m_label;
        QPainterPath m_symbol;
    };

} // namespace Caneda

#endif //PORTSYMBOL_H
