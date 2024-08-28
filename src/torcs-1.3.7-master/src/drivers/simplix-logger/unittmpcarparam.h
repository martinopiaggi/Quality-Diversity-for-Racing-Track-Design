//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unittmpcarparam.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Zeitlich variable Parameter des Fahrzeugs
//
// Datei    : unittmpcarparam.h
// Erstellt : 25.11.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
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
#ifndef _UNITTMPCARPARAM_H_
#define _UNITTMPCARPARAM_H_

#include "unitglobal.h"
#include "unitcommon.h"

//==========================================================================*
// Deklaration der Klasse TTmpCarParam
//--------------------------------------------------------------------------*
class TTmpCarParam  
{
  private:

  public:
    PtCarElt oCar;                               // Pointer to TORCS data of car

	TTmpCarParam();                              // Default constructor 
	~TTmpCarParam();                             // Destructor

	void Initialize(PtCarElt Car);
    bool Needed();
	void Update();

  public:
	double oDamage;                              // Damage of this car
	double oEmptyMass;                           // Mass of car wihtout fuel
	double oFuel;                                // Mass of fuel in car
	double oMass;                                // Mass of car.with fuel
};
//==========================================================================*
#endif // _UNITTMPCARPARAM_H_
//--------------------------------------------------------------------------*
// end of file unittmpcarparam.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
