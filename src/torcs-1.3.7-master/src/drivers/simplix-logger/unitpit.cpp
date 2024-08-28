//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitpit.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Box und Boxengasse
// (C++-Portierung der Unit UnitPit.pas)
//
// Datei    : unitpit.cpp
// Erstellt : 20.02.2007
// Stand    : 18.07.2009
// Copyright: © 2007-2009 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.001
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
// Changes:
//
// Version  : 1.10.001
// Swap side for avoid pitlanes depending on side of pits
//
//--------------------------------------------------------------------------*
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitlinalg.h"
#include "unitpit.h"
#include "unitcubicspline.h"

//==========================================================================*
// [m/s] savety margin to avoid pit speeding.
//--------------------------------------------------------------------------*
const float TPit::SPEED_LIMIT_MARGIN = 0.5;
//==========================================================================*

//==========================================================================*
// Konstruktor
//--------------------------------------------------------------------------*
TPit::TPit(TDriver *Driver)
{
  oTrack = Driver->Track();
  oCar = Driver->Car();
  oMyPit = Driver->Car()->_pit;
  oPitInfo = &oTrack->pits;
  oPitStop = oInPitLane = false;
  oPitTimer = 0.0;

  if (oMyPit != NULL)
  {
	oSpeedLimit = oPitInfo->speedLimit - SPEED_LIMIT_MARGIN;
	oSpeedLimitSqr = oSpeedLimit*oSpeedLimit;
	oPitSpeedLimitSqr = oPitInfo->speedLimit*oPitInfo->speedLimit;
  }

  for (int I = 0; I < gNBR_RL; I++)
    oPitLane[I].Init(Driver->Car());
}
//==========================================================================*

//==========================================================================*
// Destruktor
//--------------------------------------------------------------------------*
TPit::~TPit()
{
}
//==========================================================================*

//==========================================================================*
// Transforms track coordinates to spline parameter coordinates.
//--------------------------------------------------------------------------*
float TPit::ToSplineCoord(float X)
{
  X -= oPitEntry;
  while (X < 0.0f)
	X += oTrack->length;

  return X;
}
//==========================================================================*

//==========================================================================*
// Computes offset to track middle for trajectory.
//--------------------------------------------------------------------------*
float TPit::GetPitOffset(float Offset, float FromStart)
{
  if (oMyPit != NULL)
  {
	if (GetInPit() || (GetPitstop() && IsBetween(FromStart)))
	  FromStart = ToSplineCoord(FromStart);
  }
  return Offset;
}
//==========================================================================*

//==========================================================================*
// Sets the pitstop flag if we are not in the pit range.
//--------------------------------------------------------------------------*
void TPit::SetPitstop(bool PitStop)
{
  if (oMyPit == NULL)
	return;

  float FromStart = DistanceFromStartLine;

  if (!PitStop)
	this->oPitStop = PitStop;                 // Reset every time
  else if (!IsBetween(FromStart))
	this->oPitStop = PitStop;
  else if (!PitStop)
  {
	this->oPitStop = PitStop;
	oPitTimer = 0.0f;
  }
}
//==========================================================================*

//==========================================================================*
// Check if the argument fromstart is in the range of the pit.
//--------------------------------------------------------------------------*
bool TPit::IsBetween(float FromStart)
{
  if (oPitEntry <= oPitExit)
  {
	GfOut("1. FromStart: %g\n",FromStart);
	if (FromStart >= oPitEntry && FromStart <= oPitExit)
  	  return true;
	else
	  return false;
  }
  else
  {
	// Warning: TORCS reports sometimes negative values for "fromstart"!
	GfOut("2. FromStart: %g\n",FromStart);
	if (FromStart <= oPitExit || FromStart >= oPitEntry)
	  return true;
	else
	  return false;
  }
}
//==========================================================================*

//==========================================================================*
// Checks if we stay too long without getting captured by the pit.
// Distance is the distance to the pit along the track, when the pit is
// ahead it is > 0, if we overshoot the pit it is < 0.
//--------------------------------------------------------------------------*
bool TPit::IsTimeout(float Distance)
{
  if (CarSpeedLong > 1.0f || Distance > 3.0f || !GetPitstop())
  {
	oPitTimer = 0.0f;
	return false;
  }
  else
  {
	oPitTimer += (float) RCM_MAX_DT_ROBOTS;
	if (oPitTimer > 3.0f)
	{
	  oPitTimer = 0.0f;
	  return true;
	}
	else
	  return false;
  }
}
//==========================================================================*

//==========================================================================*
// Update pit data and strategy.
//--------------------------------------------------------------------------*
void TPit::Update()
{
  if (oMyPit != NULL)
  {
	if (IsBetween(DistanceFromStartLine))
	{
	  if (GetPitstop())
		SetInPit(true);
	}
	else
	  SetInPit(false);

	if (GetPitstop())
	  CarRaceCmd = RM_CMD_PIT_ASKED;

  }
}
//==========================================================================*

//==========================================================================*
// Get speed limit brake
//--------------------------------------------------------------------------*
float TPit::GetSpeedLimitBrake(float SpeedSqr)
{
  return (SpeedSqr-oSpeedLimitSqr)/(oPitSpeedLimitSqr-oSpeedLimitSqr);
}
//==========================================================================*

//==========================================================================*
// Make Path
//--------------------------------------------------------------------------*
void TPitLane::Init(PtCarElt Car)
{
  oCar = Car;
  oPitStopOffset = 0.0;
}
//==========================================================================*

//==========================================================================*
// Smooth Path with pitlane
//--------------------------------------------------------------------------*
void TPitLane::SmoothPitPath 
  (const TParam& Param)
{
  int I;                                         // Loop counter

  //GfOut("\n\n\nMagic smoother is working!\n\n\n");
  int NSEG = oTrack->Count();                    // Number of sections in the path
  int Idx0 = oTrack->IndexFromPos(oPitEntryPos); // First index and
  int Idx1 = oTrack->IndexFromPos(oPitExitPos);  // last index to modify

  // Modify path for use of normal smoothing
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    oPathPoints[I].WToL = oPathPoints[I].WPitToL;
    oPathPoints[I].WToR = oPathPoints[I].WPitToR;
  }

  // Smooth pit path
  //GfOut("SmoothPath ...\n");  
  SmoothPath(oTrack, Param, TClothoidLane::TOptions(0.43f)); 
  //GfOut("... SmoothPath\n");  

}
//==========================================================================*

//==========================================================================*
// Make Path with pitlane
//--------------------------------------------------------------------------*
void TPitLane::MakePath 
  (char* Filename, TClothoidLane* BasePath,
  const TParam& Param, int Index)
{
  // V1.10.001: >>>
  float F[3] = {0.5, 1.0, 0.0};
  // <<< V1.10.001

  const tTrackOwnPit* Pit = CarPit;              // Get my pit
  if (Pit == NULL)                               // If pit is NULL
  {                                              // nothing to do 
	GfOut("\n\nPit = NULL\n\n");                 // here
	return;
  }

  // We have a pit, let us build all we need to use it ...
  int I;                                         // Loop counter
  TCarParam CarParam = Param.oCarParam3;         // Copy parameters
  TLane::SetLane(*BasePath);                     // Copy Pathpoints
  const tTrackPitInfo* PitInfo =                 // Get pit infos
	&oTrack->Track()->pits;

  bool FirstPit = false;                         // Reset flag
  const int NPOINTS = 7;                         // Nbr of points defined for pitlane 
  double X[NPOINTS];                             // X-coordinates
  double Y[NPOINTS];                             // Y-coordinates
  double S[NPOINTS];                             // Directions

  oStoppingDist = Param.Pit.oStoppingDist;       // Distance to brake
  oPitStopOffset = Param.Pit.oLongOffset;        // Offset for fine tuning
  double PitLaneOffset =                         // Offset of the pitlane
	fabs(PitInfo->driversPits->pos.toMiddle)     //   from the middle of the 
	- PitInfo->width;                            //   track

  oCarParam.oScaleBrake =                        // Limit brake to be used
	MIN(1.30,CarParam.oScaleBrake);              //   in pitlane
  oCarParam.oScaleMu =                           // Scale friction estimation
	MIN(1.30,CarParam.oScaleMu);                 //   of pitlane

  // Compute pit spline points along the track defined by TORCS
  X[0] = PitInfo->pitEntry->lgfromstart          // Start of Pitlane defined by TORCS
	  + Param.Pit.oEntryLong;                    //   our own offset along the track
  X[1] = PitInfo->pitStart->lgfromstart;         // Start of speedlimit 
  X[3] = Pit->pos.seg->lgfromstart               // Center of our own pit
	+ Pit->pos.toStart + oPitStopOffset;         //   with own offset along track
  X[2] = X[3] - PitInfo->len;                    // Start enter own pit here
  X[4] = X[3] + PitInfo->len;                    // Leave own pit here
  X[5] = X[1] + PitInfo->nMaxPits * PitInfo->len;// End of speed limit
  X[6] = PitInfo->pitExit->lgfromstart           // End of pitlane defind by TORCS
	+ PitInfo->pitExit->length                   //   and own offset alog track  
	+ Param.Pit.oExitLong;                       

  oPitEntryPos = X[0];                           // Save this values for later
  oPitStartPos = X[1];                           // use
  oPitEndPos   = X[5];
  oPitExitPos  = X[6];
  //GfOut("oPitEntryPos: %g\n",oPitEntryPos);  
  //GfOut("oPitStartPos: %g\n",oPitStartPos);  
  //GfOut("oPitEndPos:  %g\n",oPitEndPos);  
  //GfOut("oPitExitPos: %g\n",oPitExitPos);  

  // Normalizing spline segments to >= 0.0
  for (I = 0; I < NPOINTS; I++)
  {
    X[I] = ToSplinePos(X[I]); 
    S[I] = 0.0;
  }

  // Now the dark side of TORCS ...

  // Fix point for first pit if necessary.
  if (X[2] < X[1])
  {
    FirstPit = true;                             // Hey we use the first pit!
    X[1] = X[2];
  }

  // Fix broken pit exit.
  if (X[6] < X[5])
    X[6] = X[5] + 50.0;

  // Fix point for last pit if necessary.
  if (X[5] < X[4]) 
    X[5] = X[4];

  // Splice entry/exit of pit path into the base path provided.
  TLanePoint Point;                              // Data of the point
  BasePath->GetLanePoint(oPitEntryPos, Point);   // as defined by TORCS 
  Y[0] = Point.Offset;                           // Lateral distance
  S[0] = -tan(Point.Angle                        // Direction of our
	- oTrack->ForwardAngle(oPitEntryPos));       //   basic racingline

  if (oPitExitPos < oTrack->Length())            // Use normalized position
    BasePath->GetLanePoint(oPitExitPos, Point);  //   to get the data
  else                                               
    BasePath->GetLanePoint(oPitExitPos-oTrack->Length(), Point);

  Y[6] = Point.Offset;                           // Lateral distance and
  S[6] = -tan(Point.Angle                        // and direction at the
	- oTrack->ForwardAngle(oPitExitPos));        // TORCS end of pitlane

  int Sign =                                     // Get the side of pits
	(PitInfo->side == TR_LFT) ? -1 : 1;    
  
  // V1.10.001: >>>
  if (Sign < 0) // If pits are on the left side
  {
    F[1] = 0.0; // swap
    F[2] = 1.0;
  }
  // <<< V1.10.001
  
  // First use a generic path through the pitlane without the pit itself
  if (Param.Pit.oUseFirstPit && FirstPit)        // If allowed and possible
  {                                              // we will use a special path to
	Y[3] = Y[2] = Y[1] = Sign *                  // the first pit with
	  (fabs(PitInfo->driversPits->pos.toMiddle)  //   TORCS defined offset
	  + Param.Pit.oLatOffset);                   //   and our own lateral offset
  }
  else                                           // All other pits
  {                                              // have to be reached over
    Y[3] = Y[2] = Y[1] = Sign *                  // a path defined here
	  (PitLaneOffset -                           // Sign gives the side of the pits
	  Param.Pit.oLaneEntryOffset * F[Index]);    // and we correct the TORCS offset 
  }

  Y[5] = Y[4] = Sign *                           // Leaving the own pit, we will
    (PitLaneOffset                               //   go to the pitlane and use
    - Param.Pit.oLaneExitOffset * F[Index]);     //   an additional offset

  TCubicSpline PreSpline(NPOINTS, X, Y, S);      // Calculate the splines

  // Modify points in line path for pits ...
  int NSEG = oTrack->Count();                    // Number of sections in the path

  // Start at section with speedlimit
  int Idx0 = oTrack->IndexFromPos(oPitStartPos); // Index to first point
  // Section with offset of 5 m, keep it's position         
  Idx0 = (Idx0 + 5) % NSEG;                      // Index to first point + 5 m
  int Idx1 = oTrack->IndexFromPos(oPitEndPos);   // Index to last point
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)  // Set Flag, to keep the points
	oPathPoints[I].Fix = true;

  // Pit entry and pit exit as defined by TORCS
  Idx0 = (1 + oTrack->IndexFromPos(oPitEntryPos)) % NSEG;
  Idx1 = oTrack->IndexFromPos(oPitExitPos) % NSEG;

  // Change offsets to go to the pitlane
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    double Station =                             // Station in spline coordinates
	  ToSplinePos(oTrack->Section(I).DistFromStart);
    oPathPoints[I].Offset =                      // Offset lateral to track
	  PreSpline.CalcOffset(Station); 
    oPathPoints[I].Point =                       // Recalculate point coordinates
	  oPathPoints[I].CalcPt();                   //   from offset
  }

  if (Param.Pit.oUseSmoothPit > 0)               // Smooth path at pit entry
  {
	if (FirstPit)   
	{
      int Len = strlen(Filename);
	  Filename[Len - 2] = '1'; 
	}
	else
	{
      int Len = strlen(Filename);
	  Filename[Len - 2] = 'p'; 
	}
	if (!LoadPointsFromFile(Filename)) 
	{
      GfOut("SmoothPitPath ...\n");  
      SmoothPitPath(Param);                        // and pit exit
	  SavePointsToFile(Filename);
      GfOut("... SmoothPitPath\n");  
	}
    // Find section with different path at start (different offsets)
    Idx0 = oTrack->IndexFromPos(oPitEntryPos);
    while (Dist(oPathPoints[Idx0].Point,BasePath->PathPoints(Idx0).Point) > 0.01)
      Idx0 = (Idx0 + NSEG - 1) % NSEG;

    // Find section with different path at end (different offsets)
    Idx1 = oTrack->IndexFromPos(oPitExitPos);
    while (Dist(oPathPoints[Idx1].Point,BasePath->PathPoints(Idx1).Point) > 0.01) 
      Idx1 = (Idx1 + 1) % NSEG;
  }

  // Save the positions of the new pit entry and pit exit
  oPitEntryPos = oPathPoints[Idx0].Dist();
  oPitExitPos = oPathPoints[Idx1].Dist();
  for (I = 0; I < NPOINTS; I++)
    X[I] = ToSplinePos(X[I]);

  // Now we use the complete pitlane with the pit itself
  if (!Param.Pit.oUseFirstPit || !FirstPit)      // If needed
  {                                              // have to be reached over
    Y[3] = Sign *                                // To compensate steering we use 
	  (fabs(PitInfo->driversPits->pos.toMiddle)  // as target a point beeing a little 
	  + Param.Pit.oLatOffset + 0.5);             // in the pit
  }

  TCubicSpline Spline(NPOINTS, X, Y, S);         // Calculate the splines 

  // Section with speedlimit, keep it         
  Idx0 = oTrack->IndexFromPos(oPitStartPos);     // Index to first point
  //if (Param.Pit.oUseFirstPit && FirstPit)        // If needed
    Idx0 = (Idx0 + 5) % NSEG;                    // Index to first point + 5 m
//  else
//    Idx0 = (Idx0 + 5) % NSEG;                    // Index to first point + 5 m
//    Idx0 = (Idx0 - 5 + NSEG) % NSEG;             // Index to first point - 5 m
  Idx1 = oTrack->IndexFromPos(oPitEndPos);       // Index to last point
 
  // Change offsets to go to the pitlane 
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    double Station =                             // Station in spline coordinates
	  ToSplinePos(oTrack->Section(I).DistFromStart);
    oPathPoints[I].Offset =                      // Offset lateral to track
	  Spline.CalcOffset(Station); 
    oPathPoints[I].Point =                       // Recalculate point coordinates
	  oPathPoints[I].CalcPt();                   //   from offset
    oPathPoints[I].Fix = true;
  }

  // Prepare speed calculation with changed path
  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);
  CalcCurvaturesXY();
  CalcCurvaturesZ();  
  CalcMaxSpeeds(1);

  // Overwrite speed calculations at section with speed limit
  //Idx0 = (oTrack->IndexFromPos(oPitStartPos) + NSEG - 1) % NSEG;
  Idx0 = (oTrack->IndexFromPos(oPitStartPos) + NSEG - 5) % NSEG;
  Idx1 = (oTrack->IndexFromPos(oPitEndPos) + 1) % NSEG;

  // Allowed speed in pitlane itself
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    double Speed = MIN(oPathPoints[I].Speed, PitInfo->speedLimit - 0.5);
    oPathPoints[I].MaxSpeed = oPathPoints[I].Speed = Speed;
  }

  // Save stop position
  double StopPos = Pit->pos.seg->lgfromstart     // As defined by TROCS
	+ Pit->pos.toStart + oPitStopOffset;         // with own offset along track
  if (StopPos >= oTrack->Length())               // Normalize it to be >= 0.0
    StopPos -= oTrack->Length();
  else if (StopPos < 0)
    StopPos += oTrack->Length();

  // Sections at pit stop
  Idx0 = oTrack->IndexFromPos(StopPos);
  oStopIdx = Idx0;
  oStopPos = StopPos;
  oPitStopPos = oPathPoints[Idx0].Dist();

  //Distance of pit entry to pit stop point
  oPitDist = oPitStopPos - oPitEntryPos;
  if (oPitDist < 0) 
    oPitDist += oTrack->Length();

  // Set speed to allow to stop 
  float Speed = 4.0;
  oPathPoints[Idx0].MaxSpeed = oPathPoints[Idx0].Speed = Speed;
  Idx1 = (Idx0 + NSEG - 1) % NSEG;
  for (I = 0; I < 6; I++)
  {
    Speed += 0.75;
    oPathPoints[Idx1].MaxSpeed = oPathPoints[Idx1].Speed = Speed;
    Idx1 = (Idx1 + NSEG - 1) % NSEG;
  }

  // Set speed to restart
  for (I = 0; I < 15; I++)
  {
    Idx0 = (Idx0 + 1) % NSEG;
    oPathPoints[Idx0].MaxSpeed = oPathPoints[Idx0].Speed = PitInfo->speedLimit - 0.5;
  }

  // Calculate braking
  PropagatePitBreaking(oPitStopPos,Param.oCarParam.oScaleMu);
}
//==========================================================================*

//==========================================================================*
// Are we in Pit Section?
//--------------------------------------------------------------------------*
bool TPitLane::InPitSection(double TrackPos) const
{
  TrackPos = ToSplinePos(TrackPos);
  return oPitEntryPos < TrackPos && TrackPos < ToSplinePos(oPitExitPos);
}
//==========================================================================*

//==========================================================================*
// Check wether we can stop in pit
//--------------------------------------------------------------------------*
bool TPitLane::CanStop(double TrackPos) const
{
  double D = DistToPitStop(TrackPos, true);
  if ((D < oStoppingDist) || (oTrack->Length() - D < oStoppingDist))
    return true;
  else
    return false;
}
//==========================================================================*

//==========================================================================*
// Check wether we overrun stop pos
//--------------------------------------------------------------------------*
bool TPitLane::Overrun(double TrackPos) const
{
  double D = DistToPitStop(TrackPos, true);
  if ((D > oTrack->Length() / 2) && (oTrack->Length() - D > oStoppingDist))
	return true;
  else
    return false;
}
//==========================================================================*

//==========================================================================*
// Get Distance to Pit entry
//--------------------------------------------------------------------------*
double TPitLane::DistToPitEntry(double TrackPos) const
{
  double Dist = oPitEntryPos - TrackPos;
  if (Dist < 0)
	Dist += oTrack->Length();
  return Dist;
}
//==========================================================================*

//==========================================================================*
// Get Distance to Pit entry
//--------------------------------------------------------------------------*
double TPitLane::DistToPitStop(double TrackPos, bool Pitting) const
{
  double Dist;
  float DL, DW;

  if (Pitting)
  {
//	  dist = oPitStopPos - trackPos;
    RtDistToPit(oCar,oTrack->Track(),&DL,&DW);
    DL += (float)(oPitStopOffset - TRACKRES / 2);
//	  GfOut("DistToPitStop: %g-%g=%g\n",dist,DL,dist-DL);
    Dist = DL;
  	if (Dist < 0)
	  Dist += oTrack->Length();
  }
  else
  {
	Dist = oPitStopPos - oPitEntryPos;
	if (Dist < 0)
	  Dist += oTrack->Length();
	  Dist += DistToPitEntry(TrackPos);
  }
  return Dist;
}
//==========================================================================*

//==========================================================================*
// Calculate local position
//--------------------------------------------------------------------------*
double TPitLane::ToSplinePos(double TrackPos) const
{
  if (TrackPos < oPitEntryPos)
	TrackPos += oTrack->Length();
  return TrackPos;
}
//--------------------------------------------------------------------------*
// end of file unitpit.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
