//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitopponent.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Rivalen (und Teammitglieder)
//
// Datei    : unitopponent.cpp
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
#include <robottools.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"
#include "unitopponent.h"
#include "unitparabel.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TOpponent::TOpponent()
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TOpponent::~TOpponent()
{
}
//==========================================================================*

//==========================================================================*
// Init
//--------------------------------------------------------------------------*
void TOpponent::Initialise
  (PTrackDescription Track, const PSituation Situation, int Index)
{
  oTrack = Track;                                // Save pointer
  oCar = Situation->cars[Index];                 // Opponents car
  oDeltaTime = Situation->deltaTime;             // Simulation delta time
  oIndex = Index;                                // Opponents cars index

  oInfo.Clear();                                 // Reset all information
  LapBackTimer = 0.0;                            // Don't delay
}
//==========================================================================*

//==========================================================================*
// Get opponents car
//--------------------------------------------------------------------------*
PCarElt TOpponent::Car()
{
  return oCar;
}
//==========================================================================*

//==========================================================================*
// Get opponents info as const
//--------------------------------------------------------------------------*
const TOpponent::TInfo& TOpponent::Info() const
{
  return oInfo;
}
//==========================================================================*

//==========================================================================*
// Get opponents info
//--------------------------------------------------------------------------*
TOpponent::TInfo& TOpponent::Info()
{
  return oInfo;
}
//==========================================================================*

//==========================================================================*
// Update
//
// ATTENTION oCar ist opponents car, so we can use our shortcuts!!!
//--------------------------------------------------------------------------*
void TOpponent::Update(
  const PCarElt MyCar,
  PTeamManager TeamManager,
  double MyDirX,
  double MyDirY)
{
  oTeamManager = TeamManager;                    // Save Pointer

  if((CarState & RM_CAR_STATE_NO_SIMU) &&        // omit cars out of race 
    (CarState & RM_CAR_STATE_PIT) == 0 )         //   if not in pit 
    return;

  oInfo.State.Speed = myhypot(CarSpeedX,CarSpeedY);// Speed of car

  // Track relative speed of opponents car
  TVec2d ToRight = oTrack->Normale(DistanceFromStartLine);
  oInfo.State.TrackVelLong = ToRight.x * CarSpeedY - ToRight.y * CarSpeedX;
  oInfo.State.TrackVelLat = ToRight.x * CarSpeedX + ToRight.y * CarSpeedY;

  // Track relative yaw of other car.
  oInfo.State.TrackYaw = CarYaw - TUtils::VecAngle(ToRight) - PI / 2;
  DOUBLE_NORM_PI_PI(oInfo.State.TrackYaw);

  // Average velocity of other car.
  oInfo.State.AvgVelLong = oInfo.State.AvgVelLong * AVG_KEEP + CarPubGlobVelX * AVG_CHANGE;
  oInfo.State.AvgVelLat = oInfo.State.AvgVelLat * AVG_KEEP + CarPubGlobVelY * AVG_CHANGE;
  oInfo.State.CarAvgVelLong = MyDirX * oInfo.State.AvgVelLong + MyDirY * oInfo.State.AvgVelLat;
  //oInfo.State.CarAvgVelLat = MyDirY * oInfo.State.AvgVelLong - MyDirX * oInfo.State.AvgVelLat;

  // Average acceleration of other car.
  oInfo.State.AvgAccLong = oInfo.State.AvgAccLong * AVG_KEEP + CarPubGlobAccX * AVG_CHANGE;
  oInfo.State.AvgAccLat = oInfo.State.AvgAccLat * AVG_KEEP + CarPubGlobAccY * AVG_CHANGE;
  oInfo.State.CarAvgAccLong = MyDirX * oInfo.State.AvgAccLong + MyDirY * oInfo.State.AvgAccLat;
  oInfo.State.CarAvgAccLat = MyDirY * oInfo.State.AvgAccLong - MyDirX * oInfo.State.AvgAccLat;

  // Offset from track center line.
  oInfo.State.Offset = -CarToMiddle;

  if(oCar == MyCar)
    return;

  // Car-Car relative calculations ...

  // calc other cars position, velocity relative to my car (global coords).
  double DistX = CarPubGlobPosX - MyCar->pub.DynGCg.pos.x;
  double DistY = CarPubGlobPosY - MyCar->pub.DynGCg.pos.y;
  double DiffVelX = CarSpeedX - MyCar->_speed_X;
  double DiffVelY = CarSpeedY - MyCar->_speed_Y;

  // work out relative position, velocity in local coords (coords of my car).
  oInfo.State.CarDistLong = MyDirX * DistX + MyDirY * DistY;
  oInfo.State.CarDistLat = MyDirY * DistX - MyDirX * DistY;
  oInfo.State.CarDiffVelLong = MyDirX * DiffVelX + MyDirY * DiffVelY;
  oInfo.State.CarDiffVelLat = MyDirY * DiffVelX - MyDirX * DiffVelY;

  oInfo.State.MinDistLong = (MyCar->_dimension_x + CarLength) / 2;
  oInfo.State.MinDistLat = (MyCar->_dimension_y + CarWidth) / 2;

  double MyVelAng = atan2(MyCar->_speed_Y, MyCar->_speed_X);
  double MyYaw = MyCar->_yaw - MyVelAng;
  DOUBLE_NORM_PI_PI(MyYaw);

  double OppYaw = CarYaw - MyVelAng;
  DOUBLE_NORM_PI_PI(OppYaw);

  // Additional distance needed while yawing of both cars
  double ExtSide = (oInfo.State.MinDistLong - oInfo.State.MinDistLat) *
    (fabs(sin(MyYaw)) + fabs(sin(OppYaw)));

  oInfo.State.MinDistLat += ExtSide + SIDE_MARGIN;
  oInfo.State.MinDistLong += TDriver::LengthMargin;

  // Distance of car from start of track.
  double MyPos = RtGetDistFromStart((tCarElt*)MyCar);
  double HisPos = RtGetDistFromStart((tCarElt*)oCar);
  double RelPos = HisPos - MyPos;
  double TrackLen = oTrack->Length();
  if (RelPos > TrackLen / 2)
    RelPos -= TrackLen;
  else if (RelPos < -TrackLen / 2)
    RelPos += TrackLen;

  oInfo.State.RelPos = RelPos; 
  if ((RelPos > 0) && (RelPos < 50))             // We just lapped back
    LapBackTimer = 60.0;                         //   delay granting letpass
  else if (RelPos < -100)                        // else if distance is too
    LapBackTimer = 0.0;                          //   long, forget it
}
//==========================================================================*

//==========================================================================*
// Classification of opponents
//--------------------------------------------------------------------------*
bool TOpponent::Classify(
	const PCarElt MyCar,
	const TState& MyState,
	bool OutOfPitLane,
	double MyMaxAccX)
{
  // Initialization
  bool Result = false;
  oInfo.Flags = 0;                               // Reset Opps. flags
  for (int I = 0; I < MAXBLOCKED; I++)
	  oInfo.Blocked[I] = false;

  // Classification needed?
  if ((oCar == MyCar)                            // Can't avoid me myself
	|| (CarState & RM_CAR_STATE_NO_SIMU)         // Opp. is out of race
	|| ((CarState & RM_CAR_STATE_PIT) != 0)      // Opp. is in its own pit
//	|| ((fabs(CarToMiddle) > 12.0 + oTrack->Width()/2)
//	   && OutOfPitLane)
	)          
  {
    return Result;                               // Nothing to do here
  }

  // Where is he relative to me and to track
  const TState& OpState = oInfo.State;           // Copy of Opps. state

  oInfo.Flags |= (OpState.CarDistLat < 0)        // Is Opp. left or right 
	 ? F_LEFT : F_RIGHT;                         //   of me?
  oInfo.Flags |= (OpState.Offset < 0)            // Is Opp. left or right 
	 ? F_TRK_LEFT : F_TRK_RIGHT;                 //   of track 

  // Preview to classify dangerouse slow opponents
  if (OpState.Speed < SLOWSPEED)                 // very slow 
  {
    if (OpState.RelPos > 0 && OpState.RelPos < 500) 
	{
      if (fabs(CarToMiddle) < 1.0 + oTrack->Width()/2)
	  { 
        double Off = 4 * (1 + OpState.Offset);
        oInfo.Blocked[int(floor(MIN(MAX(0,Off),MAXBLOCKED - 1)))] = true;
        oInfo.Blocked[int(ceil(MIN(MAX(0,Off),MAXBLOCKED - 1)))] = true;
        oInfo.Flags |= F_AHEAD | F_FRONT;        // Opp. is in front of me 
        oInfo.Flags |= F_PREVIEWSLOW;            // Classify as dangerous
        oInfo.DangerousLatchTime = 5.0;          //   situation
	  }
	}
  }

  // Classify dangerouse opponents
  if ((fabs(OpState.TrackYaw) > 30 * PI / 180)   // Opp. yawing > 45 deg
	|| (OpState.Speed < 15))                     //   or very slow 
  {
    if (OpState.RelPos > 0 && OpState.RelPos < 250)
	{
      oInfo.Flags |= F_DANGEROUS;                // Classify as dangerous
      oInfo.DangerousLatchTime = 2.0;            //   situation
	}
  }
  else                                           // Classification runs
  {                                              //   out of time
    oInfo.DangerousLatchTime -= oDeltaTime;
    if (oInfo.DangerousLatchTime <= 0)
	{
	  oInfo.Flags &= ~F_DANGEROUS;               // Not longer valid
	  oInfo.DangerousLatchTime = 0;              // Limit to zero
	}
  }

  // To look ahead
  double DistAhead =                             // Distance to look ahead
	MAX(30, MyState.Speed * MyState.Speed / 30); //   depending on my speed

  if ((oInfo.Flags & F_DANGEROUS) == 0)          // If not dangerouse, limit
    DistAhead = MIN(MAX(50, DistAhead), 100);    // view to min 50 max 100 m

  // Teammate?
  if (oTeamManager->IsTeamMate(MyCar,oCar))      // If Opp. is teammate
  {
    oInfo.Flags |= F_TEAMMATE;                   // Set teammate flag
    oInfo.TeamMateDamage = oCar->_dammage;       // Save his damages
  }

  // Near?
  if (OpState.RelPos < DistAhead && OpState.RelPos > -30)
  {
    oInfo.Flags |= F_TRAFFIC;                    // Classify situation as traffic 

    double OpVelLong =                           // Opps. longitudinal speed
	  MyState.Speed + OpState.CarDiffVelLong;    //   relative to me

	if (OpState.CarDistLong > OpState.MinDistLong)
	{     
      oInfo.Flags |= F_AHEAD | F_FRONT;          // Opp. is in front of me 

      TParabel MyPar                             // Const. value as parabel!
		(0, 0, 0, MyState.CarAvgAccLat);
      TParabel OpPar                             // Opps. lateral movment  
		(0, OpState.CarDistLat, OpState.CarDiffVelLat, OpState.CarAvgAccLat);
      TParabel RelPar = OpPar - MyPar;           // Relative lateral movement

      double T;                                  // Time to catch up at present speeds
	  double Acc = OpState.CarAvgAccLong;        // Opps. average accel. long. to me
      TParabel Q
	    (Acc/2, OpState.CarDiffVelLong, OpState.CarDistLong - OpState.MinDistLong);

      if (Q.SmallestNonNegativeRoot(T))          // Solution possible?
	  {
        oInfo.Flags |= F_CATCHING;               // Classify as catching

        double CatchOffset = RelPar.CalcY(T);    // Offset when Opp. is reached
        CatchOffset = RelPar.CalcY(T);           // Offset when Opp. is reached
        oInfo.CatchTime = T;                     // Save time to catch 
        oInfo.CatchSpeed =                       // Select estimate from distance  
	      (OpState.CarDistLong < 15)             // If near, relative to me 
		  ? OpVelLong : OpState.TrackVelLong;    // If far, relative to track 

        double HisSpeed =                        // Estimate Opps. speed at T
		  OpState.CarAvgVelLong                  // from currend average speed
		  + OpState.CarAvgAccLong * T;           // and average acceleration

        double Decel =                           // Decel. needed to stay behind
		  (MyState.CarAvgVelLong - HisSpeed) / T;

        oInfo.CatchDecel = MAX(0, Decel);        // Save reasonable values only

        if (fabs(CatchOffset) < OpState.MinDistLat + 0.5)
		{                                        // The offset will be to small
          oInfo.Flags |= F_COLLIDE;              // Classify as potential collision

          if (OpState.CarDistLong < OpState.MinDistLong + 0.15)
            oInfo.CatchDecel = 999;              // Maximum decel. needed
		}
        else
		{
          // See if we hit on the side while passing
		  Q.Set(Acc/2, OpState.CarDiffVelLong, OpState.CarDistLong + OpState.MinDistLong);

          if (Q.SmallestNonNegativeRoot(T))      // Solution possible?
		  {
            CatchOffset = RelPar.CalcY(T);       // Offset when Opp. is passed
            if ((fabs(CatchOffset) < OpState.MinDistLat + 0.5) // too near
              || (CatchOffset * OpState.CarDistLat < 0)) // or overlapping
			{
              oInfo.Flags |= F_COLLIDE;          // Classify as potential collision
			}
		  }
		}
	  }

      Q.Set(OpState.CarAvgAccLong - MyMaxAccX, 
	    OpState.CarAvgVelLong - MyState.CarAvgVelLong,
        OpState.CarDistLong - OpState.MinDistLong - 0.2 );

      if(Q.SmallestNonNegativeRoot(T))           // Solution possible?
	  {
        oInfo.Flags |= F_CATCHING_ACC;           // ???
        oInfo.CatchAccTime = T;                  // Save time
	  }

	  if (MyCar->_laps > CarLaps)                 
	  {
        oInfo.Flags |= F_BEING_LAPPED;           // Opp. is lapped
	  }
	}
	else
	{
	  if (OpState.CarDistLong < -OpState.MinDistLong) 
	  {                                          // Opp. is behind
		oInfo.Flags |= F_BEHIND | F_REAR;        // Set corresponding flags 

		if (OpState.CarDiffVelLong < 0)          // If Opp. is faster 
		{
		  oInfo.Flags |= F_CATCHING;             // Set flag
		  oInfo.CatchTime =                      // estimate time to catch
			(OpState.CarDistLong + OpState.MinDistLong) / OpState.CarDiffVelLong;
		  oInfo.CatchSpeed = OpVelLong;          // Save Opps. speed   
		}
	  }
	  else                                       // Opp is at side
	  {
		if ((oInfo.Flags & F_TEAMMATE) == 0)
		  oInfo.Flags |= F_AT_SIDE;              // Set flags
		else
		  if (fabs(OpState.CarDistLong) < OpState.MinDistLong - TDriver::LengthMargin)
		    oInfo.Flags |= F_AT_SIDE;            // Set flags

		oInfo.Flags |= (OpState.CarDistLong > 0) // In front or behind?
		  ? F_FRONT : F_REAR;

		double AheadDist = 
		  OpState.MinDistLong * 0.5;

		if (fabs(OpState.CarDistLat) < OpState.MinDistLat)
		{
		  oInfo.Flags |= F_COLLIDE;              // Colliding!
		  oInfo.CatchTime = 0;                   // Now!
		  oInfo.CatchSpeed =                     // 
			(OpState.CarDistLong > AheadDist) 
			? OpVelLong - 3 : 200;

		  oInfo.CatchDecel = 999;                // Maximum decel. needed
		}
		else if ((OpState.CarDistLong > 0)       // Distance is > 0 but move together
		  && (OpState.CarDistLat * OpState.CarDiffVelLat < 0))
		{
		  // side collision in T seconds?
		  double T = 
			(fabs(OpState.CarDistLat) - OpState.MinDistLat) / fabs(OpState.CarDiffVelLat);

		  double CollX = OpState.CarDistLong + OpState.CarDiffVelLong * T;
          
		  if ((CollX > AheadDist) 
			&& (CollX < OpState.MinDistLong)) 
		  {
			double RelSpd = (OpState.MinDistLong - OpState.CarDistLong) / T;
			oInfo.Flags |= F_COLLIDE;
			oInfo.CatchTime = T;
			oInfo.CatchSpeed = OpVelLong - 3;
			oInfo.CatchDecel = (MyState.Speed - (OpVelLong - RelSpd)) / T;
		  }
		}
	  }
 
	  if ((oInfo.Flags & (F_REAR | F_AT_SIDE)) 
		&& (MyCar->_laps < CarLaps)
 	    && (OpState.CarDistLong > -10)) 
	  {
		if (LapBackTimer == 0.0)
		  oInfo.Flags |= F_LAPPER;                 // We are lapped
		else
		{
          LapBackTimer -= oDeltaTime;
	      LapBackTimer = MAX(0.0,LapBackTimer);
		}
	  }
	}

	if ((0 < OpState.CarDistLong)
	  && (fabs(OpState.CarDistLong) < OpState.MinDistLong + 2)
	  && (fabs(OpState.CarDistLat) < OpState.MinDistLat + 2))
	{
  	  oInfo.Flags |= F_CLOSE;                    // Opp is close by
	}
  }
  else if (OpState.RelPos < 0)
  {
	if (fabs(OpState.CarDistLat) < OpState.MinDistLat)
  	  oInfo.Flags |= F_BEHIND;                   // Opp. is behind
	oInfo.Flags |= F_REAR;                       // Opp. at rear
  }

  return Result;                                 // Blinking?
} 
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitopponent.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*

