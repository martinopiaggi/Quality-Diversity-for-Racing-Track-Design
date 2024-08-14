//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitlinreg.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Lineare Regression
//
// Datei    : unitlinreg.h
// Erstellt : 17.11.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf 
//
// dem Roboter mouse_2006
//    Copyright: (C) 2006 Tim Foden
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
#ifndef _UNITLINREG_H_
#define _UNITLINREG_H_

#include "unitglobal.h"
#include "unitvec2d.h"

//==========================================================================*
// Class TLinearRegression
//--------------------------------------------------------------------------*
class TLinearRegression  
{
  public:
	TLinearRegression();
	~TLinearRegression();

	void Clear();
	void Add(double X, double Y);
	void Add(const TVec2d& Point);
	double CalcY(double X) const;
	void CalcLine(TVec2d& Point, TVec2d& V) const;

  public:
	int	oCount;
	double oSumX;
	double oSumY;
	double oSumXY;
	double oSumXX;
	double oSumYY;
};
//==========================================================================*
#endif // _UNITLINREG_H_
//--------------------------------------------------------------------------*
// end of file unitlinreg.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
