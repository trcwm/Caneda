/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "textedit.h"

#include <QTextBlock>

namespace Caneda
{
    //! \brief Constructor.
    TextEdit::TextEdit(QTextDocument *document, QWidget *parent) : QPlainTextEdit(parent)
    {
        QPlainTextDocumentLayout *layout = new QPlainTextDocumentLayout(document);
        document->setDocumentLayout(layout);
        setDocument(document);

        QFont font;
        font.setFamily("Monospace");
        font.setFixedPitch(true);
        font.setPointSize(10);
        setFont(font);

        connect(this, &TextEdit::focussed,                    this, &TextEdit::updateCursorPosition);
        connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEdit::updateCursorPosition);
        connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEdit::highlightCurrentLine);

        highlightCurrentLine();
    }

    void TextEdit::focusInEvent(QFocusEvent *event)
    {
        emit focussed();
        QPlainTextEdit::focusInEvent(event);
    }

    void TextEdit::updateCursorPosition()
    {
        /*!
         * \todo Replace current line number calculation
         * by textcursor.lineNumber() when implemented in Qt.
         */

        // Get the current line number
        QTextCursor textcursor = textCursor();
        QTextLayout* blocklayout = textcursor.block().layout();

        // Get the relative position in the block
        int position = textcursor.position() - textcursor.block().position();
        int line = blocklayout->lineForTextPosition(position).lineNumber() +
                   textcursor.block().firstLineNumber() + 1;

        // Get the current column number
        int column = textcursor.columnNumber() + 1;

        QPoint newCursorPos = QPoint(line, column);
        QString str = QString(tr("Line: %1 Col: %2"))
            .arg(newCursorPos.x())
            .arg(newCursorPos.y());
        emit cursorPositionChanged(str);
    }

    void TextEdit::highlightCurrentLine()
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::lightGray).lighter(126);

            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }

        setExtraSelections(extraSelections);
    }

} // namespace Caneda
