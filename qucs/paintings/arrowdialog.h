/***************************************************************************
                          arrowdialog.h  -  description
                             -------------------
    begin                : Fri Nov 28 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : margraf@mwt.ee.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ARROWDIALOG_H
#define ARROWDIALOG_H

#include <qdialog.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>


/**
  *@author Michael Margraf
  */

class ArrowDialog : public QDialog  {
Q_OBJECT
public: 
	ArrowDialog(QWidget *parent=0, const char *name=0);
	~ArrowDialog();

  void SetComboBox(Qt::PenStyle _Style);

private slots:
  void slotSetColor();
  void slotSetStyle(int index);

public:
  QRegExp      Expr;
  QLineEdit    *LineWidth, *HeadWidth, *HeadLength;
  QPushButton  *ColorButt;
  QComboBox    *StyleBox;
  Qt::PenStyle LineStyle;
};

#endif
