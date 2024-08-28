//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitpitparam.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Parameter der Box und der Anfahrt zur Box
//
// Datei    : unitpitparam.h
// Erstellt : 11.04.2007
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
#ifndef _UNITPITPARAM_H_
#define _UNITPITPARAM_H_

//==========================================================================*
// Deklaration der Klasse TPitParam
//--------------------------------------------------------------------------*
class TPitParam  
{
  private:

  public:
    TPitParam();                                 // Default constructor 
	~TPitParam();                                // Destructor

  public:
	double oEntryLong;	   	                     // Translation longitudinal
	double oExitLong;		                     // Translation longitudinal
    float oExitLength;                           // Dist in m

	double oLaneEntryOffset;                     // Additional offset to pit
	double oLaneExitOffset;                      // Additional offset to pit

	double oLatOffset;                           // Lateral offset of pit
	double oLongOffset;                          // Longitudinal offset of pit

	double oStoppingDist;                        // Stopping distance 

	int oUseFirstPit;                            // Use special path to first pit
    int oUseSmoothPit;                           // Use smoothing pitlane

};
//==========================================================================*
#endif // _UNITPITPARAM_H_
//--------------------------------------------------------------------------*
// end of file unitpitparam.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
