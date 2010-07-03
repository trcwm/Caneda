/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef CANEDA_GLOBALS_H
#define CANEDA_GLOBALS_H

#include <QDebug>

namespace Caneda
{
    enum SideBarRole {
        ItemSelection,
        PropertyBrowser
    };

    struct ZoomRange
    {
        const qreal min;
        const qreal max;

        ZoomRange(qreal _min = 0., qreal _max = 1.0) :
            min(_min), max(_max)
        {
            if (max < min) {
                qWarning() << Q_FUNC_INFO << "Invalid range" << min << max;
            }
        }

        bool contains(qreal value) const {
            return value >= min && value <= max;
        }
    };

} // namespace Caneda

#endif //CANEDA_GLOBALS_H
