/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef STYLEDIALOG_H
#define STYLEDIALOG_H

#include "ui_styledialog.h"

#include "painting.h"

// Forward declarations
class QBrush;
class QPen;
class QPolygon;

namespace Caneda
{
    // Forward declarations
    class Painting;

    class PreviewWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit PreviewWidget(int paintingType, QWidget *widget = nullptr);

        QPen pen() const { return m_pen; }
        void setPen(QPen pen);

        QBrush brush() const { return m_brush; }
        void setBrush(QBrush brush);

        int headStyle() const { return m_headStyle; }
        void setHeadStyle(int style);

        int headWidth() const { return m_headWidth; }
        void setHeadWidth(int width);

        int headHeight() const { return m_headHeight; }
        void setHeadHeight(int height);

        QSize headSize() const { return QSize(m_headWidth, m_headHeight); }
        void setHeadSize(QSize size);

        int startAngle() const { return m_startAngle; }
        void setStartAngle(int angle);

        int spanAngle() const { return m_spanAngle; }
        void setSpanAngle(int angle);

        void paintEvent(QPaintEvent *event) override;

        void calcHeadPoints();

        int heightForWidth(int w) const override { return w; }

        public slots:
            void toggleBackground(bool state);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QRect adjustedRect() const;
        void drawBackgroundBoxes(QPainter *painter);

        void drawArrow(QPainter *painter);
        void drawEllipse(QPainter *painter);
        void drawEllipseArc(QPainter *painter);
        void drawLine(QPainter *painter);
        void drawRectangle(QPainter *painter);

        QPen m_pen;
        QBrush m_brush;
        QPixmap m_lightPixmap;
        QPixmap m_darkPixmap;

        int m_headStyle;
        QPolygon m_headPolygon;
        int m_headWidth;
        int m_headHeight;

        int m_startAngle;
        int m_spanAngle;

        bool m_drawBackground;

        int m_paintingType;
    };

    class StyleDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit StyleDialog(Painting *painting, QWidget *parent = nullptr);

    public Q_SLOTS:
        void setupStyleWidgets();
        void updatePreview();

        void launchColorDialog();
        void applySettings();

    private:
        PreviewWidget *previewWidget;
        QColor lineColor;
        QColor fillColor;

        QPixmap lineColorPixmap;
        QPixmap fillColorPixmap;

        QPolygon headPolygon;

        Painting *painting;

        Ui::StyleDialogBase ui;
    };

} // namespace Caneda

#endif //STYLEDIALOG_H
