//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitpit.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Box und Boxengasse
// (C++-Portierung der Unit UnitPit.pas)
//
// Datei    : unitpit.h
// Erstellt : 20.02.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// und dem Roboter delphin 2006
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
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
#ifndef _PIT_H_
#define _PIT_H_

#include "unitglobal.h"
#include "unitcommon.h"
#include "unitdriver.h"
#include "unittrack.h"
#include "unitclothoid.h"
#include "unitparam.h"

//==========================================================================*
// Klasse TPitLane
//--------------------------------------------------------------------------*
class TPitLane : public TClothoidLane
{
  private:
	PtCarElt oCar;                               // TORCS data of car

	double oPitDist;                             // Distance to decide 
	double oPitEndPos;                           // End of speed limit
    double oPitEntryPos;                         // Start of pitlane
	double oPitExitPos;                          // End of pitlane
	double oPitStartPos;                         // Start of speed limit
	double oPitStopPos;                          // Position of pit
	double oStoppingDist;                        // Stopping distance 
	double oPitStopOffset;                       // Offset from TORCS point

	int	oStopIdx;                                // Index of section of pit
	double oStopPos;                             // Position to stop

  private:
	double ToSplinePos(double TrackPos) const;   // 

  public:
	void Init(PtCarElt Car);                     // Initialize oCar
    void MakePath                                // Build pitlane
	  (char* Filename, 
	  TClothoidLane* BasePath, 
	  const TParam& Param, 
	  int Index);
    void SmoothPitPath 
      (const TParam& Param);

	bool InPitSection(double TrackPos) const;
	bool CanStop(double TrackPos) const;
	bool Overrun(double TrackPos) const;

	double DistToPitEntry(double TrackPos) const;
	double DistToPitStop(double TrackPos, bool Pitting) const;
	double PitDist(){return oPitDist;};
	double StoppingDist(){return oStoppingDist;};
};
//==========================================================================*

//==========================================================================*
// Klasse TPit
//--------------------------------------------------------------------------*
class TPit
{
  public:
	PTrack oTrack;                               // TORCS track data
	PCarElt oCar;                                // TORCS car data
	TPitLane oPitLane[gNBR_RL];                  // Pitlanes
	tTrackOwnPit *oMyPit;			             // Pointer to my pit.
	tTrackPitInfo *oPitInfo;	                 // General pit info.

	bool oPitStop;					             // Pitstop planned.
	bool oInPitLane;			 		         // We are still in the pitlane.
	float oPitEntry;				  	         // Distance to start line of the pit entry.
	float oPitExit;					             // Distance to the start line of the pit exit.

	float oSpeedLimitSqr;			             // Pit speed limit squared.
	float oSpeedLimit;				             // Pit speed limit.
	float oPitSpeedLimitSqr;			         // The original speedlimit squared.

	float oPitTimer;					         // Timer for pit timeouts.

	static const float SPEED_LIMIT_MARGIN;

	TPit(TDriver *driver);
	~TPit();

	void SetPitstop(bool oPitStop);
	bool GetPitstop()
	  {return oPitStop;}

	void SetInPit(bool InPitLane)
	  {this->oInPitLane = InPitLane;}
	bool GetInPit()
	  {return oInPitLane;}

	float GetPitOffset(float Offset, float FromStart);

	bool IsBetween(float FromStart);
	bool IsTimeout(float Distance);

	float ToSplineCoord(float X);
	float GetSpeedLimitBrake(float SpeedSqr);

	void Update();
};
//==========================================================================*
#endif // _PIT_H_
//--------------------------------------------------------------------------*
// end of file unitpit.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*


