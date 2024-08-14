//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitparam.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Container für Parameter des Fahrzeugs, der Fahrspuren, der Box usw.
//
// Datei    : unitparam.h
// Erstellt : 11.04.2008
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
#ifndef _UNITPARAM_H_
#define _UNITPARAM_H_

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitcarparam.h"
#include "unitfixcarparam.h"
#include "unitpitparam.h"
#include "unittmpcarparam.h"

//==========================================================================*
// Deklaration der Klasse TParam
//--------------------------------------------------------------------------*
class TParam  
{
  public:
	TParam();                                    // Default constructor 
    ~TParam();                                   // Destructor

	void Initialize
	  (PtCarElt Car);
	void SetEmptyMass(float EmptyMass);
    void Update();

    PtCarElt oCar;                               // Pointer to TORCS data of car

	TCarParam oCarParam;                         // Main parameter set
	TCarParam oCarParam2;                        // Avoiding parameter set
	TCarParam oCarParam3;                        // Pitting parameter set

	TPitParam Pit;                               // Parameters of the pit
	TTmpCarParam Tmp;                            // State of the car
	TFixCarParam Fix;                            // Data of the car
};
//==========================================================================*
#endif // _UNITPARAM_H_
//--------------------------------------------------------------------------*
// end of file unitparam.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
