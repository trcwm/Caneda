/***************************************************************************
                          sp_sim.cpp  -  description
                             -------------------
    begin                : Sat Aug 23 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sp_sim.h"
#include "main.h"


SP_Sim::SP_Sim()
{
  Description = QObject::tr("S parameter simulation");

  QString s = Description;
  int a = s.find(" ");
  int b = s.findRev(" ");
  if (a != -1 && b != -1) {
    if (a > (int) s.length() - b)  b = a;
  }
  if (b != -1) s[b] = '\n';
  QFontMetrics metrics(QucsSettings.largeFont);
  QSize r = metrics.size(0, s);
  int xb = r.width()  + 15;
  int yb = r.height() + 15;

  Texts.append(new Text(0, 0, s.left(b)));
  if (b != -1) Texts.append(new Text(0, 0, s.mid(b+1)));

  x1 = -10; y1 = -9;
  x2 = x1+xb+8; y2 = y1+yb+8;

  tx = 0;
  ty = y2+1;
  Model = ".SP";
  Name  = "SP";

  Props.append(new Property("Type", "lin", true,
			QObject::tr("sweep type")+" [lin, log]"));
  Props.append(new Property("Start", "1 GHz", true,
			QObject::tr("start frequency in Hertz")));
  Props.append(new Property("Stop", "10 GHz", true,
			QObject::tr("stop frequency in Hertz")));
  Props.append(new Property("Points", "19", true,
			QObject::tr("number of simulation steps")));
  Props.append(new Property("Noise", "no", false,
			QObject::tr("calculate noise parameters")+
			" [yes, no]"));
  Props.append(new Property("NoiseIP", "1", false,
			QObject::tr("input port for noise figure")));
  Props.append(new Property("NoiseOP", "2", false,
			QObject::tr("output port for noise figure")));
}

SP_Sim::~SP_Sim()
{
}

Component* SP_Sim::newOne()
{
  return new SP_Sim();
}

Component* SP_Sim::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("S-parameter simulation");
  BitmapFile = "sparameter";

  if(getNewOne)  return new SP_Sim();
  return 0;
}
