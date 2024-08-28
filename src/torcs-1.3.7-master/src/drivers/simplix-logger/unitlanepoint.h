//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitlanepoint.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Punkt einer Fahrspur
//
// Datei    : unitlanepoint.h
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
#ifndef _UNITLANEPOINT_H_
#define _UNITLANEPOINT_H_

//==========================================================================*
// Deklaration der Klasse TLanePoint
//--------------------------------------------------------------------------*
class TLanePoint  
{
  public:
	TLanePoint();
	~TLanePoint();

  public:
	double T;		                             // Parametric distance to next seg [0..1]
	double Offset; 	                             // Offset from middle for the path.
	double Angle;	                             // Global angle
	double Crv;		                             // Curvature at point
	double Speed;                                // Speed
	double AccSpd;                               // Accelleration speed
	int Index;	                                 // Index of sec.
};
//==========================================================================*
#endif // _UNITLANEPOINT_H_
//--------------------------------------------------------------------------*
// end of file unitlanepoint.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
