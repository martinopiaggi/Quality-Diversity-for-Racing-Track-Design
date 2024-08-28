//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitfixcarparam.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Unveränderliche Parameter des Fahrzeugs
//
// Datei    : unitfixcarparam.h
// Erstellt : 25.11.2007
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
#ifndef _UNITFIXCARPARAM_H_
#define _UNITFIXCARPARAM_H_

#include "unitglobal.h"
#include "unitcommon.h"

#include "unittmpcarparam.h"
#include "unitcarparam.h"

//==========================================================================*
// Deklaration der Klasse TFixCarParam
//--------------------------------------------------------------------------*
class TFixCarParam  
{
  private:

  public:
    PtCarElt oCar;                               // Pointer to TORCS data of car
    TTmpCarParam* oTmpCarParam;                  // Variable car parameters									    

	TFixCarParam();                              // Default constructor 
	~TFixCarParam();                             // Destructor

	double CalcAcceleration
	  (									    
	   double Crv0, double Crvz0, 
	   double Crv1, double Crvz1,
	   double Speed, double Dist, 
	   double Friction,
	   double TrackRollAngle) const;

	double CalcBraking
	  (TCarParam& CarParam,									    
	   double Crv0, double Crvz0, 
	   double Crv1, double Crvz1,
	   double Speed, double Dist, 
	   double Friction,
	   double TrackRollAngle) const;

	double CalcBrakingPit
	  (TCarParam& CarParam,									    
	   double Crv0, double Crvz0, 
	   double Crv1, double Crvz1,
	   double Speed, double Dist, 
	   double Friction,
	   double TrackRollAngle) const;

	double CalcMaxSpeed
	  (TCarParam& CarParam,									    
	   double Crv0, 
	   double Crv1,
	   double Crvz, 
	   double KFriction,
	   double TrackRollAngle) const;                                  

	double CalcMaxLateralF
	  (									    
       double Speed, 
	   double Friction, 
	   double Crvz = 0.0) const;

	double CalcMaxSpeedCrv() const;

	void Initialize
	  (PtCarElt Car);

  public:
	double oBorderInner;                         // Const. Buffer to inner
	double oBorderOuter;                         // Const. Buffer to outer
	double oMaxBorderInner;                      // Max var. Buffer to inner
	double oBorderScale;                         // Scale var. Buffer to inner
	double oCa;                                  // Aerodynamic downforce constant
	double oCaFrontWing;                         // Aerod. d. const. front wing
	double oCaGroundEffect;                      // Aerod. d. const. ground effect
	double oCaRearWing;                          // Aerod. d. const. rear wing
	double oCdBody;                              // Aerodynamic drag constant car body
	double oCdWing;	                             // Aerod. drag const. wings
	double oEmptyMass;                           // Mass of car.without fuel
	double oLength;                              // Length of car (m)
	double oTyreMu;	                             // Mu of tyres
	double oTyreMuFront;                         // Mu of front tyres
	double oTyreMuRear;                          // Mu of rear tyres
	double oWidth;                               // Width of car (m)

	double oPitBrakeDist;

};
//==========================================================================*
#endif // _UNITFIXCARPARAM_H_
//--------------------------------------------------------------------------*
// end of file unitfixcarparam.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
