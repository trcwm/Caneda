/***************************************************************************
 * Copyright (C) 2011 Aurélien Gâteau <agateau@kde.org>                    *
 * Copyright (C) 2014 Dominik Haumann <dhaumann@kde.org>                   *
 * Copyright (C) 2015 by Pablo Daniel Pareja Obregon                       *
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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QFrame>

namespace Caneda
{
    // Forward declarations
    class MessageWidgetPrivate;

    /*!
     * \brief Widget to provide feedback based on current interactions.
     *
     * MessageWidget can be used to provide inline positive or negative
     * feedback, based on current interactions. As a feedback widget,
     * MessageWidget provides a less intrusive alternative to modal message
     * boxes.
     *
     * <b>Negative feedback</b>
     *
     * The MessageWidget can be used as a secondary indicator of failure: the
     * first indicator is usually the fact the action the user expected to happen
     * did not happen.
     *
     * Example: User fills a form, clicks "Submit".
     *
     * @li Expected feedback: form closes.
     * @li First indicator of failure: form stays there.
     * @li Second indicator of failure: a MessageWidget appears on top of the
     * form, explaining the error condition.
     *
     */
    class MessageWidget : public QFrame
    {
        Q_OBJECT

        Q_ENUMS(MessageType)

        Q_PROPERTY(QString text READ text WRITE setText)
        Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
        Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible)
        Q_PROPERTY(MessageType messageType READ messageType WRITE setMessageType)
        Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

    public:

        /**
         * \brief Available message types.
         *
         * The background colors are chosen depending on the message type.
         */
        enum MessageType {
            Positive,
            Information,
            Warning,
            Error
        };

        explicit MessageWidget(QWidget *parent = nullptr);
        explicit MessageWidget(const QString &text, QWidget *parent = nullptr);

        ~MessageWidget() override;

        MessageType messageType() const;
        QString text() const;
        bool wordWrap() const;

        bool isCloseButtonVisible() const;

        void addAction(QAction *action);
        void removeAction(QAction *action);

        QSize sizeHint() const Q_DECL_OVERRIDE;
        QSize minimumSizeHint() const Q_DECL_OVERRIDE;
        int heightForWidth(int width) const Q_DECL_OVERRIDE;

        bool isHideAnimationRunning() const;
        bool isShowAnimationRunning() const;

        QIcon icon() const;

    public Q_SLOTS:
        void setMessageType(MessageWidget::MessageType type);
        void setText(const QString &text);
        void setWordWrap(bool wordWrap);

        void setCloseButtonVisible(bool visible);

        void animatedShow();
        void animatedHide();
        void slotTimeLineChanged(qreal);
        void slotTimeLineFinished();

        void setIcon(const QIcon &icon);

    Q_SIGNALS:
        //! \brief This signal is emitted when the user clicks a link in the text label.
        void linkActivated(const QString &contents);

        //! \brief This signal is emitted when the user hovers over a link in the text label.
        void linkHovered(const QString &contents);

        /**
         * \brief This signal is emitted when the hide animation is finished, started by
         * calling animatedHide(). If animations are disabled, this signal is
         * emitted immediately after the message widget got hidden.
         *
         * @note This signal is @e not emitted if the widget was hidden by
         *       calling hide(), so this signal is only useful in conjunction
         *       with animatedHide().
         *
         * @see animatedHide()
         */
        void hideAnimationFinished();

        /**
         * \brief This signal is emitted when the show animation is finished, started by
         * calling animatedShow(). If animations are disabled, this signal is
         * emitted immediately after the message widget got shown.
         *
         * @note This signal is @e not emitted if the widget was shown by
         *       calling show(), so this signal is only useful in conjunction
         *       with animatedShow().
         *
         * @see animatedShow()
         */
        void showAnimationFinished();

    protected:
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
        bool event(QEvent *event) Q_DECL_OVERRIDE;
        void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    private:
        MessageWidgetPrivate *const d;
        friend class MessageWidgetPrivate;
    };

} // namespace Caneda

#endif //MESSAGEWIDGET_H
