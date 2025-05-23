/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef GRAPHICLINE_H
#define GRAPHICLINE_H

#include "painting.h"

namespace Caneda
{
    //! \brief Represents a line on schematic.
    class GraphicLine : public Painting
    {
    public:
        explicit GraphicLine(const QLineF &line, QGraphicsItem *parent = nullptr);

        //! \copydoc GraphicsItem::Type
        enum { Type = Painting::GraphicLineType };
        //! \copydoc GraphicsItem::type()
        int type() const override { return Type; }

        QPainterPath shapeForRect(const QRectF &rect) const override;

        void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *) override;

        //! \brief Returns line represented by this item.
        QLineF line() const { return lineFromRect(paintingRect()); }
        void setLine(const QLineF &line);

        GraphicLine* copy() const override;

        void saveData(Caneda::XmlWriter *writer) const override;
        void loadData(Caneda::XmlReader *reader) override;

        void launchPropertiesDialog() override;

    private:
        //! \brief Returns line from rect.
        QLineF lineFromRect(const QRectF &rect) const {
            return QLineF(rect.topLeft(), rect.bottomRight());
        }
    };

} // namespace Caneda

#endif //GRAPHICLINE_H
