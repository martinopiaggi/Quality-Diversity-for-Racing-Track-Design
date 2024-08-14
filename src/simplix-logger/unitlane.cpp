//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitlane.cpp 
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Fahrspur
//
// Datei    : unitlane.cpp
// Erstellt : 25.11.2007
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
#include <robottools.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"
#include "unitlane.h"
#include "unitlinalg.h"
#include "unittmpcarparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TLane::TLane():
  oTrack(NULL),
  oPathPoints(NULL)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TLane::~TLane()
{
  delete [] oPathPoints;
}
//==========================================================================*

//==========================================================================*
// Set operator (Sets lane)
//--------------------------------------------------------------------------*
TLane& TLane::operator= (const TLane& Lane)
{
  SetLane(Lane);
  return *this;
}
//==========================================================================*

//==========================================================================*
// Set lane
//--------------------------------------------------------------------------*
void TLane::SetLane(const TLane& Lane)
{
  oTrack = Lane.oTrack;
  oFixCarParam = Lane.oFixCarParam;
  oCarParam = Lane.oCarParam;

  const int Count = oTrack->Count();

  delete [] oPathPoints;
  oPathPoints = new TPathPt[Count];

  memcpy(oPathPoints, Lane.oPathPoints, Count * sizeof(*oPathPoints));
}
//==========================================================================*

//==========================================================================*
// Check wether position is in lane
//--------------------------------------------------------------------------*
bool TLane::ContainsPos(double TrackPos) const
{
  if (TrackPos > 0.0)
    return true;                                 // Allways true because
  else                                           // this lane type
    return true;                                 // contains all points
}
//==========================================================================*

//==========================================================================*
// Get information to the point nearest the given position
//--------------------------------------------------------------------------*
bool TLane::GetLanePoint(double TrackPos, TLanePoint& LanePoint) const
{
  int Count = oTrack->Count();

  int Idx0 = oTrack->IndexFromPos(TrackPos);
  int Idxp = (Idx0 - 1 + Count) % Count;
  int Idx1 = (Idx0 + 1) % Count;
  int Idx2 = (Idx0 + 2) % Count;

  double Dist0 = oPathPoints[Idx0].Dist();
  double Dist1 = oPathPoints[Idx1].Dist();
  if (Idx1 == 0)
    Dist1 = oTrack->Length();

  TVec3d P0 = oPathPoints[Idxp].CalcPt();
  TVec3d P1 = oPathPoints[Idx0].CalcPt();
  TVec3d P2 = oPathPoints[Idx1].CalcPt();
  TVec3d P3 = oPathPoints[Idx2].CalcPt();

  double Crv1 = TUtils::CalcCurvatureXY(P0, P1, P2);
  double Crv2 = TUtils::CalcCurvatureXY(P1, P2, P3);

  double Tx = (TrackPos - Dist0) / (Dist1 - Dist0);

  LanePoint.Index = Idx0;
  LanePoint.Crv = (1.0 - Tx) * Crv1 + Tx * Crv2;;
  LanePoint.T = Tx;
  LanePoint.Offset = 
	(oPathPoints[Idx0].Offset)
	+ Tx * (oPathPoints[Idx1].Offset - oPathPoints[Idx0].Offset);

  double Ang0 = TUtils::VecAngXY(oPathPoints[Idx1].CalcPt() -
	oPathPoints[Idx0].CalcPt());
  double Ang1 = TUtils::VecAngXY(oPathPoints[Idx2].CalcPt() -
	oPathPoints[Idx1].CalcPt());

  double DeltaAng = Ang1 - Ang0;
  DOUBLE_NORM_PI_PI(DeltaAng);
  LanePoint.Angle = Ang0 + LanePoint.T * DeltaAng;

  TVec2d Tang1, Tang2;
  TUtils::CalcTangent(P0.GetXY(), P1.GetXY(), P2.GetXY(), Tang1);
  TUtils::CalcTangent(P1.GetXY(), P2.GetXY(), P3.GetXY(), Tang2);
  TVec2d Dir = TUtils::VecUnit(Tang1) * (1 - Tx) + TUtils::VecUnit(Tang2) * Tx;

  Ang0 = TUtils::VecAngle(Tang1);
  Ang1 = TUtils::VecAngle(Tang2);
  DeltaAng = Ang1 - Ang0;
  DOUBLE_NORM_PI_PI(DeltaAng);

  LanePoint.Speed = oPathPoints[LanePoint.Index].Speed + (oPathPoints[Idx1].Speed
	- oPathPoints[LanePoint.Index].Speed) * LanePoint.T;
  LanePoint.AccSpd = oPathPoints[LanePoint.Index].AccSpd + (oPathPoints[Idx1].AccSpd
	- oPathPoints[LanePoint.Index].AccSpd) * LanePoint.T;

  return true;
}
//==========================================================================*

//==========================================================================*
// Initialize lane from track limiting width to left and right
//--------------------------------------------------------------------------*
void TLane::Initialise
  (TTrackDescription* Track,
  const TFixCarParam& FixCarParam,
  const TCarParam& CarParam,
  double MaxLeft, double MaxRight)
{
  delete [] oPathPoints;
  oTrack = Track;
  oPathPoints = new TPathPt[Track->Count()];
  oCarParam = CarParam;                          // Copy car params
  oFixCarParam = FixCarParam;                    // Copy car params
 
  if (MaxLeft < 999.0) 
  {
    for (int I = 0; I < Track->Count(); I++)
    {
	  const TSection& Sec = (*oTrack)[I];	     
	  oPathPoints[I].Sec = &Sec;
	  oPathPoints[I].Center = Sec.Center; 
	  oPathPoints[I].Crv = 0;
	  oPathPoints[I].CrvZ	= 0;
	  oPathPoints[I].Offset = 0.0;
	  oPathPoints[I].Point = oPathPoints[I].CalcPt();
	  oPathPoints[I].MaxSpeed	= 10;
	  oPathPoints[I].Speed = 10;
	  oPathPoints[I].AccSpd = 10;
	  oPathPoints[I].FlyHeight = 0;
	  oPathPoints[I].BufL	= 0;
	  oPathPoints[I].BufR	= 0;
	  oPathPoints[I].NextCrv = 0.0;
	  oPathPoints[I].WToL = MaxLeft;
  	  oPathPoints[I].WToR = Sec.WidthToRight;
	  oPathPoints[I].WPitToL = Sec.PitWidthToLeft;
      oPathPoints[I].WPitToR = Sec.PitWidthToRight;
	  oPathPoints[I].Fix = false;
    }
  }
  else if (MaxRight < 999.0) 
  {
    for (int I = 0; I < Track->Count(); I++)
    {
	  const TSection& Sec = (*oTrack)[I];	 	 
	  oPathPoints[I].Sec = &Sec;
	  oPathPoints[I].Center	= Sec.Center; 
	  oPathPoints[I].Crv = 0;
	  oPathPoints[I].CrvZ = 0;
	  oPathPoints[I].Offset = 0.0;
	  oPathPoints[I].Point = oPathPoints[I].CalcPt();
	  oPathPoints[I].MaxSpeed = 10;
	  oPathPoints[I].Speed = 10;
	  oPathPoints[I].AccSpd	= 10;
	  oPathPoints[I].FlyHeight = 0;
	  oPathPoints[I].BufL = 0;
	  oPathPoints[I].BufR = 0;
	  oPathPoints[I].NextCrv = 0.0;
	  oPathPoints[I].WToL = Sec.WidthToLeft;
	  oPathPoints[I].WToR = MaxRight;
      oPathPoints[I].WPitToL = Sec.PitWidthToLeft;
      oPathPoints[I].WPitToR = Sec.PitWidthToRight;
	  oPathPoints[I].Fix = false;
    }
  }
  else 
  {
    for (int I = 0; I < Track->Count(); I++)
    {
	  const TSection& Sec = (*oTrack)[I];		 
	  oPathPoints[I].Sec = &Sec;
	  oPathPoints[I].Center = Sec.Center;
	  oPathPoints[I].Crv = 0;
	  oPathPoints[I].CrvZ	= 0;
	  oPathPoints[I].Offset = 0.0;
	  oPathPoints[I].Point = oPathPoints[I].CalcPt();
	  oPathPoints[I].MaxSpeed	= 10;
	  oPathPoints[I].Speed = 10;
	  oPathPoints[I].AccSpd = 10;
	  oPathPoints[I].FlyHeight = 0;
	  oPathPoints[I].BufL	= 0;
	  oPathPoints[I].BufR	= 0;
	  oPathPoints[I].NextCrv = 0.0;
      oPathPoints[I].WToL = Sec.WidthToLeft;
	  oPathPoints[I].WToR = Sec.WidthToRight;
      oPathPoints[I].WPitToL = Sec.PitWidthToLeft;
      oPathPoints[I].WPitToR = Sec.PitWidthToRight;
	  oPathPoints[I].Fix = false;
    }
  }
  CalcCurvaturesXY();
  CalcCurvaturesZ();
}
//==========================================================================*

//==========================================================================*
// Get path point from index
//--------------------------------------------------------------------------*
const TLane::TPathPt& TLane::PathPoints(int Index) const
{
  return oPathPoints[Index];
}
//==========================================================================*

//==========================================================================*
// Calc curvature in XY
//--------------------------------------------------------------------------*
void TLane::CalcCurvaturesXY(int Start, int Step)
{
  const int N = oTrack->Count();

  for (int I = 0; I < N; I++)
  {
	int	P  = (Start + I) % N;                    // Point
	int	Pp = (P - Step + N) % N;                 // Prev Point
	int	Pn = (P + Step) % N;                     // Next Point

	oPathPoints[P].Crv =
	  TUtils::CalcCurvatureXY(
	    oPathPoints[Pp].CalcPt(),
	    oPathPoints[P].CalcPt(),
 	    oPathPoints[Pn].CalcPt());
  }
}
//==========================================================================*

//==========================================================================*
// Calc curvature in Z
//--------------------------------------------------------------------------*
void TLane::CalcCurvaturesZ(int Start, int Step)
{
  const int N = oTrack->Count();

  Step *= 3;

  for (int I = 0; I < N; I++)
  {
	int	P  = (Start + I) % N;                    // Point
	int	Pp = (P - Step + N) % N;                 // Prev Point
	int	Pn = (P + Step) % N;                     // Next Point

	oPathPoints[P].CrvZ = 6 * TUtils::CalcCurvatureZ(
	  oPathPoints[Pp].CalcPt(),
      oPathPoints[P].CalcPt(),
	  oPathPoints[Pn].CalcPt());
  }
}
//==========================================================================*

//==========================================================================*
// Calc max possible speed depending on car modell
//--------------------------------------------------------------------------*
void TLane::CalcMaxSpeeds
  (int Start, int Len, int Step)
{
  const int N = oTrack->Count();

  for (int I = 0; I < Len; I += Step)
  {
	int P = (Start + I) % N;
	int Q = (P + 1) % N;

    double TrackRollAngle = atan2(oPathPoints[P].Norm().z, 1);

	double Speed = oFixCarParam.CalcMaxSpeed(
      oCarParam,
      oPathPoints[P].Crv,
      oPathPoints[Q].Crv,
	  oPathPoints[P].CrvZ,
	  oTrack->Friction(P),
  	  TrackRollAngle);

	oPathPoints[P].MaxSpeed = Speed;
	oPathPoints[P].Speed = Speed;
	oPathPoints[P].AccSpd = Speed;
  }
}
//==========================================================================*

//==========================================================================*
// Propagate braking
//--------------------------------------------------------------------------*
void TLane::PropagateBreaking
  (int Start, int Len, int Step)
{
  const int N = oTrack->Count();

  for (int I = Step * ((2*Len - 1) / Step); I >= 0; I -= Step )
  {
	int	P = (Start + I) % N;
	int Q = (P + Step) % N;

	if (oPathPoints[P].Speed > oPathPoints[Q].Speed)
	{
	  // see if we need to adjust spd[i] to make it possible
	  //   to slow to spd[j] by the next seg.
      TVec3d Delta = oPathPoints[P].CalcPt() - oPathPoints[Q].CalcPt();
      double Dist = TUtils::VecLenXY(Delta);
      double K = (oPathPoints[P].Crv + oPathPoints[Q].Crv) * 0.5;
	  if (fabs(K) > 0.0001)
	    Dist = 2 * asin(0.5 * Dist * K) / K;
	  double TrackRollAngle = atan2(oPathPoints[P].Norm().z, 1);

	  double U = oFixCarParam.CalcBraking(
        oCarParam,
  		oPathPoints[P].Crv,
		oPathPoints[P].CrvZ,
		oPathPoints[Q].Crv,
		oPathPoints[Q].CrvZ,
		oPathPoints[Q].Speed,
		Dist,
		oTrack->Friction(P),
		TrackRollAngle);

	  if (oPathPoints[P].Speed > U)
		oPathPoints[P].Speed = oPathPoints[P].AccSpd = U;

	  if (oPathPoints[P].FlyHeight > 0.1)
		oPathPoints[P].Speed = oPathPoints[Q].Speed;

	}
  }
}
//==========================================================================*

//==========================================================================*
// Propagate braking
//--------------------------------------------------------------------------*
void TLane::PropagatePitBreaking
  (int Start, int Len, float PitStopPos, float ScaleMu)
{
  const float base = 0.5f; 
  int Step = 1;
  const int N = oTrack->Count();

  for (int I = Step * ((2*Len - 1) / Step); I >= 0; I -= Step )
  {
	int	P = (Start + I) % N;
	int Q = (P + Step) % N;

	if (oPathPoints[P].Speed > oPathPoints[Q].Speed)
	{
	  // see if we need to adjust spd[i] to make it possible
	  //   to slow to spd[j] by the next seg.
      TVec3d Delta = oPathPoints[P].CalcPt() - oPathPoints[Q].CalcPt();
      double Dist = TUtils::VecLenXY(Delta);
      double K = (oPathPoints[P].Crv + oPathPoints[Q].Crv) * 0.5;
	  if (fabs(K) > 0.0001)
	    Dist = 2 * asin(0.5 * Dist * K) / K;
	  double TrackRollAngle = atan2(oPathPoints[P].Norm().z, 1);

	  double Factor = 1.0 - MIN(1.0,fabs(oPathPoints[Q].Dist() - PitStopPos) / oFixCarParam.oPitBrakeDist);
	  double Friction = oTrack->Friction(P) * (Factor * ScaleMu + (1 - Factor) * oCarParam.oScaleBrakePit);

	  double U = oFixCarParam.CalcBraking(
        oCarParam,
  		oPathPoints[P].Crv,
		oPathPoints[P].CrvZ,
		oPathPoints[Q].Crv,
		oPathPoints[Q].CrvZ,
		oPathPoints[Q].Speed,
		Dist,
		Friction,
		TrackRollAngle);

	  if (oPathPoints[P].Speed > U)
		oPathPoints[P].Speed = oPathPoints[P].AccSpd = U;

	  //GfOut("I:%d P:%d Q:%d ID:%d F:%g U:%g S:%g\n",I,P,Q,ID,Factor,U,oPathPoints[P].Speed);

	  if (oPathPoints[P].FlyHeight > 0.1)
		oPathPoints[P].Speed = oPathPoints[Q].Speed;
	}
  }
}
//==========================================================================*

//==========================================================================*
// Propagate acceleration
//--------------------------------------------------------------------------*
void TLane::PropagateAcceleration
  (int Start, int Len, int Step)
{
  const int N = oTrack->Count();

  for (int I = 0; I < 2*Len; I += Step )
  {
	int Q = (Start + I + N) % N;
	int	P = (Q - Step + N) % N;

	if (Q == 0)
	  P = (N - 3);

	if (oPathPoints[P].AccSpd < oPathPoints[Q].AccSpd)
	{
	  // see if we need to adjust spd[Q] to make it possible
	  //   to speed up to spd[P] from spd[Q].
  	  double Dist = TUtils::VecLenXY(
		oPathPoints[P].CalcPt() - oPathPoints[Q].CalcPt());

	  double K = (oPathPoints[P].Crv + oPathPoints[Q].Crv) * 0.5;
	  if (fabs(K) > 0.0001)
	    Dist = 2 * asin(0.5 * Dist * K) / K;

	  double TrackRollAngle = atan2(oPathPoints[P].Norm().z, 1);

	  double V = oFixCarParam.CalcAcceleration(
	    oPathPoints[P].Crv,
		oPathPoints[P].CrvZ,
		oPathPoints[Q].Crv,
		oPathPoints[Q].CrvZ,
		oPathPoints[P].AccSpd,
		Dist,
		oTrack->Friction(P),
		TrackRollAngle);
		

		//if (oPathPoints[Q].AccSpd > V)
		oPathPoints[Q].AccSpd = MIN(V,oPathPoints[Q].Speed);
	}
  }
}
//==========================================================================*

//==========================================================================*
// Calculate curvature in XY
//--------------------------------------------------------------------------*
void TLane::CalcCurvaturesXY(int Step)
{
  CalcCurvaturesXY(0, Step);
}
//==========================================================================*

//==========================================================================*
// Calculate curvature in Z
//--------------------------------------------------------------------------*
void TLane::CalcCurvaturesZ(int Step)
{
  CalcCurvaturesZ( 0, Step);
}
//==========================================================================*

//==========================================================================*
// Calculate max possible speed
//--------------------------------------------------------------------------*
void TLane::CalcMaxSpeeds(int Step)
{
  CalcMaxSpeeds(0, oTrack->Count(), Step);
}
//==========================================================================*

//==========================================================================*
// Propagate breaking
//--------------------------------------------------------------------------*
void TLane::PropagateBreaking
  (int Step)
{
  PropagateBreaking( 0, oTrack->Count(), Step);
}
//==========================================================================*

//==========================================================================*
// Propagate breaking
//--------------------------------------------------------------------------*
void TLane::PropagatePitBreaking
  (float PitStopPos, float ScaleMu)
{
  PropagatePitBreaking( 0, oTrack->Count(), PitStopPos, ScaleMu);
}
//==========================================================================*

//==========================================================================*
// Propagate acceleration
//--------------------------------------------------------------------------*
void TLane::PropagateAcceleration
  (int Step)
{
  PropagateAcceleration( 0, oTrack->Count(), Step);
}
//==========================================================================*

//==========================================================================*
// Calculate forward absolute curvature
//--------------------------------------------------------------------------*
void TLane::CalcFwdAbsCrv(int Range, int Step)
{
  const int	N = oTrack->Count() - 1;

  int Count = Range / Step;
  int P = Count * Step;
  int Q = P;
  double TotalCrv = 0;

  while (P > 0)
  {
	TotalCrv += oPathPoints[P].Crv;
	P -= Step;
  }

  oPathPoints[0].NextCrv = TotalCrv / Count;
  TotalCrv += fabs(oPathPoints[0].Crv);
  TotalCrv -= fabs(oPathPoints[Q].Crv);

  P = (N / Step) * Step;
  Q -= Step;
  if (Q < 0)
    Q = (N / Step) * Step;

  while (P > 0)
  {
	oPathPoints[P].NextCrv = TotalCrv / Count;
	TotalCrv += fabs(oPathPoints[P].Crv);
	TotalCrv -= fabs(oPathPoints[Q].Crv);

	P -= Step;
	Q -= Step;
	if (Q < 0)
	  Q = (N / Step) * Step;
  }
}
//==========================================================================*

//==========================================================================*
// Calculate estimated time
//--------------------------------------------------------------------------*
double TLane::CalcEstimatedTime(int Start, int Len) const
{
  double TotalTime = 0;

  const int N = oTrack->Count();
  for (int I = 0; I < Len; I++)
  {
	int	P = (Start + I) % N;
	int	Q = (P + 1) % N;
	double Dist = TUtils::VecLenXY(
	  oPathPoints[P].CalcPt() - oPathPoints[Q].CalcPt());

	TotalTime += Dist / ((oPathPoints[P].AccSpd + oPathPoints[Q].AccSpd) * 0.5);
  }

  return TotalTime;
}
//==========================================================================*

//==========================================================================*
// Calculate estimated lap time
//--------------------------------------------------------------------------*
double	TLane::CalcEstimatedLapTime() const
{
  double LapTime = 0;

  const int N = oTrack->Count();
  for (int I = 0; I < N; I++)
  {
	int	Q = (I + 1) % N;
	double Dist = TUtils::VecLenXY(
	  oPathPoints[I].CalcPt() - oPathPoints[Q].CalcPt());
	LapTime += Dist / ((oPathPoints[I].AccSpd + oPathPoints[Q].AccSpd) * 0.5);
  }

  return LapTime;
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitlane.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
