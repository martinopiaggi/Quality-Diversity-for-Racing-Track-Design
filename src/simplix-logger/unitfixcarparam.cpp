//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitfixcarparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Unveränderliche Parameter des Fahrzeugs und Nebenrechnungen
//
// Datei    : unitfixcarparam.cpp
// Erstellt : 25.11.2007
// Stand    : 22.11.2008
// Copyright: © 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem Roboter mouse_2006
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
#include <math.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitparabel.h"
#include "unitfixcarparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TFixCarParam::TFixCarParam():
  oCar(NULL),
  oTmpCarParam(NULL),
  oBorderInner(0.25),
  oBorderOuter(0.25),
  oMaxBorderInner(1.00),
  oBorderScale(50.0),
  oCa(0),
  oCaFrontWing(0),
  oCaGroundEffect(0),
  oCaRearWing(0),
  oCdBody(0),
  oCdWing(0),
  oEmptyMass(0),
  oLength(4.5),
  oTyreMu(0),
  oTyreMuFront(0),
  oTyreMuRear(0),
  oWidth(2.0),
  oPitBrakeDist(150.0)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TFixCarParam::~TFixCarParam()
{
}
//===========================================================================

//==========================================================================*
// Initialize
//--------------------------------------------------------------------------*
void TFixCarParam::Initialize(PtCarElt Car)
{
  oCar = Car;
}
//==========================================================================*

//==========================================================================*
// Calculate accelleration
//--------------------------------------------------------------------------*
double TFixCarParam::CalcAcceleration(
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  double MU = Friction * oTyreMu;
  double CD = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

//  double Crv = (Crv0  + Crv1) * 0.5;
//  double Crvz = (Crvz0 + Crvz1) * 0.5; 
  double Crv = (0.25*Crv0  + 0.75*Crv1);
  double Crvz = (0.25*Crvz0 + 0.75*Crvz1); 
  if (Crvz > 0)
	Crvz = 0;

  double Gdown = G * cos(TrackRollAngle);
  double Glat = G * sin(TrackRollAngle);
  double Gtan = 0;	// TODO: track pitch angle.

  double U = Speed;
  double V = U;

  TParabel AccFromSpd(PAR_A, PAR_B, PAR_C);      // approx. carX-trb1
  double OldV = 0.0;
  for (int Count = 0; Count < 10; Count++)
  {
    double AvgV = (U + V) * 0.5;
    double AvgV2 = AvgV * AvgV;

    double Fdown = oTmpCarParam->oMass * Gdown 
	  + (oTmpCarParam->oMass * Crvz + oCa) * AvgV2;
    double Froad = Fdown * MU;
    double Flat = oTmpCarParam->oMass * Glat;
    double Ftan = oTmpCarParam->oMass * Gtan - CD * AvgV2;

    double Flatroad = fabs(oTmpCarParam->oMass * AvgV2 * Crv - Flat);
    if (Flatroad > Froad)
  	  Flatroad = Froad;

	double Ftanroad = sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	double Acc = Ftanroad / oTmpCarParam->oMass;
	double MaxAcc = MIN(11.5,AccFromSpd.CalcY(AvgV));
    if (Acc > MaxAcc)
	  Acc = MaxAcc;

	double Inner = MAX(0, U * U + 2 * Acc * Dist);
	V = sqrt(Inner);
	if (fabs(V - OldV) < 0.001)
	  break;
	OldV = V;
  }
  return V;
}
//==========================================================================*

//==========================================================================*
// Calculate decceleration
//--------------------------------------------------------------------------*
double	TFixCarParam::CalcBraking
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  Friction *= 0.95;
  double Mu = Friction * oTyreMu;
  double Mu_F = Mu;
  double Mu_R = Mu;

  Mu_F = Friction * oTyreMuFront;
  Mu_R = Friction * oTyreMuRear;
  Mu = (Mu_F + Mu_R) / 2;

  // From TORCS:
  double Cd = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

//  double Crv = (Crv0 + Crv1) * 0.5;
//  double Crvz = (Crvz0 + Crvz1) * 0.5;
  double Crv = (0.25*Crv0 + 0.75*Crv1);
  double Crvz = (0.25*Crvz0 + 0.75*Crvz1);
  if (Crvz > 0)
	Crvz = 0; 

  double Gdown = G * cos(TrackRollAngle);
  double Glat  = G * sin(TrackRollAngle);
  double Gtan  = 0;	

  double V = Speed;
  double U = V;

  for (int I = 0; I < 10; I++)
  {
	double AvgV = (U + V) * 0.5;
	double AvgV2 = AvgV * AvgV;

	double Froad;
	double Fdown = oTmpCarParam->oMass * Gdown + (oTmpCarParam->oMass * Crvz + oCaGroundEffect) * AvgV2;
	double Ffrnt = oCaFrontWing * AvgV2;
	double Frear = oCaRearWing * AvgV2;

	Froad = Fdown * Mu + Ffrnt * Mu_F + Frear * Mu_R;

	double Flat  = oTmpCarParam->oMass * Glat;
	double Ftan  = oTmpCarParam->oMass * Gtan - Cd * AvgV2;

	double Flatroad = fabs(oTmpCarParam->oMass * AvgV2 * Crv - Flat);
	if (Flatroad > Froad)
	  Flatroad = Froad;

	double Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	double Acc = CarParam.oScaleBrake * Ftanroad 
	  / oTmpCarParam->oMass;

	double Inner = MAX(0, V * V - 2 * Acc * Dist);
	double OldU = U;
	U = sqrt(Inner);
	if (fabs(U - OldU) < 0.001)
	  break;
  }

  return U;
}
//==========================================================================*

//==========================================================================*
// Calculate decceleration in pitlane
//--------------------------------------------------------------------------*
double	TFixCarParam::CalcBrakingPit
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  Friction *= 0.95;
  double Mu = Friction * oTyreMu;
  double Mu_F = Mu;
  double Mu_R = Mu;

  Mu_F = Friction * oTyreMuFront;
  Mu_R = Friction * oTyreMuRear;
  Mu = (Mu_F + Mu_R) / 2;

  // From TORCS:
  double Cd = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

  double Crv = (Crv0 + Crv1) * 0.5;
  double Crvz = (Crvz0 + Crvz1) * 0.5;
  if (Crvz > 0)
	Crvz = 0; 

  double Gdown = G * cos(TrackRollAngle);
  double Glat  = G * sin(TrackRollAngle);
  double Gtan  = 0;	

  double V = Speed;
  double U = V;

  for (int I = 0; I < 10; I++)
  {
	double AvgV = (U + V) * 0.5;
	double AvgV2 = AvgV * AvgV;

	double Froad;
	double Fdown = oTmpCarParam->oMass * Gdown + (oTmpCarParam->oMass * Crvz + oCaGroundEffect) * AvgV2;
	double Ffrnt = oCaFrontWing * AvgV2;
	double Frear = oCaRearWing * AvgV2;

	Froad = Fdown * Mu + Ffrnt * Mu_F + Frear * Mu_R;

	double Flat  = oTmpCarParam->oMass * Glat;
	double Ftan  = oTmpCarParam->oMass * Gtan - Cd * AvgV2;

	double Flatroad = fabs(oTmpCarParam->oMass * AvgV2 * Crv - Flat);
	if (Flatroad > Froad)
	  Flatroad = Froad;

	double Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	double Acc = CarParam.oScaleBrakePit * Ftanroad 
	  / oTmpCarParam->oMass;

	double Inner = MAX(0, V * V - 2 * Acc * Dist);
	double OldU = U;
	U = sqrt(Inner);
	if (fabs(U - OldU) < 0.001)
	  break;
  }

  return U;
}
//==========================================================================*

//==========================================================================*
// Calculate maximum of speed
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxSpeed
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P
  double Crv1,                                   // Curvature in xy at P
  double CrvZ,                                   // Curvature in z at P
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  // Here we calculate the theoretical maximum speed at a point on the
  // path. This takes into account the curvature of the path (crv), the
  // grip on the road (mu), the downforce from the wings and the ground
  // effect (CA), the tilt of the road (left to right slope) (sin)
  // and the curvature of the road in z (crvz).
  //
  // There are still a few silly fudge factors to make the theory match
  // with the reality (the car goes too slowly otherwise, aarrgh!).

  double Mu;

  double Cos = cos(TrackRollAngle);
  double Sin = sin(TrackRollAngle);
  //Cos *= Cos;
  //Sin *= Sin;

  double AbsCrv0 = MAX(0.001, fabs(Crv0));
  double AbsCrv1 = MAX(0.001, fabs(Crv1));
  double AbsCrv = AbsCrv0;
  double factor = 1.0;
  if (AbsCrv > AbsCrv1)
	factor = 1.00;
  else
	factor = 0.985;

  double Den;

  double ScaleBump;
  if (Crv0 > 0)
    ScaleBump = CarParam.oScaleBumpLeft;
  else
    ScaleBump = CarParam.oScaleBumpRight;

  double MuF = Friction * oTyreMuFront * CarParam.oScaleMu;
  double MuR = Friction * oTyreMuRear * CarParam.oScaleMu;
  Mu = (MuF + MuR) / 2;
  Den = (AbsCrv - ScaleBump * CrvZ)
    - (oCaFrontWing * MuF + oCaRearWing * MuR + oCaGroundEffect * Mu) / oTmpCarParam->oMass;

  if (Den < 0.00001)
   Den = 0.00001;

  double Speed = factor * sqrt((Cos * G * Mu + Sin * G * SGN(Crv0)) / Den);

  if (Speed > 150)                               // (100 m/s = 360 km/h)
    Speed = 150;                                 // (150 m/s = 540 km/h)

  return Speed;
}
//==========================================================================*

//==========================================================================*
// Calculate maximum of lateral force
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxLateralF
  (double Speed, double Friction, double Crvz) const
{
  double Fdown = oTmpCarParam->oMass * G 
	+ (oTmpCarParam->oMass * Crvz + oCa) * Speed * Speed;
  return Fdown * Friction * oTyreMu;
}
//==========================================================================*

//==========================================================================*
// Calculate curve at maximum of speed
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxSpeedCrv() const
{
//  const double MAX_SPD = 100; // 360 km/h
  const double MAX_SPD = 150; // 540 km/h
  return G * oTyreMu / (MAX_SPD * MAX_SPD);
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitfixcarparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
