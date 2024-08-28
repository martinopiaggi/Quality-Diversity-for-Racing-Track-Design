//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcommondata.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Zentrale Klasse für das Fahren bzw. den Fahrer/Roboter
//
// Datei    : unitcommondata.h
// Erstellt : 17.11.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf diversen Header-Dateien von TORCS
//
//    Copyright: (C) 2000 by Eric Espie
//    eMail    : torcs@free.fr
//
// dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter mouse_2006
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
#ifndef _UNITCOMMONDATA_H_
#define _UNITCOMMONDATA_H_

#include <track.h>
#include "unitglobal.h"
#include "unitteammanager.h"
#include "unitclothoid.h"

//==========================================================================*
// Deklaration der Klasse TCommonData
//--------------------------------------------------------------------------*
class TCommonData  
{
  public:
    TCommonData();                               // Default constructor
    ~TCommonData();                              // Destructor

  public:
    TTeamManager TeamManager;                    // Team manager 
    PTrack Track;                                // TORCS Track data  
};
//==========================================================================*
#endif // _UNITCOMMONDATA_H_
//--------------------------------------------------------------------------*
// end of file unitcommondata.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
