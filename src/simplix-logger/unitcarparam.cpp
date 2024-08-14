//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcarparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Tuningparameter des Fahrzeugs
//
// Datei    : unitcarparam.cpp
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
#include <math.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitparabel.h"
#include "unitcarparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TCarParam::TCarParam():
  oScaleMu(0.8),
  oScaleMinMu(0.8),
  oScaleBrake(0.8),
  oScaleBrakePit(1.0),
  oScaleBump(0.4f),
  oScaleBumpOuter(0.6f),
  oScaleBumpLeft(0),
  oScaleBumpRight(0)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TCarParam::~TCarParam()
{
}
//===========================================================================

//==========================================================================*
// Zuweisung
//--------------------------------------------------------------------------*
TCarParam& TCarParam::operator= (const TCarParam& CarParam)
{
  oScaleMu = CarParam.oScaleMu;
  oScaleMinMu = CarParam.oScaleMinMu;
  oScaleBrake = CarParam.oScaleBrake;
  oScaleBrakePit = CarParam.oScaleBrakePit;
  oScaleBump = CarParam.oScaleBump;
  oScaleBumpOuter = CarParam.oScaleBumpOuter;
  oScaleBumpLeft = CarParam.oScaleBumpLeft;
  oScaleBumpRight = CarParam.oScaleBumpRight;
  return *this;
}
//===========================================================================

//--------------------------------------------------------------------------*
// end of file unitcarparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
