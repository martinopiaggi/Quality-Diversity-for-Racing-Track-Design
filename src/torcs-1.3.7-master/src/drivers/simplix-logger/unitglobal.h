//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitglobal.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Globale Datentypen und Definitionen
//
// Datei    : unitglobal.h
// Erstellt : 17.11.2007
// Stand    : 18.07.2009
// Copyright: © 2007-2009 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.001
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
// Use correct name "simplix_V1_10" for MyBotName and ROBOT_DIR
//
//--------------------------------------------------------------------------*
#ifndef _UNITGOBAL_H_
#define _UNITGOBAL_H_

// The great question, ...
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>     // The rich world of windows and gates, ...
#define mysecure           // Use _fopen_s
#define myhypot _hypot     // Use _hypot instead of hypot
#define myfopen _fopen_s   // Use _fopen_s instead of fopen
#else                    // but in a poor world without walls and fences, ...
#define myhypot hypot      // Use hypot
#define myfopen fopen      // Use fopen
#endif                   // ... who needs windows and gates?   
// ... but the answer is just 42!

//#include <portability.h> do not use this, vc++ 2005 warnings ...

// VC++ 2005 ...
#if defined(_CRT_SECURE_NO_DEPRECATE) // used with vc++ 2005
#define snprintf _snprintf_s
#endif
// ... VC++ 2005

// VC++ 6.0 ...
#if defined(WIN32) && !defined(snprintf_s) 
#undef snprintf 
#define snprintf _snprintf 
#endif

#if defined(WIN32) && !defined(fopen_s)
#undef mysecure
#endif
// ... VC++ 6.0

#include <tgf.h>     // TORCS
#include <track.h>   // TORCS
#include <car.h>     // TORCS
#include <raceman.h> // TORCS

#include "identity.txt"

//==========================================================================*
// Global constants, to be changed for different wdbee-bots
//--------------------------------------------------------------------------*
#if defined(mysimplix_EXPORTS)  
#define NBBOTS 2                                 // Number of drivers/robots
#define MyBotName "mysimplix"                    // Name of this bot
#define ROBOT_DIR "drivers/mysimplix"            // Sub path to dll
#define SECT_PRIV "simplix private"              // Private section
#define DEFAULTCARTYPE "car1-trb1"               // Default car type
#elif defined(simplix_trb1a_EXPORTS)  
#define NBBOTS 10                                // Number of drivers/robots
#define MyBotName "simplix_trb1a"                // Name of this bot
#define ROBOT_DIR "drivers/simplix_trb1a"        // Sub path to dll
#define SECT_PRIV "simplix private"              // Private section
#define DEFAULTCARTYPE "car1-trb1"               // Default car type
#elif defined(simplix_trb1b_EXPORTS)  
#define NBBOTS 2                                 // Number of drivers/robots
#define MyBotName "simplix_trb1b"                // Name of this bot
#define ROBOT_DIR "drivers/simplix_trb1b"        // Sub path to dll
#define SECT_PRIV "simplix private"              // Private section
#define DEFAULTCARTYPE "car6-trb1"               // Default car type
#else
#define NBBOTS 2                                 // Number of drivers/robots
// V1.10.001: >>>
#define MyBotName "simplix_V1_10"                // Name of this bot
#define ROBOT_DIR "drivers/simplix_V1_10"        // Sub path to dll
// <<< V1.10.001
#define SECT_PRIV "simplix private"              // Private section
#define DEFAULTCARTYPE "car1-trb1"               // Default car type
#endif

// Estimation of acceleration: depends on car types
// carX-trb1:
const double PAR_A = 0.001852;                   // Parameters of a quadratic
const double PAR_B = -0.35;                      // of X = speed in [m/s]
const double PAR_C = 17.7;                       // Acc = (A*X+B)*X+C
//==========================================================================*

//==========================================================================*
// Other global constants
//--------------------------------------------------------------------------*
const int cMAX_OPP = 40;                         // Max number of drivers
const float TRACKRES = 2.5f;                     // Digitising sampling rate
const int FLY_COUNT = 20;                        // Fly counter
//==========================================================================*

//==========================================================================*
// Forewarding for Classes and pointers to
//--------------------------------------------------------------------------*
class TAbstractStrategy;
typedef TAbstractStrategy* PAbstractStrategy;

class TAdjustedCharacteristic;
typedef TAdjustedCharacteristic* PAdjustedCharacteristic;

class TCarParam;
typedef TCarParam* PCarParam;

class TClothoidLane;
typedef TClothoidLane* PClothoidLane;

class TCollision;
typedef TCollision* PCollision;

class TCommonData;
typedef TCommonData* PCommonData;

class TDriver;
typedef TDriver* PDriver;

class TGenericAvoidance;
typedef TGenericAvoidance* PGenericAvoidance;

class TLane;
typedef TLane* PLane;

class TLanePoint;
typedef TLanePoint* PLanePoint;

class TLinAttractor;
typedef TLinAttractor* PLinAttractor;

class TLinearRegression;
typedef TLinearRegression* PLinearRegression;

class TOpponent;
typedef TOpponent* POpponent;

class TParam;
typedef TParam* PParam;

class TPath;
typedef TPath* PPath;

class TPit;
typedef TPit* PPit;

class TQuadratic;  
typedef TQuadratic* PQuadratic;

class TSimpleStrategy;
typedef TSimpleStrategy* PSimpleStrategy;

class TSection;  
typedef TSection* PSection;

class TTeamManager;
typedef TTeamManager* PTeamManager;

class TTrackDescription;
typedef TTrackDescription* PTrackDescription;
//==========================================================================*

//==========================================================================*
// Type definitions for TORCS pointers
//--------------------------------------------------------------------------*
typedef tTrack* PTrack;                            // TORCS track  
typedef tCarElt* PtCarElt;                         // TORCS car
typedef CarElt* PCarElt;                           // TORCS car
typedef void* PCarHandle;                          // TORCS file handle
typedef void* PCarSettings;                        // TORCS file handle
typedef tSituation* PSituation;                    // TORCS situation
typedef tTrackSeg* PTrackSeg;                      // TORCS segment of track

//==========================================================================*

//==========================================================================*
// Indices for the different racinglines
//--------------------------------------------------------------------------*
enum	               
{
  RL_FREE,                                       // Racing untroubled
  RL_LEFT,                                       // Racing on left side
  RL_RIGHT,                                      // Racing on right side

  gNBR_RL                                        // Nbr of racinglines 
};
//==========================================================================*

//==========================================================================*
// Collision flags
//--------------------------------------------------------------------------*
enum	
{
  F_LEFT			= 0x000001, // You are at my left side
  F_RIGHT			= 0x000002, // or my right side
  F_FRONT			= 0x000004, // in front of me
  F_REAR			= 0x000008, // behind me 

  F_AHEAD			= 0x000010, // I see you in front of me
  F_AT_SIDE			= 0x000020, // or looking to a side
  F_BEHIND			= 0x000040, // or in the mirror

  F_TRK_LEFT		= 0x000100, // British
  F_TRK_RIGHT		= 0x000200, // European

  F_CATCHING		= 0x001000, // Tom and
  F_CATCHING_ACC	= 0x002000, //   Jerry 
  F_COLLIDE			= 0x004000, // My assurance nbr is ...
  F_TRAFFIC			= 0x008000, // Business as usual
  F_CLOSE			= 0x010000, // You are too close to me!
  F_TEAMMATE		= 0x020000, // Not like Alonso and Hamilton
  F_LAPPER			= 0x040000,	// it's lapping us
  F_BEING_LAPPED	= 0x080000,	// we're lapping it
  F_DANGEROUS		= 0x100000, // ugly!
  F_PREVIEWSLOW		= 0x1000000  // ugly!
};
//==========================================================================*

//==========================================================================*
// #defines
//--------------------------------------------------------------------------*
// Array sizes ...
#define NBR_BRAKECOEFF 50                        // Number of brake coeffs 
// ... Array sizes

// my own planet ...
#define G 9.81                                   // Hello earth gravity
// ... my own planet

// pit states ...
#define PIT_IS_FREE (-1)
// ... pit states

// Shortcuts for TORCS commands ...
#define CarAccelCmd (oCar->_accelCmd)
#define CarBrakeCmd (oCar->_brakeCmd)
#define CarClutchCmd (oCar->_clutchCmd)
#define CarGearCmd (oCar->_gearCmd)
#define CarSteerCmd (oCar->_steerCmd)
#define CarRaceCmd (oCar->_raceCmd)
// ... Shortcuts for TORCS

// Shortcuts for TORCS commands ...
#define CarAccelLat (oCar->_accel_y)
#define CarAccelLong (oCar->_accel_x)
#define CarDamage (oCar->_dammage)
#define CarDriverIndex (oCar->_driverIndex)
#define CarFriction (oCar->_trkPos.seg->surface->kFriction)
#define CarFuel (oCar->_fuel)
#define CarGearNbr (oCar->_gearNb)
#define CarGearOffset (oCar->_gearOffset)
#define CarGearRatio oCar->_gearRatio
#define CarCarHandle (oCar->_carHandle)
#define CarIndex (oCar->index)
#define CarLaps (oCar->_laps)
#define CarLapsBehindLeader (oCar->_lapsBehindLeader)
#define CarLength (oCar->_dimension_x)
#define CarPit (oCar->_pit)
#define CarPosX (oCar->_pos_X)
#define CarPosY (oCar->_pos_Y)
#define CarPosZ (oCar->_pos_Z)
#define CarPubGlobPosX (oCar->pub.DynGCg.pos.x)
#define CarPubGlobPosY (oCar->pub.DynGCg.pos.y)
#define CarPubGlobVelX (oCar->pub.DynGCg.vel.x)
#define CarPubGlobVelY (oCar->pub.DynGCg.vel.y)
#define CarPubGlobAccX (oCar->pub.DynGCg.acc.x)
#define CarPubGlobAccY (oCar->pub.DynGCg.acc.y)
#define CarRpm (oCar->_enginerpm)
#define CarRpmLimit (oCar->_enginerpmRedLine)
#define CarSeg (oCar->_trkPos.seg)
#define CarSpeedLat (oCar->_speed_y)
#define CarSpeedLong (oCar->_speed_x)
#define CarSpeedX (oCar->_speed_X)
#define CarSpeedY (oCar->_speed_Y)
#define CarSteerLock (oCar->_steerLock)
#define CarState (oCar->_state)
#define CarTeamname (oCar->_teamname)
#define CarToMiddle (oCar->_trkPos.toMiddle)
#define CarTrackPos (oCar->_trkPos)
#define CarWidth (oCar->_dimension_y)
#define CarYaw (oCar->_yaw)
#define CarYawRate (oCar->_yaw_rate)
#define DistanceFromStartLine (oCar->_distFromStartLine)
#define DistanceRaced (oCar->_distRaced)
#define HasDriveTrainFront (oDriveTrainType == cDT_FWD || oDriveTrainType == cDT_4WD)
#define HasDriveTrainRear (oDriveTrainType == cDT_RWD || oDriveTrainType == cDT_4WD)
#define NextGear (oCar->_gear + 1)
#define PrevGear (oCar->_gear - 1)
#define RemainingLaps (oCar->_remainingLaps)
#define TreadClutch (oClutch = 0.5)
#define NextRpm (oShift[oCar->_gear])
#define IsFullThrottle ((oAccel >= 1.0) && (oBrake <= 0.0))
#define SteerLock (oCar->_steerLock)
#define WheelRad(x) (oCar->_wheelRadius(x))
#define WheelSpinVel(x) (oCar->_wheelSpinVel(x))
#define UsedGear (oCar->_gear)
// ... Shortcuts for TORCS

// Shortcuts for this robot ...
#define IsTickover (oCar->_gear <= 0)
#define FixRange ((AvoidTarget < OldAvoidRange) && (AvoidTarget >= oAvoidRange) || (AvoidTarget > OldAvoidRange) && (AvoidTarget <= oAvoidRange) || (fabs(oAvoidRange - AvoidTarget) < 0.0005))
#define FixOffset ((OldAvoidOffset < Target) && (oAvoidOffset >= Target) || (OldAvoidOffset > Target) && (oAvoidOffset <= Target))
#define	DEG_TO_RAD(x)	((x) * PI / 180.0)
#define	STEER_SPD_IDX(x)	(int(floor((x) / 5)))
#define	STEER_K_IDX(k)		(MAX(0, MIN(int(20 + floor((k) * 500 + 0.5)), 40)))
#define	SGN(X) ((X) < 0 ? -1 : (X) > 0 ? 1 : 0)
#define MINMAX(X,Y) (MAX(-X,MIN(X,Y)))
#define DIRCHANGED(X,Y,Z) ((X < Z) && (Y >= Z) || (X > Z) && (Y <= Z))
#define XX2Y(X,Y) (X*X/(2*Y))

#define FLOAT_NORM_PI_PI(x) 				\
{ \
	while ((x) > PI) { (x) -= (float)(2*PI); } \
	while ((x) < -PI) { (x) += (float)(2*PI); } \
}

#define DOUBLE_NORM_PI_PI(x) 				\
{ \
	while ((x) > PI) { (x) -= 2*PI; } \
	while ((x) < -PI) { (x) += 2*PI; } \
}
// ... Shortcuts for this robot

// Internal parameters ...
#define ANALYSE_STEPS 2 
#define LENGTH_MARGIN (2.0f) // Initial value for PRV_LENGTH_MARGIN
#define NOAVOIDLENGTH 0 
#define AVG_KEEP (0.75)
#define AVG_CHANGE (1 - AVG_KEEP) 
#define MAX_SPEED_CRV 0.00175 // R = 571,428 m
#define SIDE_MARGIN (0.5)   
#define DELTA_T 0.0001   
#define UNSTUCK_COUNTER 60
#define MAXBLOCKED 9
// ... Internal parameters

// Parameters of this robot ...
#define PRV_MAX_FUEL         "max fuel"
#define PRV_MIN_LAPS         "min laps"
#define PRV_LENGTH_MARGIN	 "length margin"
#define PRV_QUALIFICATION    "qualification"      // Practice as qualifying

#define PRV_PIT_USE_FIRST    "pit use first" 
#define PRV_PIT_USE_SMOOTH   "pit use smooth" 
#define PRV_PITLANE_ENTRY    "pitlane entry offset" 
#define PRV_PITLANE_EXIT     "pitlane exit offset" 
#define PRV_PIT_ENTRY_LONG	 "pit entry long"
#define PRV_PIT_EXIT_LONG	 "pit exit long"
#define PRV_PIT_EXIT_LEN	 "pit exit length"
#define PRV_PIT_LAT_OFFS	 "pit lat offset"
#define PRV_PIT_LONG_OFFS	 "pit long offset"
#define PRV_PIT_SCALE_BRAKE  "pit scale brake" 
#define PRV_PIT_STOP_DIST    "pit stop dist"
#define PRV_PIT_BRAKE_DIST   "pit brake dist"
#define PRV_PIT_TEST_STOP    "pit test stop" 

#define PRV_SCALE_BRAKE      "scale brake"        // Scale brake force
#define PRV_SCALE_BUMP       "scale bump"         // Scale bump detection inside
#define PRV_SCALE_BUMPOUTER  "scale bump outer"   // Scale bump detection outside
#define PRV_SCALE_MU         "scale mu"           // Scale friction calculation 
#define PRV_SCALE_FRICTION	 "scale friction"     // Scale friction calculation 
#define PRV_SCALE_BRAKING	 "scale braking"      // Scale brake calculation 

#define PRV_SCALE_MIN_MU     "scale min mu"
#define PRV_SCALE_STEER	     "scale steer"

#define PRV_AVOID_SCALE		 "avoid scale"
#define PRV_AVOID_WIDTH		 "avoid width"

#define PRV_LOOKAHEADFACTOR  "lookaheadfactor"

#define PRV_STAY_TOGETHER	 "stay together"

#define PRV_NO_AVOIDLENGTH	 "no avoid"
#define PRV_START_SIDE	     "start side"

#define PRV_TRKPIT_START	 "trkpit start"
#define PRV_TRKPIT_END       "trkpit end"

#define PRV_RESERVE          "reserve"            // Reserve in m
#define PRV_FUELPER100KM     "fuelper100km"       // Spritverbrauch pro 100 km
// ... Parameters of this robot

// Parameter candidates ...
#define ABS_DELTA (1.1)
#define ABS_SCALE (0.5)

#define CLUTCH_MAX (0.5)
#define CLUTCH_DELTA (0.05)
#define CLUTCH_RANGE (0.82)

#define SLOWSPEED (5.0)
// ... Parameter candidates 
//==========================================================================*
#endif // _UNITGOBAL_H_
//--------------------------------------------------------------------------*
// end of file unitglobal.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
