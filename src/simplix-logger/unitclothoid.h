//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitclothoid.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Fahrspur clothoidenähnlich
//
// Datei    : unitclothoid.h
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
#ifndef _UNITCLOTHOID_H_
#define _UNITCLOTHOID_H_

#include "unitglobal.h"
#include "unitlane.h"
#include "unittrack.h"

//==========================================================================*
// Class TClothoidLane
//--------------------------------------------------------------------------*
class TClothoidLane : public TLane
{
  public:
	enum
	{
	  FLAG_FLYING = 0x01
	};

	struct TOptions
	{
	  double BumpMod;
	  double MaxL;
	  double MaxR;
	  bool Side;
	  bool Smooth;

	  TOptions() : BumpMod(1), MaxL(FLT_MAX), MaxR(FLT_MAX), Side(false), Smooth(false) {}
	  TOptions(double BM, double ML = FLT_MAX, double MR = FLT_MAX, bool SI = false, bool SM = false):
		BumpMod(BM), MaxL(ML), MaxR(MR), Side(SI), Smooth(SM) {}
	};

  public:
	TClothoidLane();
	virtual ~TClothoidLane();

	void MakeSmoothPath
	  (TTrackDescription* pTrack, 
	   TParam& Param, 
	   const TOptions& Opts);

	bool LoadSmoothPath
	  (char* TrackLoad, 
	   TTrackDescription* pTrack, 
	   TParam& Param, 
	   const TOptions& Opts);

	void SmoothPath
	  (TTrackDescription* pTrack, 
	   const TParam& Param, 
	   const TOptions& Opts);

	bool SaveToFile(const char* Filename);       // Save to file
    void SavePointsToFile(const char* TrackLoad);
    bool LoadPointsFromFile(const char* TrackLoad);

  private:
	void AnalyseBumps
	  (bool DumpInfo = false);
    void Adjust
      (double Crv1, double Len1,
	   double Crv2, double Len2, 
       const TPathPt* PP, 
       TPathPt* P, 
       const TPathPt* PN, 
       TVec3d VPP,
       TVec3d VPN,
       double BumpMod);
	void SmoothBetween
	  (int Step,
	   double BumpMode = 0.0);
	void SetOffset
	  (double Crv, 
	   double T, 
	   TPathPt* P, 
	   const TPathPt* PP, 
	   const TPathPt* PN);
	void OptimiseLine
	  (int Idx, 
	   int Step, 
	   double HLimit, 
	   TPathPt* L3, 
	   const TPathPt* L2, 
	   const TPathPt* L4);
	void Optimise
	  (double Factor, 
	   TPathPt* L3, 
	   const TPathPt* L0, 
	   const TPathPt* L1, 
	   const TPathPt* L2, 
	   const TPathPt* L4, 
	   const TPathPt* L5, 
	   const TPathPt* L6, 
	   double BumpMod);
	void OptimisePath
	  (int Step, 
	   int NIterations, 
	   double BumpMod,
	   bool Smooth);

};
//==========================================================================*
#endif // _UNITCLOTHOID_H_
//--------------------------------------------------------------------------*
// end of file unitclothoid.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
