//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitclothoid.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Fahrspur clothoidenähnlich
//
// Datei    : unitclothoid.cpp
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
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitclothoid.h"
#include "unitlinreg.h"
#include "unitparam.h"
#include "unitsysfoo.h"
#include "unitsection.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TClothoidLane::TClothoidLane()
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TClothoidLane::~TClothoidLane()
{
}
//==========================================================================*

//==========================================================================*
// Create a smooth lane
//--------------------------------------------------------------------------*
void TClothoidLane::MakeSmoothPath(
  TTrackDescription* Track,
  TParam& Param,
  const TOptions& Opts)
{
  TCarParam& CarParam = Param.oCarParam;
  if (Opts.Side)
  {
    //GfOut("Switch to CarParam2\n");
    CarParam = Param.oCarParam2;
  }
  TLane::Initialise(Track, Param.Fix, CarParam, Opts.MaxL, Opts.MaxR);
  const int Count = Track->Count();

  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);

  const int Delta = 25;
  const int L = 8;

  int Step = 1;                                  // Initialize step width
  while (Step * 16 < Count)                      // Find largest step width
    Step *= 2;

  //GfOut("OptimisePath:\n");
  while (Step > 0)
  {
	//GfOut("Step: %d\n",Step);
 	for (int I = 0; I < L; I++)
	{
	  OptimisePath(Step, Delta, 0, Opts.Smooth);
	}
    Step >>= 1;
  } 
  if (Opts.BumpMod)
  {
    //GfOut("AnalyseBumps:\n");
	AnalyseBumps(false);
	//AnalyseBumps(true);

	Step = 1;
	Step <<= ANALYSE_STEPS;
	while (Step > 0)
	{
	  //GfOut("Step: %d\n",Step);
  	  for (int I = 0; I < L; I++)
	  {
		OptimisePath(Step, Delta, Opts.BumpMod, Opts.Smooth);
	    CalcCurvaturesZ();
		CalcFwdAbsCrv(FwdRange);
		CalcMaxSpeeds(Step);
		PropagateBreaking(Step);
		PropagateAcceleration(Step);
	  }
      Step >>= 1;
	}
  }
  else
  {
	Step = 1;
    CalcCurvaturesZ();
	CalcMaxSpeeds(Step);
	PropagateBreaking(Step);
	PropagateAcceleration(Step);
  }
}
//==========================================================================*

//==========================================================================*
// Load a smooth lane
//--------------------------------------------------------------------------*
bool TClothoidLane::LoadSmoothPath(
  char* TrackLoad,
  TTrackDescription* Track,
  TParam& Param,
  const TOptions& Opts)
{
  TCarParam& CarParam = Param.oCarParam;
  if (Opts.Side)
    CarParam = Param.oCarParam2;
  TLane::Initialise(Track, Param.Fix, CarParam, Opts.MaxL, Opts.MaxR);
  return LoadPointsFromFile(TrackLoad);
}
//==========================================================================*

//==========================================================================*
// Smooth a pitlane
//--------------------------------------------------------------------------*
void TClothoidLane::SmoothPath(
  TTrackDescription* Track,
  const TParam& Param,
  const TOptions& Opts)
{
  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);

  const int Delta = 25;
  const int L = 8;

  int Step = 1;
  Step <<= ANALYSE_STEPS;
  while (Step > 0)
  {
    //GfOut("Step: %d\n",Step);
    for (int I = 0; I < L; I++)
	{
	  OptimisePath(Step, Delta, Opts.BumpMod, Opts.Smooth);
	  CalcCurvaturesZ();
	  CalcFwdAbsCrv(FwdRange);
	  CalcMaxSpeeds(Step);
	  PropagateBreaking(Step);
	  PropagateAcceleration(Step);
	}
    Step >>= 1;
  }
}
//==========================================================================*

//==========================================================================*
// Analyse lane to find dangerous bumps
//--------------------------------------------------------------------------*
void TClothoidLane::AnalyseBumps(bool DumpInfo)
{
  // Here we look at the bumps on the track, and increase the buffers
  // from the edge after the bumps

  // Get height profile 
  CalcCurvaturesZ();
  // Get an estimate of speeds
  CalcMaxSpeeds();
  PropagateBreaking();
  PropagateAcceleration();

  const int Count = oTrack->Count();

  double Sz = oPathPoints[0].Point.z;
  double Vz = 0;
  double Pz = Sz;

  int I;
  int J;
  int K;
  for (I = 0; I < 2; I++)
  {
	K = Count - 1;

	for (J = 0; J < Count; J++)
	{
	  double OldPz = Pz;

	  double V = (oPathPoints[J].AccSpd + oPathPoints[K].AccSpd) * 0.5;
	  double S = TUtils::VecLenXY(oPathPoints[J].Point - oPathPoints[K].Point);
	  double Dt = S / V;

	  Pz = oPathPoints[J].Point.z;
	  Sz += Vz * Dt - 0.5 * G * Dt * Dt;
	  Vz -= G * Dt;

	  if (Sz <= Pz)
	  {
		double NewVz = (Pz - OldPz) / Dt;
		if (Vz < NewVz)
		  Vz = NewVz;
		Sz = Pz;
	  }

	  oPathPoints[J].FlyHeight = Sz - Pz;

	  if ((I == 1) && DumpInfo)
	  {
		GfOut( "%4d v %3.0f crv %7.4f dt %.3f pz %5.2f sz %5.2f vz %5.2f -> h %5.2f\n",
		  J, oPathPoints[J].AccSpd * 3.6, oPathPoints[J].Crv, Dt,
		  Pz, Sz, Vz, oPathPoints[J].FlyHeight);
	  }
	  K = J;
	}
  }

  for (I = 0; I < 3; I++)
  {
    for (J = 0; J < Count; J++)
	{
	  K = (J + 1) % Count;
	  if (oPathPoints[J].FlyHeight < oPathPoints[K].FlyHeight)
	    oPathPoints[J].FlyHeight = oPathPoints[K].FlyHeight;
	}
  }
}
//==========================================================================*

//==========================================================================*
// Smooth points between steps
//--------------------------------------------------------------------------*
void TClothoidLane::SmoothBetween(int Step, double BumpMod)
{
  const int Count = oTrack->Count();

  // Smooth values between Steps
  TPathPt* L0 = 0;
  TPathPt* L1 = &oPathPoints[((Count - 1) / Step) * Step];
  TPathPt* L2 = &oPathPoints[0];
  TPathPt* L3 = &oPathPoints[Step];

  int J = 2 * Step;
  for (int I = 0; I < Count; I += Step)
  {
	L0 = L1;
	L1 = L2; 
	L2 = L3;
	L3 = &oPathPoints[J];

	J += Step;
	if (J >= Count)
	  J = 0;

	TVec3d P0 = L0->Point;
	TVec3d P1 = L1->Point;
	TVec3d P2 = L2->Point;
	TVec3d P3 = L3->Point;

	double Crv1 = TUtils::CalcCurvatureXY(P0, P1, P2);
	double Crv2 = TUtils::CalcCurvatureXY(P1, P2, P3);

	if (I + Step > Count)
	  Step = Count - I;

	for (int K = 1; K < Step; K++)
	{
	  TPathPt* P = &(oPathPoints[(I + K) % Count]);
	  double Len1 = (P->CalcPt() - P1).len();
	  double Len2 = (P->CalcPt() - P2).len();
      Adjust(Crv1, Len1, Crv2, Len2, L1, P, L2, P1, P2, BumpMod);
	}
  }
}
//==========================================================================*

//==========================================================================*
// Set offset 
//--------------------------------------------------------------------------*
void TClothoidLane::SetOffset
  (double Crv, double T, TPathPt* P, const TPathPt* PP, const TPathPt* PN)
{
  double Margin = oFixCarParam.oWidth / 2; 
  double WL = -P->WtoL() + Margin;
  double WR = P->WtoR() - Margin;
  double BorderInner = oFixCarParam.oBorderInner + MAX(0.0,MIN(oFixCarParam.oMaxBorderInner, oFixCarParam.oBorderScale * fabs(Crv) - 1));
  double BorderOuter = oFixCarParam.oBorderOuter;
  double W;

  if (Crv >= 0)
  {
	WL += BorderInner;
	T = MAX(T,WL);
 	T = MIN(T,WR - P->BufR - BorderOuter);
  }
  else
  {
	WR -= BorderInner;
	T = MIN(T,WR);
	T = MAX(T,WL + P->BufL + BorderOuter);
  }

  if (!(P->Fix))
  {
    P->Offset = T;
    P->Point = P->CalcPt();
    P->Crv = TUtils::CalcCurvatureXY(PP->Point, P->Point, PN->Point);
  }
}
//==========================================================================*

//==========================================================================*
// Optimize Line
//--------------------------------------------------------------------------*
void TClothoidLane::OptimiseLine
  (int Index, int Step,	double HLimit,
  TPathPt* L3,	const TPathPt* L2, const TPathPt* L4)
{
  TLinearRegression LR;

  const int Count = oTrack->Count();

  int I = (Index + Count - Step) % Count;
  while (oPathPoints[I].FlyHeight > HLimit)
  {
	LR.Add(oPathPoints[I].Point.GetXY());
	I = (I + Count - Step) % Count;
  }

  LR.Add(oPathPoints[I].Point.GetXY());

  I = Index;
  while (oPathPoints[I].FlyHeight > HLimit)
  {
 	LR.Add(oPathPoints[I].Point.GetXY());
	I = (I + Step) % Count;
  }

  LR.Add(oPathPoints[I].Point.GetXY());

  //GfOut("OptimiseLine Index: %4d", Index);

  TVec2d P, V;
  LR.CalcLine(P, V);

  double T;
  TUtils::LineCrossesLine(L3->Pt().GetXY(), L3->Norm().GetXY(), P, V, T);

  SetOffset(0, T, L3, L2, L4);
}
//==========================================================================*

//==========================================================================*
// Optimize
//--------------------------------------------------------------------------*
void TClothoidLane::Adjust
  (double Crv1, double Len1, double Crv2, double Len2, 
  const TPathPt* PP, 
  TPathPt* P, 
  const TPathPt* PN, 
  TVec3d VPP,
  TVec3d VPN,
  double BumpMod)
{
  double T = P->Offset;
  double Crv = (Len2 * Crv1 + Len1 * Crv2) / (Len1 + Len2);
  if (Crv != 0.0)
  {
    if (Crv1 * Crv2 >= 0 
	  && fabs(Crv1) < MAX_SPEED_CRV
	  && fabs(Crv2) < MAX_SPEED_CRV)
    {
	  Crv *= 0.9;
    }

    TUtils::LineCrossesLineXY(P->Pt(), P->Norm(), VPP, VPN - VPP, T);

    double Delta = DELTA_T;
    double DeltaCrv = TUtils::CalcCurvatureXY
	  (VPP, P->Pt() + P->Norm() * (T + Delta), VPN);

    if (BumpMod > 0 && BumpMod < 2)
      Delta *= 1 - MAX(0.0,MIN(0.5,P->FlyHeight - 0.1)) * BumpMod;
  

    T += Delta * Crv / DeltaCrv;
  }
  SetOffset(Crv, T, P, PP, PN);
}
//==========================================================================*

//==========================================================================*
// Optimize
//--------------------------------------------------------------------------*
void TClothoidLane::Optimise
  (double Factor, TPathPt* L3,
  const TPathPt* L0, const TPathPt*	L1, const TPathPt*	L2,
  const TPathPt* L4, const TPathPt*	L5,	const TPathPt*	L6,
  double BumpMod)
{
  TVec3d P0 = L0->Point;
  TVec3d P1 = L1->Point;
  TVec3d P2 = L2->Point;
  TVec3d P3 = L3->Point;
  TVec3d P4 = L4->Point;
  TVec3d P5 = L5->Point;
  TVec3d P6 = L6->Point;

  double Crv1 = TUtils::CalcCurvatureXY(P1, P2, P3);
  double Crv2 = TUtils::CalcCurvatureXY(P3, P4, P5);

  double Len1 = myhypot(P3.x - P2.x, P3.y - P2.y);
  double Len2 = myhypot(P4.x - P3.x, P4.y - P3.y);

  if (Crv1 * Crv2 > 0)
  {
    double Crv0 = TUtils::CalcCurvatureXY(P0, P1, P2);
    double Crv3 = TUtils::CalcCurvatureXY(P4, P5, P6);
	if (Crv0 * Crv1 > 0 && Crv2 * Crv3 > 0)
	{
	  if (fabs(Crv0) < fabs(Crv1) && fabs(Crv1) * 1.02 < fabs(Crv2))
	  {
		Crv1 *= Factor;
		Crv0 *= Factor;
	  }
	  else if(fabs(Crv0) > fabs(Crv1) * 1.02 && fabs(Crv1) > fabs(Crv2))
	  {
		Crv1 *= Factor;
		Crv0 *= Factor;
	  }
	}
  }
  else if(Crv1 * Crv2 < 0)
  {
	double Crv0 = TUtils::CalcCurvatureXY(P0, P1, P2);
	double Crv3 = TUtils::CalcCurvatureXY(P4, P5, P6);
	if (Crv0 * Crv1 > 0 && Crv2 * Crv3 > 0)
	{
	  if(fabs(Crv1) < fabs(Crv2) && fabs(Crv1) < fabs(Crv3))
	  {
		Crv1 = (Crv1 * 0.25 + Crv2 * 0.75);
		Crv0 = (Crv0 * 0.25 + Crv3 * 0.75);
	  }
	  else if(fabs(Crv2) < fabs(Crv1) && fabs(Crv2) < fabs(Crv0))
	  {
		Crv2 = (Crv2 * 0.25 + Crv1 * 0.75);
		Crv3 = (Crv3 * 0.25 + Crv0 * 0.75);
	  }
	}
  }
  Adjust(Crv1, Len1, Crv2, Len2, L2, L3, L4, P2, P4, BumpMod);
}
//==========================================================================*

//==========================================================================*
// Optimize Lane
//--------------------------------------------------------------------------*
void TClothoidLane::OptimisePath
  (int Step, int NIterations, double BumpMod, bool Smooth)
{
  const int Count = oTrack->Count();

  for (int I = 0; I < NIterations; I++)
  {
    TPathPt* L0 = NULL;
	TPathPt* L1 = &oPathPoints[Count - 3 * Step];
	TPathPt* L2 = &oPathPoints[Count - 2 * Step];
	TPathPt* L3 = &oPathPoints[Count - Step];
	TPathPt* L4 = &oPathPoints[0];
	TPathPt* L5 = &oPathPoints[Step];
	TPathPt* L6 = &oPathPoints[2 * Step];

	// Go forwards
	int	K = 3 * Step;
	int	N = (Count + Step - 1) / Step;
	for (int J = 0; J < N; J++)
	{
	  L0 = L1;
	  L1 = L2;
	  L2 = L3;
	  L3 = L4;
	  L4 = L5;
	  L5 = L6;
	  L6 = &oPathPoints[K];

	  int Index = (K + Count - 3 * Step) % Count;

	  if (BumpMod == 2 && L3->FlyHeight > 0.1)
	  {
		//GfOut("OptimiseLine Index: %d\n",Index);
		OptimiseLine(Index, Step, 0.1, L3, L2, L4);
	  }
	  else
	  {
		Optimise(1.015, L3, L0, L1, L2, L4, L5, L6, BumpMod);
	  }
	  
      K += Step;
	  if (K >= Count)
		K = 0;
	}
  }

  // Smooth values between Steps
  if (Step > 1)
    SmoothBetween(Step,BumpMod);
}
//==========================================================================*

//==========================================================================*
// Save lane to file
//--------------------------------------------------------------------------*
bool TClothoidLane::SaveToFile(const char* Filename)
{
#ifdef mysecure
  FILE* F;
  int err = myfopen(&F, Filename, "w");
#else
  FILE* F = fopen(Filename, "w");
#endif
  if (F == 0)
    return false;

  fprintf(F, "#Track Count = %d\n",oTrack->Count());
  fprintf(F, "#Track Length = %g\n",oTrack->Length());
  fprintf(F, "#LapTime = %g\n",CalcEstimatedLapTime());

  //fprintf(F, "#I,C.x, C.y, C.z, N.x, N.y, N.z, P.WtoL(), P.Offset, P.WtoR(), P.Point.x, P.Point.y,t1.x,t1.y,t2.x,t2.y,P.speed,P.AccSpd");
  fprintf(F, "#I,P.Point.x, P.Point.y,t1.x,t1.y,t2.x,t2.y,P.speed,P.AccSpd\n");

  for (int I = 0; I < oTrack->Count(); I++)
  {
	TPathPt& P = oPathPoints[I];                 // Points in this lane
	const TVec3d& C = P.Pt();
	const TVec3d& N = P.Norm();
	//fprintf(F, "%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
	  //I,C.x, C.y, C.z, N.x, N.y, N.z, P.WtoL(), P.Offset, P.WtoR(),
	  //P.Point.x, P.Point.y,P.Sec->Seg->vertex[0].x,P.Sec->Seg->vertex[0].y,P.Sec->Seg->vertex[1].x,P.Sec->Seg->vertex[1].y,P.Speed,P.AccSpd);
	fprintf(F, "%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
		  I,P.Point.x, P.Point.y,P.Sec->Seg->vertex[0].x,P.Sec->Seg->vertex[0].y,P.Sec->Seg->vertex[1].x,P.Sec->Seg->vertex[1].y,P.Speed,P.AccSpd);
  }

  fclose(F);

  return true;
}
//==========================================================================*

//==========================================================================*
// Save path points
//--------------------------------------------------------------------------*
bool TClothoidLane::LoadPointsFromFile(const char* TrackLoad)
{
  FILE* F = fopen(TrackLoad, "rb");
  if (F == 0)
    return false;

  int K;
  fread(&K,sizeof(int),1,F);
  if (K > 0) 
    return false;

  int Version;
  fread(&Version,sizeof(int),1,F);
  if (Version < 109) 
    return false;

  int N;
  fread(&N,sizeof(int),1,F);
  for (int I = 0; I < N; I++)
  {
    fread(&(oPathPoints[I]),sizeof(TPathPt),1,F);
	const TSection& Sec = (*oTrack)[I];	     
	oPathPoints[I].Sec = &Sec;
  }
  fclose(F);

  return true;
}
//==========================================================================*

//==========================================================================*
// Save path points
//--------------------------------------------------------------------------*
void TClothoidLane::SavePointsToFile(const char* TrackLoad)
{
  FILE* F = fopen(TrackLoad, "wb");
  if (F == 0)
    return;

  int K = 0;
  fwrite(&K,sizeof(int),1,F);

  int Version = 109;
  fwrite(&Version,sizeof(int),1,F);

  int N = oTrack->Count();
  fwrite(&N,sizeof(int),1,F);

  for (int I = 0; I < N; I++)
    fwrite(&(oPathPoints[I]),sizeof(TPathPt),1,F);
  fclose(F);
}
//==========================================================================*

//==========================================================================*
// end of file unitclothoid.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
