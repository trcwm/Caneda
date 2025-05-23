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

#ifndef CHART_SCENE_H
#define CHART_SCENE_H

#include <chartitem.h>

#include <QWidget>

namespace Caneda
{
    /*!
     * \brief This class implements the scene class of Qt's Graphics View
     * Architecture, representing the actual document interface (scene),
     * containing the simulation waveform data.
     *
     * Each scene must have at least one associated view (ChartView), to
     * display the contents of the scene (waveforms). Several views can be
     * attached to the same scene, providing different viewports into the same
     * data set (for example, when using split views).
     *
     * \sa ChartView
     */
    class ChartScene : public QWidget
    {
        Q_OBJECT

    public:
        explicit ChartScene(QWidget *parent = nullptr);

        //! \brief Returns a list of all items in the scene in descending stacking
        QList<ChartSeries*> items() const { return m_items; }
        void addItem(ChartSeries *item);

    private:
        QList<ChartSeries*> m_items;  //! \brief Items available in the scene (curves, markers, etc)
    };

} // namespace Caneda

#endif //CHART_SCENE_H
