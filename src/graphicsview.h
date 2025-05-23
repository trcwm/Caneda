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

#ifndef GRAPHICS_VIEW_H
#define GRAPHICS_VIEW_H

#include "global.h"

#include <QGraphicsView>

namespace Caneda
{
    // Forward declarations
    class GraphicsScene;

    /*!
     * \brief This class provides a view for displaying all Caneda graphics
     * scenes (schematics, symbols, layouts, etc).
     *
     * This class implements the view class of Qt's Graphics View Architecture.
     * The view class provides the view widget, which visualizes the contents
     * of a scene. You can attach several views to the same scene, to provide
     * different viewports into the same data set of the document (for example,
     * when using split views).
     *
     * In this class common view operations are implemented, for example
     * zooming and focus events. In this way, one single scene can have
     * multiple views associated to it, allowing the user to look at the scene
     * for example, with multiple zoom levels.
     *
     * \sa GraphicsScene
     */
    class GraphicsView : public QGraphicsView
    {
        Q_OBJECT

    public:
        explicit GraphicsView(GraphicsScene *scene = nullptr);

        GraphicsScene* graphicsScene() const;

        void zoomIn();
        void zoomOut();
        void zoomFitInBest();
        void zoomOriginal();
        void zoomFitRect(const QRectF &rect);

        qreal currentZoom() { return m_currentZoom; }

    Q_SIGNALS:
        void cursorPositionChanged(const QString& newPos);
        void focussedIn(GraphicsView *view);
        void focussedOut(GraphicsView *view);

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void focusInEvent(QFocusEvent *event) override;
        void focusOutEvent(QFocusEvent *event) override;

    private Q_SLOTS:
        void onMouseActionChanged(Caneda::MouseAction mouseAction);

    private:
        void setZoomLevel(qreal zoomLevel);

        const qreal m_zoomFactor;
        ZoomRange m_zoomRange;
        qreal m_currentZoom;

        //! \brief Auxiliary pan variables
        bool panMode;
        QPointF panStartPosition;
    };

} // namespace Caneda

#endif //GRAPHICS_VIEW_H
