//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitpidctrl.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// PID Controller
//
// Datei    : unitpidctrl.cpp
// Erstellt : 25.11.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*

#include "unitglobal.h"
#include "unitcommon.h"
#include "unitpidctrl.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TPidController::TPidController(): oLastPropValue(0), oTotal(0),
  oMaxTotal(100), oTotalRate(0), oP(1),	oI(0), oD(0)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TPidController::~TPidController()
{
}
//==========================================================================*

//==========================================================================*
//
//--------------------------------------------------------------------------*
double TPidController::Sample(double PropValue)
{
  return Sample(PropValue, PropValue - oLastPropValue);
}
//==========================================================================*

//==========================================================================*
//
//--------------------------------------------------------------------------*
double TPidController::Sample(double PropValue, double DiffValue)
{
  oLastPropValue = PropValue;

  double Cntrl = PropValue * oP;

  if (oD != 0)
  {
    Cntrl += DiffValue * oD;
  }

  if (oI != 0)
  {
	if (oTotalRate == 0)
	  oTotal += PropValue;
	else
	  oTotal += (PropValue - oTotal) * oTotalRate;

	if (oTotal > oMaxTotal)
	  oTotal = oMaxTotal;
	else if (oTotal < -oMaxTotal)
	  oTotal = -oMaxTotal;
	
	Cntrl += oTotal * oI;
  }

  return Cntrl;
}
//==========================================================================*
// end of file unitpidctrl.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
