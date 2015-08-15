/***************************************************************************
 * Copyright (C) 2013-2015 by Pablo Daniel Pareja Obregon                  *
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

#include "csimulationview.h"

#include "csimulationscene.h"
#include "settings.h"

#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_zoomer.h>

namespace Caneda
{
    /*!
     * \brief Constructs a new simulation view.
     *
     * \param sv Simulation scene to point this view to.
     * \param parent Parent of this object.
     */
    CSimulationView::CSimulationView(CSimulationScene *scene, QWidget *parent) :
        QwtPlot(parent),
        m_csimulationScene(scene)
    {
        // Canvas
        m_canvas = new QwtPlotCanvas();
        m_canvas->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        setCanvas(m_canvas);

        // Axes
        setAxisTitle(xBottom, QwtText(tr("Time [s]")));
        setAxisTitle(yLeft, QwtText(tr("Voltage [V]")));

        // Panning with the middle mouse button
        QwtPlotPanner *panner = new QwtPlotPanner(m_canvas);
        panner->setMouseButton(Qt::MidButton);

        // Zoom in/out with the wheel
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(m_canvas);
        magnifier->setMouseButton(Qt::NoButton);  // Disable default left button action

        // Box zoom with left button and position label
        QwtPlotZoomer *zoomer = new QwtPlotZoomer(m_canvas);
        zoomer->setTrackerMode(QwtPicker::AlwaysOn);
        zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::NoButton); // Disable default right button action
        zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::NoButton); // Disable default middle button action

        // Grid
        m_grid = new QwtPlotGrid();
        m_grid->enableXMin(true);

        // Legend
        m_legend = new QwtLegend();
        insertLegend(m_legend, QwtPlot::TopLegend);

        // Load user settings, for example canvas color
        loadUserSettings();
    }

    void CSimulationView::zoomIn()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomOut()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomFitInBest()
    {
        //! \todo Implement this
    }

    void CSimulationView::zoomOriginal()
    {
        //! \todo Implement this
    }

    //! \brief Displays all items available in the scene, in the plot widget.
    void CSimulationView::showAll()
    {
        QList<QwtPlotCurve*> m_items = m_csimulationScene->items();

        QColor color = QColor(0, 0, 0);
        int colorIndex= 0;
        int valueIndex = 255;

        // Attach the items to the plot
        foreach(QwtPlotCurve *item, m_items) {
            // Recreate the curve to be able to attach
            // the same curve to different views
            QwtPlotCurve *newCurve = new QwtPlotCurve();
            newCurve->setData(item->data());
            newCurve->attach(this);
            newCurve->setTitle(item->title());

            newCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);

            // Select the color of the new curve
            color.setHsv(colorIndex , 200, valueIndex);
            newCurve->setPen(QPen(color));

            // Set the next color to be used (to change colors
            // from curve to curve.
            if(colorIndex < 300) {  // Avoid 360, as it equals 0
                colorIndex += 60;
            }
            else {
                colorIndex = 0;
                if(valueIndex == 255) {
                    valueIndex = 100;
                }
                else {
                    valueIndex = 255;
                }
            }
        }

        // Refresh the plot
        replot();
    }

    //! \brief Loads the saved user settings, updating the values on the canvas.
    void CSimulationView::loadUserSettings()
    {
        // Load settings
        Settings *settings = Settings::instance();
        QColor foregroundColor = settings->currentValue("gui/foregroundColor").value<QColor>();
        QColor backgroundColor = settings->currentValue("gui/simulationBackgroundColor").value<QColor>();

        // Canvas
        QPalette canvasPalette(backgroundColor);
        m_canvas->setPalette(canvasPalette);

        // Grid
        if(Settings::instance()->currentValue("gui/gridVisible").value<bool>()) {
            m_grid->setMajorPen(QPen(foregroundColor, 1, Qt::DashLine));
            m_grid->setMinorPen(QPen(foregroundColor, 0 , Qt::DotLine));
            m_grid->attach(this);
        }
        else {
            m_grid->detach();
        }
    }

    void CSimulationView::print(QPrinter *printer, bool fitInView)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);

        renderer.renderTo(this, *printer);
    }

    void CSimulationView::exportImage(QPaintDevice &device)
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardNone, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);

        renderer.renderTo(this, device);
    }

    //! \brief Shows or hides selected plot.
    void CSimulationView::setPlotVisible(QwtPlotItem *plotItem, bool visible)
    {
        // Perform show/hide action
        if(visible){
            plotItem->show();
        }
        else{
            plotItem->hide();
        }

        // Refresh the plot
        replot();
    }

} // namespace Caneda
