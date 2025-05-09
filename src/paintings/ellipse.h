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

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "painting.h"

namespace Caneda
{
    //! \brief Represents an ellipse on a graphics scene.
    class Ellipse : public Painting
    {
    public:
        explicit Ellipse(QRectF rect, QGraphicsItem *parent = nullptr);

        //! \copydoc GraphicsItem::Type
        enum { Type = Painting::EllipseType };
        //! \copydoc GraphicsItem::type()
        int type() const override { return Type; }

        QPainterPath shapeForRect(const QRectF &rect) const override;

        void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget *) override;

        //! \brief Returns ellipse rect represented by this item.
        QRectF ellipse() const { return paintingRect(); }
        void setEllipse(const QRectF& rect) { setPaintingRect(rect); }

        Ellipse* copy() const override;

        void saveData(Caneda::XmlWriter *writer) const override;
        void loadData(Caneda::XmlReader *reader) override;

        void launchPropertiesDialog() override;
    };

} // namespace Caneda

#endif //ELLIPSE_H
