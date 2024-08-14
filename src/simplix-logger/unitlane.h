//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitlane.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Fahrspur
//
// Datei    : unitlane.h
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
// dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter delphin
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
#ifndef _UNITLANE_H_
#define _UNITLANE_H_

#include "unittrack.h"
#include "unitcarparam.h"
#include "unitfixcarparam.h"

//==========================================================================*
// Class TLane
//--------------------------------------------------------------------------*
class TLane 
{
  public:
	struct TPathPt
	{
		const TSection*	Sec;		             // Track seg that contains this Seg
		TVec3d Center;                           // Lane specific center
		double Crv;	 		                     // Curvature in xy
		double CrvZ;			                 // Curvature in z direction... e.g. bumps
		double Offset;                           // Offset from centre point
		TVec3d Point;                            // Actual point (same as CalcPt())
		double MaxSpeed;                         // Max speed through this point
		double Speed;                            // Speed through this point (braking only)
		double AccSpd;                           // Speed through this point, with modelled accel
		double FlyHeight;                        // Predicted height of car above track (flying)
		double BufL;		                     // Buffer from left for safety
		double BufR;		                     // Buffer from right for safety
		double NextCrv;                          // Cuvature comming next
		double WToL;                             // Lane specfic width to left
		double WToR;                             // Lane specfic width to right 
		double WPitToL;                          // Lane specfic width to left
		double WPitToR;                          // Lane specfic width to right 
		bool Fix;

		double Dist() const {return Sec->DistFromStart;}
		double WtoL() const {return WToL;}
		double WtoR() const {return WToR;}
		const TVec3d& Pt() const {return Center;}
		const TVec3d& Norm() const {return Sec->ToRight;}
		TVec3d CalcPt() const {return Center + Sec->ToRight * Offset;}
	};

  public:
	TLane();
	virtual ~TLane();

	virtual TLane& operator= (const TLane& Lane);

	virtual bool ContainsPos
	  (double TrackPos) const;
	virtual bool GetLanePoint
	  (double TrackPos, TLanePoint& LanePoint) const;

	void SetLane(const TLane& Lane);
	void Initialise
	  (TTrackDescription* pTrack,
      const TFixCarParam& FixCarParam,
	  const TCarParam& CarParam,
	  double MaxLeft = FLT_MAX,
	  double MaxRight = FLT_MAX);

	const TPathPt& PathPoints(int Index) const;

	void CalcCurvaturesXY
	  (int Start, int Step = 1);
	void CalcCurvaturesZ
	  (int Start, int Step = 1);
	void CalcMaxSpeeds
	  (int Start, int Len, int Step = 1);
	void PropagateBreaking
	  (int Start, int Len, int Step = 1);
	void PropagatePitBreaking
	  (int Start, int Len, float PitStopPos, float ScaleMu);
	void PropagateAcceleration
	  (int Start, int Len, int Step = 1);

	void CalcCurvaturesXY
	  (int Step = 1);
	void CalcCurvaturesZ
	  (int Step = 1);
	void CalcMaxSpeeds
	  (int Step = 1);
	void PropagateBreaking
	  (int Step = 1);
	void PropagatePitBreaking
	  (float PitStopPos, float ScaleMu);
	void PropagateAcceleration
	  (int Step = 1);
	void CalcFwdAbsCrv
	  (int Range, int Step = 1);

	double CalcEstimatedTime
	  (int Start, int Len) const;
	double CalcEstimatedLapTime() const;

  protected:
	TTrackDescription* oTrack;                   // TORCS track data
	TPathPt* oPathPoints;                        // Points in this lane
	TFixCarParam oFixCarParam;                   // Copy of car params
	TCarParam oCarParam;                         // Copy of car params
    int Dummy;
};
//==========================================================================*
#endif // _UNITLANE_H_
//--------------------------------------------------------------------------*
// end of file unitlane.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
