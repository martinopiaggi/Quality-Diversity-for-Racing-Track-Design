//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitdriver.cpp  
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0
// Zentrale Klasse f�r das Fahren bzw. den Fahrer/Roboter
//
// Datei    : unitdriver.cpp 
// Erstellt : 25.11.2007
// Stand    : 22.11.2008
// Copyright: � 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Teile dieser Unit basieren auf diversen Header-Dateien von TORCS
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
//    Copyright: (C) 2006-2007 Tim Foden.
//
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass f�r Fehler, Sch�den,
// Folgefehler oder Folgesch�den keine Haftung �bernommen wird.
//
// Im �brigen gilt f�r die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine sp�tere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*
#include <tmath/v2_t.h>
#include <tgf.h>
#include <robottools.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitcollision.h"
#include "unitdriver.h"
#include "unitlinalg.h"
#include "unitparabel.h"
#include "unitpit.h"
#include "unitstrategy.h"
#include "unittrack.h"


//==========================================================================*
// Statics
//--------------------------------------------------------------------------*
double TDriver::LengthMargin;
bool TDriver::Qualification;
static char *WheelSect[4] =
{SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
//==========================================================================*

//==========================================================================*
// Buffers
//--------------------------------------------------------------------------*
#define BUFLEN 256
static char PathFilenameBuffer[BUFLEN];          // for path and filename
static char PathStatFilenameBuffer[BUFLEN];      // for path and filename
static char TrackNameBuffer[BUFLEN];             // for track name
static char TrackLoadBuffer[BUFLEN];             // for track filename F
static char TrackLoadQualifyBuffer[BUFLEN];      // for track filename F
static char TrackLoadLeftBuffer[BUFLEN];         // for track filename L 
static char TrackLoadRightBuffer[BUFLEN];        // for track filename R
static char PitLoadBuffer[BUFLEN];               // for pit track filename F
static char PitLoadLeftBuffer[BUFLEN];           // for pit track filename L
static char PitLoadRightBuffer[BUFLEN];          // for pit track filename R
//==========================================================================*
/*
//==========================================================================*
// Randomness
//--------------------------------------------------------------------------*
#define RANDOM_SEED 0xfded
#define RANDOM_A    1664525
#define RANDOM_C    1013904223
//==========================================================================*

//==========================================================================*
// Initialize Randomness
//--------------------------------------------------------------------------*
void TDriver::SetRandomSeed(unsigned int Seed)
{
 oRandomSeed = Seed ? Seed : RANDOM_SEED;
 return;
}
//==========================================================================*

//==========================================================================*
// Get Randomness
//--------------------------------------------------------------------------*
unsigned int TDriver::getRandom()
{
 oRandomSeed = RANDOM_A * oRandomSeed + RANDOM_C;
 return (oRandomSeed >> 16);
}
//==========================================================================*
*/
//==========================================================================*
// Interpolation for point Q between two points P0 and P1 (Result in P0)
//--------------------------------------------------------------------------*
void TDriver::InterpolatePointInfo
  (TLanePoint& P0, const TLanePoint& P1, double Q)
{
  double DeltaOAngle = P1.Angle - P0.Angle;

  P0.Crv = TUtils::InterpCurvature(P0.Crv, P1.Crv, Q);
  DOUBLE_NORM_PI_PI(DeltaOAngle);
  P0.Angle = P0.Angle + DeltaOAngle * (1 - Q);
  P0.Offset = Q * P0.Offset + (1 - Q) * P1.Offset;
  P0.Speed = Q * P0.Speed + (1 - Q) * P1.Speed;
}
//==========================================================================*

//==========================================================================*
// Constructor
//--------------------------------------------------------------------------*
TDriver::TDriver(int Index):
  CurrSimTime(0.0),
  oCommonData(NULL),
  oFlyHeight(0.06f),
  oScaleSteer(1.0),
  oStayTogether(10.0),
  oAvoidScale(10.0),
  oAvoidWidth(0.5),
  oGoToPit(false),
  oTeam(NULL),

  oDriveTrainType(cDT_RWD),
  oFlying(0),
  oNbrCars(0),
  oOwnOppIdx(0),

  oAvoidRange(0.5),
  oAvoidRangeDelta(0.0),
  oAvoidOffset(0.0),
  oAvoidOffsetDelta(0.0),
  oMaxAccel(0, 100, 101, 1),
  oLastBrakeCoefIndex(0),
  oLastBrake(0.0),
  oLastTargetSpeed(0.0),

  oAccel(0),
  oBrake(0),
  oClutch(CLUTCH_MAX),
  oGear(0),
  oSteer(0),
  oAlone(true),
  oAngle(0),
  oAngleSpeed(0),
  oBotName(NULL),
  oBrakeDiffInitial(2.0),
  oBrakeForceMax(0.5),
  oBrakeScale(72.0),
  oInitialBrakeCoeff(0.5), 
  oCar(NULL),
  oSteerAngle(0.0f),
  oCarType(NULL),
  oCurrSpeed(0),
  oIndex(0),
  oLastGear(0),
  oLetPass(false),
  oLookAhead(5.0),
  oLookAheadFactor(0.05),
  oLookScale(0.2),
  oLookBase(5.0),
  oOmegaBase(5.0),
  oOmegaScale(0.2),
  oOmegaAheadFactor(0.1),
  oOmegaAhead(5.0),
  oDistFromStart(0.0),
  oShiftMargin(0),
  oSituation(NULL),
  oStartDistance(50.0),
  oStuckCounter(0),
  oSysFooStuckX(NULL),
  oSysFooStuckY(NULL),
  oTestPitStop(0),
  oTrackAngle(0.0),
  oTargetSpeed(0.0),
  oTclRange(10.0),
  oTclSlip(1.6),
  oTrackName(NULL),
  oTrack(NULL),
  oTolerance(2.0),
  oUnstucking(false),
  oWheelRadius(0),
  oDeltaOffset(0.0),
  oDriftAngle(0.0),
  oLetPassSide(0),
  oOldTarget(0.0),
  oReduced(false),
  oCarHandle(NULL),
  oStrategy(NULL),
  oNoAvoidLength(NOAVOIDLENGTH),
  oStartSide(0.0),
  oFuelNeeded(0.0),
  oRepairNeeded(0.0),
  oStanding(true),
  oSpeedScale(0.0),
  oTreatTeamMateAsLapper(false)
{
//  GfOut("TDriver::TDriver() >>>\n");
  int I;
  oIndex = Index;                                // Save own index

  // Motion survey
  oSysFooStuckX = new TSysFoo(1,128);            // Ringbuffer for X
  oSysFooStuckY = new TSysFoo(1,128);            // and Y coordinates

  for (I = 0; I < NBR_BRAKECOEFF; I++)           // Initialize braking
    oBrakeCoeff[I] = 0.5;

  NBRRL = gNBR_RL;                               // Setup number
  oRL_FREE = RL_FREE;                            // and index for
  oRL_LEFT = RL_LEFT;                            // normal races
  oRL_RIGHT = RL_RIGHT;

  TDriver::LengthMargin = LENGTH_MARGIN;

//  GfOut("<<< TDriver::TDriver()\n");
}
//==========================================================================*

//==========================================================================*
// destructor
//--------------------------------------------------------------------------*
TDriver::~TDriver()
{
//  GfOut("TDriver::~TDriver() >>>\n");
  if (oStrategy != NULL)
    delete oStrategy;
  if (oSysFooStuckX != NULL)
    delete oSysFooStuckX;
  if (oSysFooStuckY != NULL)
    delete oSysFooStuckY;
//  GfOut("<<< TDriver::~TDriver()\n");
}
//==========================================================================*

//==========================================================================*
// Set name of robot (an other appendant features)
//--------------------------------------------------------------------------*
void TDriver::SetBotName(char* Value)
{
  // At this point TORCS gives us no information 
  // about the name of the driver, the team and
  // our own car type!
  // Because we want it to set the name as defined
  // in the teams xml file and to load depending 
  // setup files we have to find it out:

  // Initialize the base param path.
  char* PathFilename = PathFilenameBuffer;

  snprintf(PathFilenameBuffer,BUFLEN,            // Build path to
    "drivers/%s/%s.xml"                          // own robot dll from
	,MyBotName,MyBotName);                       // name of own robot

  char SectionBuffer[256];                       // Buffer
  snprintf(SectionBuffer,BUFLEN,                 // Build name of
    "%s/%s/%d"                                   // section from
	,ROB_SECT_ROBOTS,ROB_LIST_INDEX,oIndex);     // Index of own driver
  char* Section = SectionBuffer;

  void* RobotSettings = GfParmReadFile           // Open team setup file
    (PathFilename,GFPARM_RMODE_STD);

  GfOut("\n\n\nPathFilename: %s\n",PathFilename);
  if (RobotSettings)
  {
	oCarType = (char*)GfParmGetStr                      // Get pointer to
      (RobotSettings                             // car type
      , Section                                  // defined in corresponding
      , ROB_ATTR_CAR, DEFAULTCARTYPE);           // section, default car type

	oBotName = Value;                            // Get pointer to drivers name

	oTeamName = (char*) GfParmGetStr                     // Get pointer to
      (RobotSettings                             // drivers team name
      , Section                                  // defined in corresponding
      , ROB_ATTR_TEAM, oCarType);                // section, car type as default

	oRaceNumber = (int) GfParmGetNum             // Get pointer to
      (RobotSettings                             // race number
      , Section, ROB_ATTR_RACENUM                // defined in corresponding
      , (char *) NULL, (tdble) oIndex + 1);      // section, index as default
  }
  else
  {
	GfOut("\n\n FATAL ERROR: File '%s' not found\n\n",PathFilename);
	oCarType = DEFAULTCARTYPE;
	oBotName = Value;
	oTeamName = oCarType;
	oRaceNumber = oIndex + 1;
  }
  GfOut("Bot name: %s\n",oBotName);
  GfOut("Team name: %s\n",oTeamName);
  GfOut("Car type: %s\n",oCarType);
  GfOut("Race number: %d\n",oRaceNumber);

};
//==========================================================================*

//==========================================================================*
// Called for every track change or new race.
//--------------------------------------------------------------------------*
void TDriver::InitTrack
  (PTrack Track, PCarHandle CarHandle,
  PCarSettings *CarSettings, PSituation Situation)
{
  if (CarHandle == NULL)
    CarHandle = NULL;

  oTrack = Track;                                // save pointers
  oSituation = Situation;

  // Initialize race type array
  char* RaceType[] =
    {"practice", "qualify", "race"};

  // Initialize the base param path.
  char BaseParamPath[] = ROBOT_DIR;

  // Get the name of the track
  strncpy(TrackNameBuffer,                       // Copy name of track file
    strrchr(oTrack->filename, '/') + 1,          // from path and filename
	sizeof(TrackNameBuffer));                    // regarding length of buffer
  *strrchr(TrackNameBuffer, '.') = '\0';         // Truncate at point
  oTrackName = TrackNameBuffer;                  // Set pointer to buffer

  // Read/merge car parms
  PCarHandle Handle = NULL;                      // TORCS does not initialize it!
  char Buf[1024];                                // Multi purpose buffer

  // TORCS params for car type (e.g. .../cars/carX-trb1/carX-trb1.xml)
  snprintf(Buf,sizeof(Buf),                      // Build path to
    "cars/%s/%s.xml"                             // cars torcs config file
	,oCarType,oCarType);                         // name of car type
  Handle = TUtils::MergeParamFile(Handle,Buf);
  oMaxFuel = GfParmGetNum(Handle                 // Maximal m�glicher
    , SECT_CAR, PRM_TANK                         //   Tankinhalt
    , (char*) NULL, 100.0);
  GfOut("oMaxFuel (TORCS)   = %.1f\n",oMaxFuel);
  Handle = 0;                                    // Reset handle

  // Default params for car type (e.g. .../ROBOT_DIR/sc-petrol/default.xml)
  snprintf(Buf,sizeof(Buf),"%s/%s/default.xml",
    BaseParamPath,oCarType);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  snprintf(TrackLoadBuffer,sizeof(TrackLoadBuffer),"%s/tracks/%s.trk",
    BaseParamPath,oTrackName);
  oTrackLoad = TrackLoadBuffer;                  // Set pointer to buffer
  snprintf(TrackLoadQualifyBuffer,sizeof(TrackLoadQualifyBuffer),"%s/tracks/%s.trq",
    BaseParamPath,oTrackName);
  oTrackLoadQualify = TrackLoadQualifyBuffer;    // Set pointer to buffer
  snprintf(TrackLoadLeftBuffer,sizeof(TrackLoadLeftBuffer),"%s/tracks/%s.trl",
    BaseParamPath,oTrackName);
  oTrackLoadLeft = TrackLoadLeftBuffer;          // Set pointer to buffer
  snprintf(TrackLoadRightBuffer,sizeof(TrackLoadRightBuffer),"%s/tracks/%s.trr",
    BaseParamPath,oTrackName);
  oTrackLoadRight = TrackLoadRightBuffer;        // Set pointer to buffer
  snprintf(PitLoadBuffer,sizeof(PitLoadBuffer),"%s/tracks/%s.tpk",
    BaseParamPath,oTrackName);
  oPitLoad[0] = PitLoadBuffer;                   // Set pointer to buffer
  snprintf(PitLoadLeftBuffer,sizeof(PitLoadLeftBuffer),"%s/tracks/%s.tpl",
    BaseParamPath,oTrackName);
  oPitLoad[1] = PitLoadLeftBuffer;               // Set pointer to buffer
  snprintf(PitLoadRightBuffer,sizeof(PitLoadRightBuffer),"%s/tracks/%s.tpr",
    BaseParamPath,oTrackName);
  oPitLoad[2] = PitLoadRightBuffer;              // Set pointer to buffer

  // Override params for track (Pitting)
  snprintf(Buf,sizeof(Buf),"%s/tracks/%s.xml",
    BaseParamPath,oTrackName);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Override params for car type with params of track
  snprintf(Buf,sizeof(Buf),"%s/%s/%s.xml",
    BaseParamPath,oCarType,oTrackName);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Override params for car type on track with params of specific race type
  snprintf(Buf,sizeof(Buf),"%s/%s/%s-%s.xml",
    BaseParamPath,oCarType,oTrackName,RaceType[oSituation->_raceType]);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Setup the car param handle to be returned
  *CarSettings = Handle;

  // Get the private parameters now.
  TDriver::LengthMargin =
	GfParmGetNum(Handle,SECT_PRIV,PRV_LENGTH_MARGIN,0,LENGTH_MARGIN);
  GfOut("LengthMargin %.2f\n",TDriver::LengthMargin);

  int TestQualification =
	(int) GfParmGetNum(Handle,SECT_PRIV,PRV_QUALIFICATION,0,0);
  if ((oSituation->_raceType == RM_TYPE_QUALIF)
	|| (TestQualification > 0))
  {
	Qualification = true;
	GfOut("Qualification = True\n");
	NBRRL = 1;
  }

  Param.Fix.oLength =
	GfParmGetNum(Handle,SECT_CAR,PRM_LEN,0,4.5);

  // Adjust pitting ...
  Param.Pit.oUseFirstPit = (int)
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_USE_FIRST,0,1);
  GfOut("oUseFirstPit %d\n",Param.Pit.oUseFirstPit);

  Param.Pit.oUseSmoothPit = (int)
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_USE_SMOOTH,0,1);
  GfOut("oUseSmoothPit %d\n",Param.Pit.oUseSmoothPit);

  Param.Pit.oLaneEntryOffset = 
	GfParmGetNum(Handle,SECT_PRIV,PRV_PITLANE_ENTRY,0,3.0f);
  GfOut("oLaneEntryOffset %g\n",Param.Pit.oLaneEntryOffset);

  Param.Pit.oLaneExitOffset = 
	GfParmGetNum(Handle,SECT_PRIV,PRV_PITLANE_EXIT,0,5.0f);
  GfOut("oLaneExitOffset %g\n",Param.Pit.oLaneExitOffset);

  Param.Pit.oEntryLong =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_ENTRY_LONG,0,0);
  GfOut("oEntryLong %g\n",Param.Pit.oEntryLong);

  Param.Pit.oExitLong =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_EXIT_LONG,0,0);
  GfOut("oExitLong %g\n",Param.Pit.oExitLong);

  Param.Pit.oExitLength =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_EXIT_LEN,0,0);
  GfOut("oExitLength %g\n",Param.Pit.oExitLength);

  Param.Pit.oLatOffset =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_LAT_OFFS,0,0.0);
  GfOut("Lateral Pit Offset %f\n",Param.Pit.oLatOffset);

  Param.Pit.oLongOffset =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_LONG_OFFS,0,0.0);
  GfOut("Longitudinal Pit  Offset %f\n",Param.Pit.oLongOffset);

  Param.oCarParam.oScaleBrakePit =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_SCALE_BRAKE,0,
	(float) MIN(1.0,Param.oCarParam.oScaleBrake));
  GfOut("ScaleBrakePit %g\n",Param.oCarParam.oScaleBrakePit);

  Param.Pit.oStoppingDist =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_STOP_DIST,0,1.5);
  GfOut("oStoppingDist %g\n",Param.Pit.oStoppingDist);

  Param.Fix.oPitBrakeDist =
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_BRAKE_DIST,0,150.0);
  GfOut("oPitBrakeDist %g\n",Param.Fix.oPitBrakeDist);

  oTestPitStop = (int)
	GfParmGetNum(Handle,SECT_PRIV,PRV_PIT_TEST_STOP,0,0);
  GfOut("TestPitStop %d\n",oTestPitStop);
  // ... Adjust pitting
 
  // Adjust driving ...
  Param.oCarParam.oScaleBrake =
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_BRAKE,NULL,0.85f);
  GfOut("Scale Brake: %g\n",Param.oCarParam.oScaleBrake);

  Param.oCarParam.oScaleBump = 
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_BUMP,NULL,Param.oCarParam.oScaleBump);
  Param.oCarParam.oScaleBumpLeft = 
    Param.oCarParam.oScaleBump; 
  Param.oCarParam.oScaleBumpRight = 
    Param.oCarParam.oScaleBump; 
  GfOut("Scale Bump: %g\n",Param.oCarParam.oScaleBump);

  Param.oCarParam.oScaleBumpOuter = 
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_BUMPOUTER,NULL,(float) Param.oCarParam.oScaleBump);
  GfOut("Scale Bump Outer: %g\n",Param.oCarParam.oScaleBumpOuter);

  Param.oCarParam.oScaleMu =
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_MU,NULL,Param.oCarParam.oScaleMu);
  GfOut("Scale Mu: %g\n",Param.oCarParam.oScaleMu);

  Param.oCarParam.oScaleMinMu =
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_MIN_MU,NULL,Param.oCarParam.oScaleMinMu);
  GfOut("Scale Min Mu %g\n",Param.oCarParam.oScaleMinMu);

  oAvoidScale =
	GfParmGetNum(Handle,SECT_PRIV,PRV_AVOID_SCALE,0,oAvoidScale);
  GfOut("oAvoidScale %g\n",oAvoidScale);

  oAvoidWidth =
	GfParmGetNum(Handle,SECT_PRIV,PRV_AVOID_WIDTH,0,oAvoidWidth);
  GfOut("oAvoidWidth %g\n",oAvoidWidth);

  //Param.Fix.oBorderInner =
	//GfParmGetNum(Handle,SECT_PRIV,PRV_BORDER_INNER,0,Param.Fix.oBorderInner);
  //GfOut("Border Inner: %g\n",Param.Fix.oBorderInner);

  //Param.Fix.oBorderOuter =
	//GfParmGetNum(Handle,SECT_PRIV,PRV_BORDER_OUTER,0,Param.Fix.oBorderOuter);
  //GfOut("Border Outer: %g\n",Param.Fix.oBorderOuter);

  //Param.Fix.oMaxBorderInner =
	//GfParmGetNum(Handle,SECT_PRIV,PRV_MAX_BORDER_INNER,0,Param.Fix.oMaxBorderInner);
  //GfOut("Max Border Inner: %g\n",Param.Fix.oMaxBorderInner);

  //Param.Fix.oBorderScale =
	//GfParmGetNum(Handle,SECT_PRIV,PRV_BORDER_SCALE,0,Param.Fix.oBorderScale);
  //GfOut("Border Scale: %g\n",Param.Fix.oBorderScale);

  //oFlyHeight =
  //  GfParmGetNum(Handle,SECT_PRIV,PRV_FLY_HEIGHT,"m",oFlyHeight);
  //GfOut("FLY_HEIGHT %g\n",oFlyHeight);

  oLookAhead = Param.Fix.oLength;
	//GfParmGetNum(Handle,SECT_PRIV,PRV_LOOKAHEAD,0,(float) Param.Fix.oLength);
  //GfOut("LookAhead %g\n",oLookAhead);

  oLookAheadFactor =
	GfParmGetNum(Handle,SECT_PRIV,PRV_LOOKAHEADFACTOR,0,(float)oLookAheadFactor);
  GfOut("LookAheadFactor %g\n",oLookAheadFactor);

  oOmegaAhead = Param.Fix.oLength;
	//GfParmGetNum(Handle,SECT_PRIV,PRV_OMEGAAHEAD,0,(float) Param.Fix.oLength);
  //GfOut("OmegaAhead %g\n",oOmegaAhead);

  //oOmegaAheadFactor =
	//GfParmGetNum(Handle,SECT_PRIV,PRV_OMEGAAHEADFACTOR,0,(float) oOmegaAheadFactor);
  //GfOut("OmegaAheadFactor %g\n",oOmegaAheadFactor);

  oScaleSteer =
	GfParmGetNum(Handle,SECT_PRIV,PRV_SCALE_STEER,0,oScaleSteer);
  GfOut("oScaleSteer %g\n",oScaleSteer);

  oStayTogether =
	GfParmGetNum(Handle,SECT_PRIV,PRV_STAY_TOGETHER,0,0);
  GfOut("oStayTogether %g\n",oStayTogether);

  oInitialBrakeCoeff = oBrakeCoeff[0];
	//GfParmGetNum(Handle,SECT_PRIV,PRV_INIT_BRAKE,0,oBrakeCoeff[0]);
  //GfOut("oInitialBrakeCoeff %g\n",oInitialBrakeCoeff);

  for (int I = 0; I < NBR_BRAKECOEFF; I++)       // Initialize braking
    oBrakeCoeff[I] = oInitialBrakeCoeff;

  oNoAvoidLength =
	GfParmGetNum(Handle,SECT_PRIV,PRV_NO_AVOIDLENGTH,0,(float)oNoAvoidLength);
  GfOut("oNoAvoidLength %g\n",oNoAvoidLength);

  oStartSide =
	GfParmGetNum(Handle,SECT_PRIV,PRV_START_SIDE,0,(float)oStartSide);
  GfOut("oStartSide %g\n",oStartSide);
  // ... Adjust driving

  // Find side and sections of pits ...
  TTrackDescription::PitSideMod PitSideMod;      // Data for track description
  PitSideMod.side = PitSide();                   // Get side of pitlane
  PitSideMod.start =                             // Exclude pits 
	int(GfParmGetNum(Handle,                     //   while getting extra width
	SECT_PRIV,PRV_TRKPIT_START,0,0));            //   starting here  
  PitSideMod.end =                               //   and stopping here
	int(GfParmGetNum(Handle,
	SECT_PRIV,PRV_TRKPIT_END,0,0));
  //GfOut("SIDE MOD %d %d %d\n",PitSideMod.side,PitSideMod.start,PitSideMod.end);
  //GfOut("\n");
  // ... Find side and sections of pits

  // Create track description
  oTrackDesc.InitTrack(oTrack,Param.oCarParam,&PitSideMod); 
  // Create pitting strategy
  oStrategy = new TSimpleStrategy();
  oStrategy->oDriver = this;
  oStrategy->oMaxFuel = oMaxFuel;

  // Setup initial fuel for race ...
  float Fuel = GfParmGetNum(Handle               // Estimate fuel consum
    , SECT_PRIV, PRV_FUELPER100KM                // based on kg per 100 km
    , (char*) NULL
	, TSimpleStrategy::cMAX_FUEL_PER_METER * 100000);

  float Reserve = GfParmGetNum(Handle            // Reserve in m
    , SECT_PRIV, PRV_RESERVE
    , (char*) NULL, 5000);
  GfOut("Reserve: %.0f\n",Reserve);
  oStrategy->oReserve = Reserve;
  oFuelNeeded = 
    oStrategy->SetFuelAtRaceStart                // Fueling and pitting
	  (oTrack,CarSettings,oSituation,Fuel);      //   strategy
  GfOut("oFuelNeeded: %.1f\n",oFuelNeeded);
  // ... Setup initial fuel for race
}
//==========================================================================*

//==========================================================================*
// Start a new race.
//--------------------------------------------------------------------------*
void TDriver::NewRace(PtCarElt Car, PSituation Situation)
{
  //GfOut(">>> TDriver::NewRace()\n");
  oCar = Car;                                    // Save pointers to TORCS
  oCarHandle = CarCarHandle;                     // data of car, car param
  oSituation = Situation;                        // file and situation
  oLastGear = CarGearNbr - 1;                    // Save index of last gear

  OwnCarOppIndex();                              // Find own opponent index

  InitCarModells();                              // Initilize Car modells
  oStrategy->Init(this);                         // Init strategy
  FindRacinglines();                             // Find a good racingline
  TeamInfo();                                    // Find team info

   //save points on the file
  char filename[100];
  sprintf(filename,"%stelemetry/%s.simplix-trajectory.dat",GetDataDir(),oTrackName);
  printf("Saving trajectory to: %s\n",filename);
  oRacingLine[RL_FREE].SaveToFile(filename);
  
  oFlying = 0;                                   // Initialize Flags
  oAvoidRange = 0.5;                             // Relative avoiding offset
  oAvoidRangeDelta = 0.0;                        // Avoiding range change
  oAvoidOffset = CalcPathTarget                  // Get initial offset
	(oTrackDesc.CalcPos(oCar), -CarToMiddle);    // from start grid
  oAvoidOffsetDelta = 0.0;                       // Avoiding speed
  //GfOut("<<< TDriver::NewRace()\n");
}
//==========================================================================*

//==========================================================================*
// Drive
//--------------------------------------------------------------------------*
void TDriver::Drive()
{
/*
  if (CarLaps == 3 + oIndex) 
    oTestPitStop = 1;
  else
    oTestPitStop = 0;
*/
  if (oTestPitStop)                              // If defined, try
    oStrategy->TestPitStop();                    //   to stop in pit

  Propagation();                                 // Propagation

  oAlone = true;                                 // Assume free way to race
  bool Close = false;                            // Assume free way to race
  oLetPass = false;                              // Assume no Lappers

  oAccel = 1.0;                                  // Assume full throttle
  oBrake = 0.0;                                  // Assume no braking

  double Pos = oTrackDesc.CalcPos(oCar);         // Get current pos on track 

  GetPosInfo(Pos,oLanePoint);                    // Get info about pts on track
  oTargetSpeed = oLanePoint.Speed;               // Target for speed control
  AvoidOtherCars(oLanePoint.Crv,Close,oLetPass); // General avoiding
  oSteer = Steering();                           // Steering

  if(Close)                                      // If opponents are close
  {
	BrakingForceRegulatorTraffic();              // Control breaking force
  }
  else if(oStrategy->GoToPit())                  // Going to pitlane
  {
	BrakingForceRegulatorTraffic();              // Control breaking force
  }
  else
  {
	if (oAvoidRange == 0.0)                      // Still avoiding?
      BrakingForceRegulator();                   // Control breaking force
	else
	  BrakingForceRegulatorAvoid();              // Control breaking force
  }

  Clutching();                                   // Tread/Release clutch
  GearTronic();                                  // Shift if needed
  Turning();                                     // Check driving direction
  FlightControl();                               // Prepare landing while flying

  if (IsFullThrottle)                            // Learning only if untroubled
	oMaxAccel.Measurement                        // get samples
	  (CarSpeedLong,CarAccelLong);

  // Filters for brake
  oBrake = FilterBrake(oBrake);
  oBrake = FilterABS(oBrake);
  if (oBrake == 0.0)
  {
    // Filters for throttle
    oAccel = FilterLetPass(oAccel);
	if (!Qualification)
	{
      oAccel = FilterDrifting(oAccel);
      oAccel = FilterTrack(oAccel);
      oAccel = FilterTCL(oAccel);
	}
  } 

  // Tell TORCS what we want do do
  oCar->ctrl.accelCmd = (float) oAccel;
  oCar->ctrl.brakeCmd = (float) oBrake;
  oCar->ctrl.clutchCmd = (float) oClutch;
  oCar->ctrl.gear = oGear;
  oCar->ctrl.steer = (float) oSteer;
  //GfOut("%d: A: %g B: %g C: %g G: %d S: %g\n",oIndex,oAccel,oBrake,oClutch,oGear,oSteer);

  if (oDoAvoid)
    oCar->ctrl.lightCmd = RM_LIGHT_HEAD2;        // Only small lights on
  else
    oCar->ctrl.lightCmd = RM_LIGHT_HEAD1;        // Only big lights on
//    oCar->ctrl.lightCmd =                      // All front lights on
//	    RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;             

  if (!Qualification)                            // Don't use pit while
    oStrategy->CheckPitState(0.6f);               //  qualification
}
//==========================================================================*

//==========================================================================*
// Pitstop callback
//--------------------------------------------------------------------------*
int TDriver::PitCmd()
{
  oStanding = true;                              // Standing, no unstucking!
  oUnstucking = false;                           // Reset pending flag

  // Tell TORCS ...
  oCar->pitcmd.fuel = oStrategy->PitRefuel();    // ... how much to refuel
  oCar->pitcmd.repair = oStrategy->PitRepair();  // and to repair
  oCar->pitcmd.stopType = RM_PIT_REPAIR;         // Set repair flag

  if (oCar->pitcmd.repair > 0)                   // If repairing, show
    GfOut("%s repairing: %d damage\n",       // who and how much
	oBotName,oCar->pitcmd.repair);
  if (oCar->pitcmd.fuel > 0.0)                   // If refueling
    GfOut("%s refueling: %.2f\n",            // show who and how much
	oBotName,oCar->pitcmd.fuel);

  oFuelNeeded += oCar->pitcmd.fuel;
  oRepairNeeded += oCar->pitcmd.repair;

  return ROB_PIT_IM;                             // Ready to be serviced
}
//==========================================================================*

//==========================================================================*
// End of the current race.
//--------------------------------------------------------------------------*
void TDriver::EndRace()
{
  //GfOut("TDriver::EndRace() >>>\n");
  // This is never called by TORCS! Don't use it!
  //GfOut("<<< TDriver::EndRace()\n");
}
//==========================================================================*

//==========================================================================*
// Called before the module is unloaded.
//--------------------------------------------------------------------------*
void TDriver::Shutdown()
{
}
//==========================================================================*

//==========================================================================*
// Side of pitlane
//--------------------------------------------------------------------------*
int TDriver::PitSide()
{
  return oTrack->pits.side == TR_LFT ? -1 : 1;
}
//==========================================================================*

//==========================================================================*
// Find own car in opponents list
//--------------------------------------------------------------------------*
void TDriver::OwnCarOppIndex()
{
  oOwnOppIdx = -1;                               // Mark as undefined

  oNbrCars = oSituation->_ncars;                 // Save Nbr of cars in race
  for (int I = 0; I < oNbrCars; I++)             // Loop all cars
  {
    oOpponents[I].Initialise                     // Initialize opponents
	  (&oTrackDesc, oSituation, I);              //   situation
    if (oSituation->cars[I] == oCar)             // Check if is own car
      oOwnOppIdx = I;                            //   save index
  }
}
//==========================================================================*

//==========================================================================*
// Init drive train
//--------------------------------------------------------------------------*
void TDriver::InitDriveTrain()
{
  oDriveTrainType = cDT_RWD;                     // Assume rear wheel drive
  char* TrainType = (char*) GfParmGetStr(oCarHandle,     // but check it
	SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);

  if (strcmp(TrainType, VAL_TRANS_FWD) == 0)     // If front wheel drive
    oDriveTrainType = cDT_FWD;                   //   change mode
  else if (strcmp(TrainType, VAL_TRANS_4WD) == 0)// and if all wheel drive
    oDriveTrainType = cDT_4WD;                   //   too
}
//==========================================================================*

//==========================================================================*
// FindRacinglines
//--------------------------------------------------------------------------*
void TDriver::FindRacinglines()
{
  GfOut("Update car parameters ...\n");
  Param.Update();                                // update car parameters

  GfOut("... set track ...\n");
  if(oCommonData->Track != oTrackDesc.Track())   // New track?
  {
    oCommonData->Track = oTrackDesc.Track();     // Save pointer
    oCommonData->TeamManager.Clear();            // release old informations
  }

  GfOut("... load smooth path ...\n");
  if (oSituation->_raceType == RM_TYPE_PRACTICE)
  {
    GfOut("... make smooth path ...\n");
    oRacingLine[oRL_FREE].MakeSmoothPath         // Calculate a smooth path
	  (&oTrackDesc, Param,                       // as main racingline
	  TClothoidLane::TOptions(1));
  }
  else if (oSituation->_raceType == RM_TYPE_QUALIF)
  {
    if (!oRacingLine[oRL_FREE].LoadSmoothPath    // Load a smooth path
	  (oTrackLoadQualify,
	  &oTrackDesc, Param,                        // as main racingline
	  TClothoidLane::TOptions(1)))
	{
      GfOut("... make smooth path ...\n");
      oRacingLine[oRL_FREE].MakeSmoothPath       // Calculate a smooth path
	    (&oTrackDesc, Param,                     // as main racingline
	    TClothoidLane::TOptions(1));
      oRacingLine[oRL_FREE].SavePointsToFile(oTrackLoadQualify);
	}
  }
  else if (!oRacingLine[oRL_FREE].LoadSmoothPath // Load a smooth path
	  (oTrackLoad,
	  &oTrackDesc, Param,                        // as main racingline
	  TClothoidLane::TOptions(1)))
  {
    GfOut("... make smooth path ...\n");
    oRacingLine[oRL_FREE].MakeSmoothPath         // Calculate a smooth path
	  (&oTrackDesc, Param,                       // as main racingline
	  TClothoidLane::TOptions(1));
    oRacingLine[oRL_FREE].SavePointsToFile(oTrackLoad);
  }

  if (Qualification)
  {
	oRL_LEFT = oRL_FREE; 
	oRL_RIGHT = oRL_FREE;
  } 
  else 
  {
    Param.oCarParam2.oScaleBumpRight =           // Adjust outer bump scale
	  Param.oCarParam.oScaleBumpOuter;           //   to be able to avoid
	Param.oCarParam2.oScaleMu =                  // Adjust mu scale
	  MIN(0.97,Param.oCarParam.oScaleMu);        //   to be able to avoid
	Param.oCarParam2.oScaleBrake =               // Adjust brake scale
	  MIN(0.95,Param.oCarParam.oScaleBrake);     //   to be able to avoid

    if (!oRacingLine[oRL_LEFT].LoadSmoothPath    // Load a smooth path
	  (oTrackLoadLeft,
	  &oTrackDesc, Param,                        // as avoid to left racingline
	    TClothoidLane::TOptions(1, FLT_MAX, -oAvoidWidth, true)))
	{
      GfOut("... make avoid path left ...\n");

      oRacingLine[oRL_LEFT].MakeSmoothPath       // Avoid to left racingline
	    (&oTrackDesc, Param,
		TClothoidLane::TOptions(1, FLT_MAX, -oAvoidWidth, true));

      oRacingLine[oRL_LEFT].SavePointsToFile(oTrackLoadLeft);
	}

  	Param.oCarParam2.oScaleBumpLeft =            // Adjust outer bump scale
	  Param.oCarParam.oScaleBumpOuter;           //   to be able to avoid
    Param.oCarParam2.oScaleBumpRight =           // Reset outer bump scale
	  Param.oCarParam.oScaleBump;                //   to keep speed

    if (!oRacingLine[oRL_RIGHT].LoadSmoothPath   // Load a smooth path
	  (oTrackLoadRight,
	  &oTrackDesc, Param,                        // as avoid to left racingline
  	    TClothoidLane::TOptions(1, -oAvoidWidth, FLT_MAX, true)))
	{
      GfOut("... make avoid path right ...\n");

	  oRacingLine[oRL_RIGHT].MakeSmoothPath      // Avoid to right racingline
	    (&oTrackDesc, Param,
  	    TClothoidLane::TOptions(1, -oAvoidWidth, FLT_MAX, true));
      oRacingLine[oRL_RIGHT].SavePointsToFile(oTrackLoadRight);
	}

    double MaxPitDist = 0.0;
    for (int I = 0; I < NBRRL; I++)              // Adjust racinglines
    {                                            // using car parameters
	  GfOut("... adjust pit path %d ...\n",I); 
      oStrategy->oPit->oPitLane[I].MakePath
	    (oPitLoad[I],&oRacingLine[I], Param, I);

	  if (MaxPitDist < oStrategy->oPit->oPitLane[I].PitDist())
        MaxPitDist = oStrategy->oPit->oPitLane[I].PitDist();
	}

    oStrategy->oDistToSwitch = MaxPitDist + 100; // Abstand zur Boxeneinfahrt
  }
  GfOut("... Done\n");
}
//==========================================================================*

//==========================================================================*
// Get Team info
//--------------------------------------------------------------------------*
void TDriver::TeamInfo()
{
  oTeam = oCommonData->TeamManager.Add(oCar);
  //GfOut("\n\nTeam: %s\n\n\n",oTeam->TeamName);
}
//==========================================================================*

//==========================================================================*
// Initialize Car modells
//--------------------------------------------------------------------------*
void TDriver::InitCarModells()
{
  oCarParams[0] = &Param.oCarParam;              // Get pointers as array
  oCarParams[1] = &Param.oCarParam2;
  oCarParams[2] = &Param.oCarParam2;

  Param.Initialize(oCar);                        // Initialize parameters

  Param.SetEmptyMass(                            // Set car mass
	GfParmGetNum(oCarHandle,
	  SECT_CAR, PRM_MASS, NULL, 1000.0));

  InitCa();                                      // Ca
  InitCw();                                      // Cw
  InitDriveTrain();                              // Drive train
  InitTireMu();                                  // Tyre friction
  InitWheelRadius();                             // Wheel radius
  InitAdaptiveShiftLevels();                     // Gear shifting

  Param.Tmp.oFuel = 0;                           // Still unfueld
  Param.Fix.oWidth = CarWidth;                   // width of car
  Param.oCarParam2 = Param.oCarParam;            // Copy to avoid set
  Param.oCarParam2.oScaleMu =                    // Adjust mu scale to
	MIN(0.95, 0.9 * Param.oCarParam.oScaleMu);   //   be able to avoid
  Param.oCarParam3 = Param.oCarParam;            // Copy to pit set
}
//==========================================================================*

//==========================================================================*
// Update pointers and multiple used values
//--------------------------------------------------------------------------*
void TDriver::Update(tCarElt* Car, tSituation* S)
{
  oCar = Car;                                    // Update pointers
  oSituation = S;

  // Shortcuts
  oCurrSpeed = myhypot(CarSpeedLong, CarSpeedLat); // Save currend speed
  if (fabs(oCurrSpeed) < 1)                      // At slow speeds use
    oAngleSpeed = CarYaw;                        // direction of cars x-axis
  else                                           // else use
    oAngleSpeed = atan2(CarSpeedY, CarSpeedX);   // direction of movement

  oTrackAngle =                                  // Direction of track at the
	 RtTrackSideTgAngleL(&CarTrackPos);          // position of the car
  oDistFromStart = oTrackDesc.CalcPos(oCar, 0.0);// Cars distance from Start
  TVec2d Target =                                // Target to steer to
	CalcPathTarget2(oDistFromStart + 5.0, 0.0);  // while unstucking
  oSteerAngle = (float) atan2                    // Direction to steer
	(Target.y - CarPosY, Target.x - CarPosX);    //   from here to target
  oSteerAngle -= (float) CarYaw;                 // Relative to cars x-axis
//  oSteerAngle -= (float) oAngleSpeed;            // Relative to cars movement
  FLOAT_NORM_PI_PI(oSteerAngle);                 // Normalize to -PI,+PI

  oDriftAngle =                                  // Actual drift angle
	atan2(CarSpeedY, CarSpeedX) - CarYaw;
  DOUBLE_NORM_PI_PI(oDriftAngle);                // normalized to +Pi .. -Pi

  // Get direction of motion
  double MySpd = MAX(0.01,myhypot(CarSpeedX, CarSpeedY));
  double MyDomX = CarSpeedX / MySpd;
  double MyDomY = CarSpeedY / MySpd;

  // Update all opponents data
  for (int I = 0; I < oNbrCars; I++)
  {
	oOpponents[I].Update(oCar,
	  &oCommonData->TeamManager, MyDomX, MyDomY);
  }

  oStrategy->Update(oCar);                       // Update strategic params
}
//==========================================================================*

//==========================================================================*
// Detect flight
//--------------------------------------------------------------------------*
void TDriver::DetectFlight()
{
  double H[4];
  double HMax = 0.0;

  for (int I = 0; I < 4; I++)
  {
    tTrkLocPos Wp;
    float Wx = oCar->pub.DynGCg.pos.x;
    float Wy = oCar->pub.DynGCg.pos.y;
    RtTrackGlobal2Local(CarSeg, Wx, Wy, &Wp, TR_LPOS_SEGMENT);
    H[I] = CarPosZ - RtTrackHeightL(&Wp) - WheelRad(I);
	if (HMax < H[I])
      HMax = H[I];
  }

  if (HMax > oFlyHeight)
  {
    oFlying = MIN(FLY_COUNT, oFlying + (FLY_COUNT / 2));
  }
  else if (oFlying > 0)
  {
    oFlying--;
  }
}
//==========================================================================*

//==========================================================================*
// Prepare landing
//--------------------------------------------------------------------------*
void TDriver::FlightControl()
{ 
  if (oFlying)
  {
    // Steer in direction of car movement
    double Angle = oAngleSpeed - CarYaw;
    DOUBLE_NORM_PI_PI(Angle);
    int F = FLY_COUNT - oFlying;
    double T = MAX(0, MIN(1.0 * F / FLY_COUNT, 1));
    oSteer = oSteer * T + (1 - T) * Angle / SteerLock;
  }
}
//==========================================================================*

//==========================================================================*
// Propagation
//--------------------------------------------------------------------------*
void TDriver::Propagation()
{
  if (Param.Tmp.Needed())
  {
	Param.Update();
    
	for (int I = 0; I < NBRRL; I++)
	{
      oRacingLine[I].CalcMaxSpeeds(1);
	  oRacingLine[I].PropagateBreaking(1);
      oRacingLine[I].PropagateAcceleration(1);
	}
  }
} 
//==========================================================================*

//==========================================================================*
// Steering
//--------------------------------------------------------------------------*
double TDriver::Steering()
{
  TLanePoint AheadPointInfo;
  if (oUnstucking)
  {
    double Factor = 4.0 * MAX(0,MIN(1.0,CarSpeedLong));
    double Angle = UnstuckSteerAngle(oLanePoint,AheadPointInfo) * Factor;
    oAngle = SteerAngle(AheadPointInfo);
    double T = MAX(0.0,7 - MAX(0,CarSpeedLong));
	oAngle = Angle * T + oAngle * (1 - T); 
  }
  else
    oAngle = SteerAngle(AheadPointInfo);
  oDeltaOffset = oLanePoint.Offset + CarToMiddle;// Delta to planned offset
  return oAngle / SteerLock;
}
//==========================================================================*

//==========================================================================*
// Calculate mean wheel radius
//--------------------------------------------------------------------------*
void TDriver::InitWheelRadius()
{
  int Count = 0;
  oWheelRadius = 0.0;

  if(HasDriveTrainFront)
  {
    oWheelRadius += WheelRad(FRNT_LFT) + WheelRad(FRNT_RGT);
    Count += 2;
  }

  if(HasDriveTrainRear)
  {
    oWheelRadius += WheelRad(REAR_LFT) + WheelRad(REAR_RGT);
    Count += 2;
  }
  oWheelRadius /= Count;
}
//==========================================================================*

//==========================================================================*
// Initialize tire mu
//--------------------------------------------------------------------------*
void TDriver::InitTireMu()
{
  int I;

  Param.Fix.oTyreMuFront = FLT_MAX;
  for (I = 0; I < 2; I++)
	Param.Fix.oTyreMuFront = MIN(Param.Fix.oTyreMuFront,
	  GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_MU, (char*) NULL, 1.0f));

  Param.Fix.oTyreMuRear = FLT_MAX;
  for (I = 2; I < 4; I++)
	Param.Fix.oTyreMuRear = MIN(Param.Fix.oTyreMuRear,
	  GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_MU, (char*) NULL, 1.0f));

  Param.Fix.oTyreMu = 
	MIN(Param.Fix.oTyreMuFront,Param.Fix.oTyreMuRear);
}
//==========================================================================*

//==========================================================================*
// Initialize Ca
//--------------------------------------------------------------------------*
void TDriver::InitCa()
{
  float FrontWingArea =
	GfParmGetNum(oCarHandle, SECT_FRNTWING,
	  PRM_WINGAREA, (char*) NULL, 0.0);
  float FrontWingAngle =
	GfParmGetNum(oCarHandle, SECT_FRNTWING,
	  PRM_WINGANGLE, (char*) NULL, 0.0);
  float RearWingArea =
	GfParmGetNum(oCarHandle, SECT_REARWING,
	  PRM_WINGAREA, (char*) NULL, 0.0f);
  float RearWingAngle =
	GfParmGetNum(oCarHandle, SECT_REARWING,
	   PRM_WINGANGLE, (char*) NULL, 0.0f);

  FrontWingArea = FrontWingArea * sin(FrontWingAngle);
  RearWingArea = RearWingArea * sin(RearWingAngle);
  float WingCd = (float) (1.23 * (FrontWingArea + RearWingArea));
  Param.Fix.oCdWing = WingCd;

  float CL =
	GfParmGetNum(oCarHandle, SECT_AERODYNAMICS,
	  PRM_FCL, (char*) NULL, 0.0f)
	+ GfParmGetNum(oCarHandle, SECT_AERODYNAMICS,
	  PRM_RCL, (char*) NULL, 0.0f);

  float H = 0.0;
  int I;
  for (I = 0; I < 4; I++)
	H += GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_RIDEHEIGHT, (char*) NULL, 0.20f);

  H *= 1.5;
  H = H*H;
  H = H*H;
  H = (float) (2.0 * exp(-3.0 * H));
  Param.Fix.oCa = H * CL + 4.0 * WingCd;
  Param.Fix.oCaFrontWing = 4 * 1.23 * FrontWingArea;
  Param.Fix.oCaRearWing = 4 * 1.23 * RearWingArea;
  Param.Fix.oCaGroundEffect = H * CL;

}
//==========================================================================*

//==========================================================================*
// Initialize Cw
//--------------------------------------------------------------------------*
void TDriver::InitCw()
{
  float Cx =
	GfParmGetNum(oCarHandle,
	SECT_AERODYNAMICS, PRM_CX, (char*) NULL, 0.0f);
  float FrontArea =
	GfParmGetNum(oCarHandle,
	SECT_AERODYNAMICS, PRM_FRNTAREA, (char*) NULL, 0.0f);

  Param.Fix.oCdBody = 0.645 * Cx * FrontArea;
}
//==========================================================================*

//==========================================================================*
// Get gear ratio
//--------------------------------------------------------------------------*
double TDriver::GearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset];
}
//==========================================================================*

//==========================================================================*
// Get gear ratio of previous gear
//--------------------------------------------------------------------------*
double TDriver::PrevGearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset-1];
}
//==========================================================================*

//==========================================================================*
// Get gear ratio of next gear
//--------------------------------------------------------------------------*
double TDriver::NextGearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset+1];
}
//==========================================================================*

//==========================================================================*
// Simplified clutch controller
//--------------------------------------------------------------------------*
void TDriver::Clutching()
{
  double ClutchMax = CLUTCH_MAX;
  if (CurrSimTime < oSituation->deltaTime)
    oClutch = ClutchMax;

//  if (CurrSimTime > 10.0)
//	  GfOut("D: %g\n",DistanceRaced);

  if(oClutch > 0) 
  {
    double Delta = CLUTCH_DELTA;
    if (oGear < 2)
	{
	  Delta /= 2;
      ClutchMax *= 1.3;
      if (CurrSimTime < 1.5)
        oClutch = ClutchMax;
	}

	oClutch = MIN(ClutchMax,oClutch);
	if(oClutch == ClutchMax)
	{
	  if(GearRatio() * CarSpeedLong
		  / (oWheelRadius * CarRpm) > CLUTCH_RANGE)
	  {
        oClutch = ClutchMax - 0.01;
	  }

	}
	else
	{
	  oClutch -= Delta;
	  oClutch = MAX(0.0,oClutch);
	}
  }
}
//==========================================================================*

//==========================================================================*
// Turn if driving backwards unexpectedly
//--------------------------------------------------------------------------*
void TDriver::Turning()
{
  if (!oUnstucking && (DistanceRaced > 10))
  {
    double Angle = oLanePoint.Angle - CarYaw;    // Direction moving to
    DOUBLE_NORM_PI_PI(Angle);                    // normalize it

    if((oGear > 0)                               // If a gear is selected
      && (fabs(Angle) > 75 * PI / 180))          // but irregular direction
    {
      if(Angle * CarToMiddle < 0)                // Lets turn
  	  {
        oGear = -1;
        oAccel = 0.5;
        oBrake = 0;
        oSteer = -SGN(Angle);
	  }
    }

    if((oGear > 0) && (CarSpeedLong < -0.01))    // Rolling back?
    {                                            // Shift down and start
      oGear = 1;
      oBrake = CarSpeedLong < -0.5 ? 0.25 : 0;
      oAccel = 0.25;
    }

    if ((oGear == 1)                             // If starting
	  && (CarSpeedLong < 10)                     //   and slow
	  && (fabs(CarSpeedLong) >= 0.01)            //   but moving
	  && (oAccel == 1.0 && oBrake == 0))         //   and acc. (not braking)
    {                                            // use clutch
      double rpm = CarRpm;
      oClutch = (850 - rpm) / 400;
      if(CarSpeedLong < 0.05)
        oClutch = 0.5;

      oClutch = MAX(0, MIN(oClutch, 0.9));       // Normalize
    }
  }
}
//==========================================================================*

//==========================================================================*
// Calculate shift levels
//--------------------------------------------------------------------------*
void TDriver::InitAdaptiveShiftLevels()
{
  //GfOut("TDriver::InitShiftLevels() >>>\n");

  struct tEdesc
  {
    tdble rpm;
    tdble tq;
  } *Edesc;

  struct TDataPoints
  {
    tdble rads;
    tdble a;
    tdble b;
  } *DataPoints;

  float RevsMax;
  float Tickover;
  float RevsLimiter;
  double RpmFactor = 30 / PI;                    // Unit conversion

  char	idx[64];
  sprintf(idx, "%s/%s", SECT_ENGINE, ARR_DATAPTS);
  int IMax = GfParmGetEltNb(oCarHandle, idx);
  //GfOut("\nIMax: %d\n",IMax);

  RevsMax = GfParmGetNum(oCarHandle, SECT_ENGINE,
	PRM_REVSMAX, (char*)NULL, 1000);
  //GfOut("RevsMax: %f = %0.0f [rpm]\n\n",RevsMax,RevsMax*RpmFactor);

  Tickover = GfParmGetNum(oCarHandle, SECT_ENGINE,
	  PRM_TICKOVER, (char*)NULL, 150);
  //GfOut("Tickover: %f = %0.0f [rpm]\n\n",Tickover,Tickover*RpmFactor);

  RevsLimiter = GfParmGetNum(oCarHandle, SECT_ENGINE,
	  PRM_REVSLIM, (char*)NULL, 800);
  //GfOut("RevsLimiter: %f = %0.0f [rpm]\n",RevsLimiter,RevsLimiter*RpmFactor);

  Edesc = (struct tEdesc*) malloc((IMax + 1) * sizeof(struct tEdesc));

  int I;

  oShiftMargin = 0.9;                            //
  for (I = 0; I < MAX_GEARS; I++)
  {
    oShift[I] = 2000.0;
    oGearEff[I] = 0.95;
  }

  for (I = 0; I < IMax; I++)
  {
	sprintf(idx, "%s/%s/%d", SECT_ENGINE, ARR_DATAPTS, I+1);
    Edesc[I].rpm = GfParmGetNum(oCarHandle, idx,
	  PRM_RPM, (char*) NULL, RevsMax);
    Edesc[I].tq = GfParmGetNum(oCarHandle, idx,
	  PRM_TQ, (char*) NULL, 0.0f);

	//GfOut("rpm: %f = %0.0f [rpm] tq: %0.1f\n",Edesc[I].rpm,Edesc[I].rpm*RpmFactor,Edesc[I].tq);
  }

  Edesc[IMax].rpm = Edesc[IMax - 1].rpm;
  Edesc[IMax].tq  = Edesc[IMax - 1].tq;
  //GfOut("rpm: %f = %0.0f [rpm] tq: %0.1f\n",Edesc[IMax].rpm,Edesc[IMax].rpm*RpmFactor,Edesc[IMax].tq);
  //GfOut("\n");

  double maxTq = 0;
  double rpmMaxTq = 0;
  double maxPw = 0;
  double rpmMaxPw = 0;
  double TqAtMaxPw = 0;
  DataPoints = (TDataPoints *) malloc(IMax * sizeof(TDataPoints));
  TDataPoints *Data;
  for (I = 0; I < IMax; I++)
  {
	Data = &DataPoints[I];

	Data->rads = Edesc[I+1].rpm;
	if ((Data->rads >= Tickover)
			&& (Edesc[I+1].tq > maxTq)
			&& (Data->rads < RevsLimiter))
	{
	  maxTq = Edesc[I+1].tq;
	  rpmMaxTq = Data->rads;
	}
	if ((Data->rads >= Tickover)
			&& (Data->rads * Edesc[I+1].tq > maxPw)
			&& (Data->rads < RevsLimiter))
	{
	  TqAtMaxPw = Edesc[I+1].tq;
	  maxPw = Data->rads * Edesc[I+1].tq;
	  rpmMaxPw = Data->rads;
	}

	Data->a = (Edesc[I+1].tq - Edesc[I].tq)
	  / (Edesc[I+1].rpm - Edesc[I].rpm);
	Data->b = Edesc[I].tq - Data->a * Edesc[I].rpm;
  }

  //GfOut("maxTq: %f rpmMaxTq: %f maxPw: %f rpmMaxPw: %f\n",
  //  maxTq,rpmMaxTq*RpmFactor,maxPw,rpmMaxPw*RpmFactor);
  //GfOut("TqAtMaxPw: %f\n", TqAtMaxPw);

/*
  for (I = 0; I < IMax; I++)
  {
	Data = &DataPoints[I];
	GfOut("rads: %f a: %f b: %f\n",Data->rads,Data->a,Data->b);
  }
  GfOut("\n");
*/

  //GfOut("Nbr of Gears: %d\n",CarGearNbr - 1); 
  for (I = 0; I < CarGearNbr - 1; I++)
  {
	sprintf(idx, "%s/%s/%d", SECT_GEARBOX, ARR_GEARS, I+1);
    oGearEff[I] = GfParmGetNum(oCarHandle, idx,
	  PRM_EFFICIENCY, (char*) NULL, 0.94f);

	//GfOut("Gear: %d Efficency: %f\n",I+1,oGearEff[I]);
  }

  int J;
  for (J = 0; J < CarGearNbr; J++)
    oShift[J] = RevsLimiter * 0.974;

  //GfOut("\n\n");
  for (J = 1; J < oLastGear; J++)
  {
      double Rpm = Tickover;
      double RpmNext = Tickover;
      //double BrakeCoeff = 0.33f;
      double Tq = 0.0;
      double TqNext = 0.0;
      double GearRatioAct;
      double GearRatioNext;

      while (Rpm <= RevsLimiter)
	  {
		for (I = 0; I < IMax; I++)
		{
			if (Rpm < DataPoints[I].rads)
			{
				Tq = (Rpm * DataPoints[I].a
				  + DataPoints[I].b) * oGearEff[J-1];
				break;
			}
		}

        GearRatioAct = CarGearRatio[J + CarGearOffset];
        GearRatioNext = CarGearRatio[J + 1 + CarGearOffset];
        RpmNext = Rpm * GearRatioNext / GearRatioAct;
		for (I = 0; I < IMax; I++)
		{
			if (RpmNext < DataPoints[I].rads)
			{
				TqNext = (RpmNext * DataPoints[I].a
				  + DataPoints[I].b) * GearRatioNext
				  / GearRatioAct * oGearEff[J];
		        //GfOut("J: %d UD: %0.3f Rpm: %0.0f Rpm: %0.0f Tq; %0.1f RpmNext: %0.0f TqNext; %0.1f\n",
		        //  J, Rpm / RevsLimiter, oShift[J], Rpm*RpmFactor, Tq, RpmNext*RpmFactor, TqNext);
				break;
			}
		}

        if ((TqNext > Tq ) && (Rpm*RpmFactor > 2000))
		{
		  oShift[J] = Rpm * 0.98;
		  //GfOut("J: %d UD: %0.3f Shift: %0.0f Rpm: %0.0f Tq; %0.1f RpmNext: %0.0f TqNext; %0.1f\n",
		  //  J, Rpm / RevsLimiter, oShift[J]*RpmFactor, Rpm*RpmFactor, Tq, RpmNext*RpmFactor, TqNext);
		  break;
		}
 	    Rpm += 1;
	  }
	  //GfOut("J: %d UD: %0.3f Rpm: %0.0f [rpm] Shift: %0.5f\n",
	  //  J, oShift[J] / RevsLimiter, oShift[J]*RpmFactor, oShift[J]);
  }
  //GfOut("\n\n");

  free(DataPoints);
  free(Edesc);

  //GfOut("<<< TDriver::InitShiftLevels()\n");
}
//==========================================================================*

//==========================================================================*
// S�GCuASL ;D
// = Simplified sequential gear controller using adaptive shift levels
//--------------------------------------------------------------------------*
void TDriver::GearTronic()
{
  if (IsTickover)
  {
    oGear = 1;
  }
  else
  {
    if((UsedGear < oLastGear)
	  && (GearRatio() * CarSpeedLong / oWheelRadius > NextRpm))
	{
      oUnstucking = false;
      TreadClutch;
	  oGear = NextGear;
	}
    else if(UsedGear > 1)
	{
      double PrevRpm =
  	    oShift[UsedGear-1] * oShiftMargin
	    * GearRatio() / PrevGearRatio();

      if(GearRatio() * CarSpeedLong / oWheelRadius < PrevRpm)
	  {
	    TreadClutch;
	    oGear = PrevGear;
	  }
	}
  }
}
//==========================================================================*

//==========================================================================*
// Get info to point
//--------------------------------------------------------------------------*
void TDriver::GetLanePoint(int Path, double Pos, TLanePoint& LanePoint)
{
  if (oStrategy->oPit != NULL && !oStrategy->oWasInPit
	&& oStrategy->GoToPit() && oStrategy->oPit->oPitLane[Path].ContainsPos(Pos))
  {
    //GfOut("+");
    oStrategy->oPit->oPitLane[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = 0.05;
	oOmegaScale = 0.2;
	oLookBase = Param.Fix.oLength / 4;
	oOmegaBase = Param.Fix.oLength / 2;
	oGoToPit = true;
  }
  else if (oStrategy->oPit != NULL && oStrategy->oWasInPit
	&& oStrategy->oPit->oPitLane[Path].ContainsPos(Pos))
  {
    //GfOut("-");
    oStrategy->oPit->oPitLane[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = 0.02;
	oOmegaScale = 0.2;
	oLookBase = Param.Fix.oLength / 10;
	oOmegaBase = Param.Fix.oLength / 2;
	oGoToPit = true;
  }
  else
  {
    //GfOut("*");
    oRacingLine[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = oLookAheadFactor;
	oOmegaScale = oOmegaAheadFactor;
	oLookBase = oLookAhead;
	oOmegaBase = oOmegaAhead;
	oGoToPit = false;
  }
}
//==========================================================================*

//==========================================================================*
// Get info to position
//--------------------------------------------------------------------------*
void TDriver::GetPosInfo
  (double Pos, TLanePoint& PointInfo, double U, double V )
{
  GetLanePoint(oRL_FREE, Pos, PointInfo);

  if(U != 0.0)
  {
    TLanePoint PointInfoL, PointInfoR;
    GetLanePoint(oRL_LEFT,  Pos, PointInfoL );
    GetLanePoint(oRL_RIGHT, Pos, PointInfoR );

    double T = (1.0 - V) * 0.5;

    InterpolatePointInfo(PointInfoL, PointInfo, U);
    InterpolatePointInfo(PointInfoR, PointInfo, U);

    PointInfo = PointInfoL;

    InterpolatePointInfo(PointInfo, PointInfoR, T);
  }
}
//==========================================================================*

//==========================================================================*
// Get info to position
//--------------------------------------------------------------------------*
void TDriver::GetPosInfo(double Pos, TLanePoint& PointInfo)
{
  GetPosInfo(Pos, PointInfo, oAvoidRange, oAvoidOffset);
}
//==========================================================================*

//==========================================================================*
// Calculate path target
//--------------------------------------------------------------------------*
double TDriver::CalcPathTarget(double Pos, double Offset)
{
  TLanePoint PointInfo, PointInfoL, PointInfoR;

  GetLanePoint(oRL_FREE,Pos,PointInfo);
  GetLanePoint(oRL_LEFT,Pos,PointInfoL);
  GetLanePoint(oRL_RIGHT,Pos,PointInfoR);

  InterpolatePointInfo(PointInfoL,PointInfo,oAvoidRange);
  InterpolatePointInfo(PointInfoR,PointInfo,oAvoidRange);

  double T = (Offset - PointInfoL.Offset) / (PointInfoR.Offset - PointInfoL.Offset);

  return MAX(-1, MIN(T, 1)) * 2 - 1;
}
//==========================================================================*

//==========================================================================*
// Calculate path target
//--------------------------------------------------------------------------*
TVec2d TDriver::CalcPathTarget2(double Pos, double Offset)
{
  TLanePoint PointInfo, PointInfoL, PointInfoR;

  GetLanePoint(oRL_FREE,Pos,PointInfo);
  GetLanePoint(oRL_LEFT,Pos,PointInfoL);
  GetLanePoint(oRL_RIGHT,Pos,PointInfoR);

  InterpolatePointInfo(PointInfoL,PointInfo,oAvoidRange);
  InterpolatePointInfo(PointInfoR,PointInfo,oAvoidRange);

  double T = (Offset - PointInfoL.Offset) / (PointInfoR.Offset - PointInfoL.Offset);

  return TVec2d(MAX(-1, MIN(T, 1)) * 2 - 1, 1);
}
//==========================================================================*

//==========================================================================*
// Calculate path to left and to right
//--------------------------------------------------------------------------*
void TDriver::GetPathToLeftAndRight
  (const PCarElt pCar, double& ToL, double& ToR)
{
  double Pos = pCar->_distFromStartLine;
  double Offset = -pCar->_trkPos.toMiddle;

  TLanePoint PointInfo;
  GetLanePoint(oRL_LEFT,Pos,PointInfo);
  ToL = -(PointInfo.Offset - Offset);
  GetLanePoint(oRL_RIGHT,Pos,PointInfo);
  ToR = PointInfo.Offset - Offset;
}
//==========================================================================*

//==========================================================================*
// Steering angle
//--------------------------------------------------------------------------*
double TDriver::SteerAngle(TLanePoint& AheadPointInfo)
{
  // Look this far ahead.
  double AheadDist = oLookBase + oCurrSpeed * oLookScale;
  if (oGoToPit)
	AheadDist = 2.0;
  double AheadPos = oTrackDesc.CalcPos(oCar, AheadDist);

  // Get info about pts on track.
  GetPosInfo(AheadPos,AheadPointInfo);

  TLanePoint PointInfoOmega;
  double AheadOmega = oOmegaBase + oCurrSpeed * oOmegaScale;
  double AheadOmegaPos = oTrackDesc.CalcPos(oCar, AheadOmega);
  GetPosInfo(AheadOmegaPos,PointInfoOmega);

  // Work out basic steering angle.
  double Angle = AheadPointInfo.Angle - CarYaw;
  DOUBLE_NORM_PI_PI(Angle);

  if (oCurrSpeed < SLOWSPEED)
    return Angle;

  double Delta = oLanePoint.Offset + CarToMiddle;

  // Control rotational velocity.
  double AvgK = (oLanePoint.Crv + PointInfoOmega.Crv) / 2;
  double Omega = CarSpeedLong * AvgK;
  double O2 = (AheadPointInfo.Crv - oLanePoint.Crv) * oCurrSpeed / AheadDist;

  Angle += 0.08 * (Omega - CarYawRate);
  Angle += O2 * 0.08;
  Angle += AvgK * oScaleSteer;

  // control offset from path.
  oPIDCLine.oP = 1.0;
  oPIDCLine.oD = 10;
  Angle -= 0.15 * atan(oPIDCLine.Sample(Delta));

  return Angle;
}
//==========================================================================*

//==========================================================================*
// Unstuck Steering angle
//--------------------------------------------------------------------------*
double TDriver::UnstuckSteerAngle
  (TLanePoint& PointInfo, TLanePoint& AheadPointInfo)
{
  // Get current pos on track.
  double Pos = oTrackDesc.CalcPos(oCar);

  // Look this far ahead.
  double AheadDist = 3.0;
  double AheadPos = oTrackDesc.CalcPos(oCar, AheadDist);

  // Get info about pts on track.
  GetPosInfo(Pos,PointInfo);
  GetPosInfo(AheadPos,AheadPointInfo);

  // Work out basic steering angle.
  double Angle = AheadPointInfo.Angle - CarYaw;
  if (CarToMiddle < 0)
	  Angle += 0.5 * fabs(CarToMiddle/(2*oTrack->width));
  else
    Angle -= 0.5 * fabs(CarToMiddle/(2*oTrack->width));
  DOUBLE_NORM_PI_PI(Angle);
  return Angle;
}
//==========================================================================*

//==========================================================================*
// Control brake press
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulator()
{
  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
    if (Diff > oBrakeDiffInitial)
	{
	  oAccel = 0;
      oBrake = MIN(oBrakeForceMax,Diff * Diff / oBrakeScale);
//	  GfOut("Diff1: %g oBrake: %g\n",Diff,oBrake);
	}
	else
	{
	  if (oTargetSpeed > 1)
	  {
	    oAccel = MIN(oAccel, 0.25);
		oBrake = 0.0;
//      GfOut("Diff2: %g oBrake: %g\n",Diff,oBrake);
	  }
	  else
	  {
		oAccel = 0;
		oBrake = 0.1;
//	    GfOut("Diff3: %g oBrake: %g\n",Diff,oBrake);
	  }
	}
  }

  if ((oLastBrake > 0)
	&& (oBrake > 0)
	&& (Diff < 6))
  {
	//GfOut("*");
	oBrake = 0;
	oAccel = 0.06;
//  GfOut("Diff4: %g oBrake: %g\n",Diff,oBrake);
  }

  oLastBrake = oBrake;
  oLastTargetSpeed = 0;
}
//==========================================================================*

//==========================================================================*
// Control brake press while avoiding
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulatorAvoid()
{
  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
    if (Diff > 1.0)
	{
	  oAccel = 0;
      oBrake = MIN(oBrakeForceMax,Diff * Diff / oBrakeScale);
//	  GfOut("Diff5: %g oBrake: %g\n",Diff,oBrake);
	}
	else
	{
	  if (oTargetSpeed > 1)
	  {
		oAccel = MIN(oAccel, 0.25);
        oBrake = 0.0;
//      GfOut("Diff6: %g oBrake: %g\n",Diff,oBrake);
	  }
	  else
	  {
		oAccel = 0;
		oBrake = 0.1;
//      GfOut("Diff7: %g oBrake: %g\n",Diff,oBrake);
	  }
	}
  }

  oLastTargetSpeed = 0;
}
//==========================================================================*

//==========================================================================*
// Control brake press in traffic
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulatorTraffic()
{
  double Err = 0.0;
  if(oLastBrake && oLastTargetSpeed) 
  {
    Err = oCurrSpeed - oLastTargetSpeed;
    oBrakeCoeff[oLastBrakeCoefIndex] += Err * 0.001;
	oLastBrake = 0;
	oLastTargetSpeed = 0;
  }

  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
	int	B = int(floor(oCurrSpeed/2)); 
	oAccel = 0;
	oBrake = MAX(0, MIN(oBrakeCoeff[B] * Diff, oBrakeForceMax));
//    Err = 1.0 + MIN(Err,0.5);
//    oBrake = Err * oBrake;
	oLastBrakeCoefIndex = B;
	oLastBrake = oBrake;
	oLastTargetSpeed = 0;
//	GfOut("Diff8: %g oBrake: %g CS: %g TS: %g Err: %g\n",Diff,oBrake,oCurrSpeed,oTargetSpeed,Err);

	if ((oBrake > 0) && (oBrake < oBrakeForceMax))
//	if (oBrake > 0)
	{
	  if (oTargetSpeed > 0)
		oLastTargetSpeed = oTargetSpeed;
	}
  }
  else
  {
	//GfOut("Diff9: %g oBrake: %g CS: %g TS: %g Err: %g\n",Diff,oBrake,oCurrSpeed,oTargetSpeed,Err);
  }
}
//==========================================================================*

//==========================================================================*
// Evaluate collision flags
//--------------------------------------------------------------------------*
void TDriver::EvaluateCollisionFlags(
  int I, 
  TCollision::TCollInfo& Coll,
  double Crv,
  double& MinCatchTime,
  double& MinCatchAccTime,
  double& MinVCatTime,
  bool& IsLapper)
{
  TOpponent::TInfo& OppInfo =                    // Information about opponent
    oOpponents[I].Info();                        // collected at classification

  PCarElt OppCar = oOpponents[I].Car();          // TORCS data of opponents car

  Coll.Flags |= OppInfo.Flags;                   // subsume the collision flags
  for (int I = 0; I < MAXBLOCKED; I++)
    Coll.Blocked[I] |= OppInfo.Blocked[I];       // subsume the blocked flags

  if (OppInfo.GotFlags(F_FRONT))                 // Is opponent in front of us
  {
	if (OppInfo.GotFlags(F_COLLIDE) 
	  && (OppInfo.CatchDecel > 12.5 * CarFriction))
	  Coll.TargetSpeed = MIN(Coll.TargetSpeed, OppInfo.CatchSpeed);

	if (OppInfo.Flags & (F_COLLIDE | F_CATCHING))
	  MinCatchTime = MIN(MinCatchTime, OppInfo.CatchTime);

	if (OppInfo.Flags & F_CATCHING_ACC)
	  MinCatchAccTime = MIN(MinCatchAccTime, OppInfo.CatchAccTime);

	if (OppInfo.State.CarDiffVelLong < 0)
	{
	  double VCatTime =
		-(OppInfo.State.CarDistLong - OppInfo.State.MinDistLong) / OppInfo.State.CarDiffVelLong;
 
	  if (VCatTime > 0)
	    MinVCatTime = MIN(MinVCatTime, VCatTime);
	}

	bool IgnoreTeamMate =
	  OppInfo.GotFlags(F_TEAMMATE) 
	  && (CarLaps < OppCar->_laps 
	  || CarDamage + 1000 >= OppInfo.TeamMateDamage);

	OppInfo.AvoidLatchTime = MAX(0, OppInfo.AvoidLatchTime - oSituation->deltaTime);

	double MaxSpdCrv = Param.Fix.CalcMaxSpeedCrv();
	double ColTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 1.2;
	double CatTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 3.0;
	double CacTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 3.0;
	bool Catching =
	  OppInfo.CatchTime < ColTime && OppInfo.GotFlags(F_COLLIDE)  
	  || OppInfo.CatchTime < CatTime && OppInfo.GotFlags(F_CATCHING) 
	  || OppInfo.CatchAccTime < CacTime && OppInfo.GotFlags(F_CATCHING_ACC);

	if (!IgnoreTeamMate &&
	  (OppInfo.AvoidLatchTime > 0 || Catching || OppInfo.GotFlags(F_DANGEROUS)))
	{
	  double ToL, ToR;

	  GetPathToLeftAndRight(OppCar, ToL, ToR);
	  ToL += OppInfo.State.TrackVelLat * OppInfo.CatchTime;
	  ToR -= OppInfo.State.TrackVelLat * OppInfo.CatchTime;
	  bool SpaceL = ToL > OppInfo.State.MinDistLat + 0.75;// + 0.25;
	  bool SpaceR = ToR > OppInfo.State.MinDistLat + 0.75;// + 0.25;
	  bool AvoidL = OppInfo.State.CarDistLat < 0 && SpaceR;
	  bool AvoidR = OppInfo.State.CarDistLat > 0 && SpaceL;

	  if (Catching)
	    OppInfo.AvoidLatchTime = fabs(Crv) < MaxSpdCrv ? 0.5 : 0.1;

	  if (fabs(Crv) < MaxSpdCrv)
	  {
	    if (!AvoidL && !AvoidR)
		{
		  AvoidL = !SpaceL && SpaceR;
		  AvoidR = !SpaceR && SpaceL;
		}
	  }

  	  if (AvoidL)
		Coll.OppsAhead |= F_LEFT;

	  if (AvoidR)
		Coll.OppsAhead |= F_RIGHT;

	  if (AvoidL)
		Coll.MinLDist = MIN(OppInfo.State.CarAvgVelLong, Coll.MinLDist);

  	  if (AvoidR)
		Coll.MinRDist = MIN(OppInfo.State.CarAvgVelLong, Coll.MinRDist);
	}
  }

  if (OppInfo.GotFlags(F_AT_SIDE))               // Is Opponent at side of us
  {
    if (DistanceRaced > 100.0
      && OppInfo.GotFlags(F_FRONT))   
      oNoAvoidLength = 0;
	Coll.OppsAtSide |= OppInfo.State.CarDistLat < 0 ? F_LEFT : F_RIGHT;
	if (OppInfo.State.CarDistLat < 0)
	  Coll.MinLSideDist = MIN(Coll.MinLSideDist, 
	    -OppInfo.State.CarDistLat - OppInfo.State.MinDistLat);
	else
	  Coll.MinRSideDist = MIN(Coll.MinRSideDist,  
	    OppInfo.State.CarDistLat - OppInfo.State.MinDistLat);
  }

  oTreatTeamMateAsLapper =
	OppInfo.GotFlags(F_TEAMMATE | F_REAR) 
	&& OppInfo.State.RelPos > -25 
	&& CarLaps == OppCar->_laps 
	&& CarDamage > OppInfo.TeamMateDamage + 1000;

  if (oStayTogether > 50 
	&& OppInfo.GotFlags(F_TEAMMATE | F_REAR) 
	&& OppInfo.State.RelPos < -35 
	&& OppInfo.State.RelPos > -oStayTogether 
	&& CarDamage + 1000 > OppInfo.TeamMateDamage)
  {
	IsLapper = true;
  }

  if (OppInfo.GotFlags(F_LAPPER) || oTreatTeamMateAsLapper)
  {
	Coll.LappersBehind |= OppInfo.State.CarDistLat < 0 ? F_LEFT : F_RIGHT;
	IsLapper = true;
  }
}
//==========================================================================*

//==========================================================================*
// Get next curvature to opponent
//--------------------------------------------------------------------------*
void TDriver::NextCurvature(TCollision::TCollInfo& Coll, PtCarElt Car)
{
  int OppPosIndex = oTrackDesc.IndexFromPos(oTrackDesc.CalcPos(Car));
  Coll.NextSide = 
	(oRacingLine[oRL_FREE].PathPoints(OppPosIndex).Crv < 0) ? -1 : 0;
}
//==========================================================================*

//==========================================================================*
// Target Reached
//--------------------------------------------------------------------------*
bool TDriver::TargetReached(double Target, double AvoidTarget)
{
  if((oAvoidRange != 0.0)
	&& (Target == 0)
	|| (AvoidTarget != oAvoidRange)
	&& (Target != 0))
    return false;
  else
    return true;
}
//==========================================================================*

//==========================================================================*
// Runaround most urgent obstacle
//--------------------------------------------------------------------------*
void TDriver::Runaround(double Scale, double Target, bool DoAvoid)
{
  // Scale limits of change of lateral movement (accellerations/velocities)
  double RangeAccMax = 0.00075 * Scale;          // Range accelleration and
  double RangeVelMax = 0.005 * Scale;            // velocity per sim.step
  double OffsetAccMax = 0.0003 * Scale;          // Offset accelleration and
  double OffsetVelMax = 0.2 * Scale;             // velocity per sim.step

  double AvoidTarget = 0;                        // Assume come back to RL
  if (DoAvoid)                                   // But if needed
    AvoidTarget = 2.0;                           //   avoid to side

  // Adjusting allowed range of offset ...
  if (!TargetReached(Target,AvoidTarget))        // Target Range reached?
  {
	AvoidTarget = (Target == 0) ? 0 : 1;         // Direction to move to
    double LatAccel = oAvoidRange > AvoidTarget  // Xceleration of changing
      ? RangeAccMax : -RangeAccMax;              // the range
    double Dist =                                // Remaining distance to
	  oAvoidRange - AvoidTarget;                 //   target range

	if (fabs(Dist) < 0.0005)                     // If close to target
	{
      oAvoidRangeDelta = 0.0;                    //   don't change any longer
	}
	else
	{
	  if (fabs(Dist) <=                          // Check wether to decellerate
		XX2Y(oAvoidRangeDelta,RangeAccMax))      // enlargement/contracting
	  {
		LatAccel = -XX2Y(oAvoidRangeDelta,Dist); // Decelerate
	  }
	  oAvoidRangeDelta += LatAccel;              // Accellerate or Decellerate
      oAvoidRangeDelta =                         // Restrict to limits
	    MINMAX(RangeVelMax,oAvoidRangeDelta);
	}
  }
  else                                           // If target is reached
	oAvoidRangeDelta = 0;                        //   stop changing

  double OldAvoidRange = oAvoidRange;            // Save old range
  oAvoidRange -= oAvoidRangeDelta;               // Set new range

  if ((oAvoidRange > 0.9995)                     // If close to Max
	&& (oAvoidRangeDelta < 0))                   // fix range and change
  {
    oAvoidRange = 1.0;                           // Max reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }
  else if((oAvoidRange <= 0.0005)                // If close to Min
	&& (oAvoidRangeDelta > 0))                   // fix range and change
  {
	oAvoidRange = 0.0;                           // Min reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }
  else if (FixRange)                             // If close to tagret
  {
	oAvoidRange = AvoidTarget;                   // Target reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }

  // Adjusting offset ...
  if (Target != oAvoidOffset)                    // Target Offset reached?
  {
 	double LatAccel =                            // Xcelleration of lateral movement
	  OffsetAccMax / MAX(0.2, oAvoidRange);
	LatAccel = Target > oAvoidOffset             // Accelleate or Decellerate?
	  ? LatAccel : -LatAccel;
	double Dist = Target - oAvoidOffset;         // Distance to target offset
	if ((Dist * oAvoidOffsetDelta > 0)           // Check wether to decellerate
      && (fabs(Dist) <=                          //   lateral movement
	    XX2Y(oAvoidOffsetDelta,OffsetAccMax)))
	{
	  LatAccel = -XX2Y(oAvoidOffsetDelta,Dist);  // Decellerate
	}
	LatAccel = MINMAX(OffsetAccMax,LatAccel);    // Restrict to limits
	oAvoidOffsetDelta += LatAccel;               // Accellerate or decellerate
    oAvoidOffsetDelta =                          // Restrict to limits
	    MINMAX(OffsetVelMax,oAvoidOffsetDelta);
  }
  else                                           // If target offset reached
    oAvoidOffsetDelta = 0;                       //   stop changing

  double OldAvoidOffset = oAvoidOffset;          // Save old offset
    oAvoidOffset += oAvoidOffsetDelta;           // Set new offset

  if ((oAvoidOffset < -0.99)                     // If close to Min
	&& (Target < 0))                             // of target dir
  {
	oAvoidOffset = -1;                           // Min reached
	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
  else if ((oAvoidOffset > 0.99)                 // If close to Max
	&& (Target > 0))                             // of target dir
  {
	oAvoidOffset = 1;                            // Max reached
	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
  else if (FixOffset)                            // If close to target
  {
	oAvoidOffset = Target;                       // Target reached
 	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
}
//==========================================================================*

//==========================================================================*
// General avoidance controll
//--------------------------------------------------------------------------*
void TDriver::AvoidOtherCars(double K, bool& IsClose, bool& IsLapper)
{
  const TOpponent::TState& MyState =             // Get my own state
	oOpponents[oOwnOppIdx].Info().State;

  int I;
  for (I = 0; I < oNbrCars; I++)                 // Get info about imminent
  {                                              //   collisions from
      oOpponents[I].Classify(                    //   all opponents depending
	  oCar,                                      //   on TORCS data of own car
	  MyState,                                   //   my own state,
	  oStrategy->OutOfPitlane(),                 //   In pitlane?
	  oMaxAccel.Estimate(CarSpeedLong));         //   Estimate of accelleration
  }

  // Place to subsume the collision flags from all opponents
  TCollision::TCollInfo Coll;

  double MinCatchTime = FLT_MAX;                 // Initialize limits
  double MinCatchAccTime = FLT_MAX;
  double MinVCatTime = FLT_MAX;

  IsLapper = false;                              // Initialize flags

  TLanePoint PointInfo;                          // Infos to point 
  GetLanePoint                                   //   own position
	(oRL_FREE,DistanceFromStartLine,PointInfo);

  for (I = 0; I < oNbrCars; I++)                 // Loop all opponents
  {
    EvaluateCollisionFlags(                      // Evaluate collision flags
      I, Coll, K,
	  MinCatchTime,
	  MinCatchAccTime,
	  MinVCatTime,
	  IsLapper);
  }

  if (((Coll.Flags & F_PREVIEWSLOW) != 0)
    && (DistanceRaced > 1000))
  {
    Coll.AvoidSide = 0;            
	int I;
	int K = 0;
	if (Coll.Blocked[0] || Coll.Blocked[1] || Coll.Blocked[2])
	{
      for (I = MAXBLOCKED - 1; I > 1; I--) 
      {
		K++;
	    if (Coll.Blocked[I] || Coll.Blocked[I-1] || Coll.Blocked[I-2])
		  continue;
	    else
	    {
	      Coll.AvoidSide = ((I-1) * 2.0/(MAXBLOCKED - 1.0) - 1);
	      break;
	    }
  	  }
	}
	else
	{
      for (I = 0; I < MAXBLOCKED - 2; I++) 
      {
		K++;
	    if (Coll.Blocked[I] || Coll.Blocked[I+1] || Coll.Blocked[I+2])
		  continue;
	    else
	    {
	      Coll.AvoidSide = ((I+1) * 2.0/(MAXBLOCKED - 1.0) - 1);
	      break;
	    }
  	  }
	}

	if (K == MAXBLOCKED - 2)
	{
      oSpeedScale += 0.00025; 
      Coll.TargetSpeed = MIN(Coll.TargetSpeed, MAX(0.8,(1 - oSpeedScale)) * oTargetSpeed);
	}
  }
  
  if ((Coll.Flags & F_PREVIEWSLOW) == 0)
    oSpeedScale = 0.0;

  NextCurvature(Coll,oCar);                      // Find side of next curvature

  oDoAvoid = false;                              // Assume not avoiding

  TCollision RunAround;                          // To runaround we have to decide
  double Target = 0.0;                           // which way to take
  float Ratio = 0.0;

  if (DistanceRaced > oNoAvoidLength)
    Target = RunAround.AvoidTo                   // Check which way we should take
      (Coll,oCar,*this,oDoAvoid);                //   depending on opponents
  else
  {
    Target = oStartSide * -1 * oIndex;           // side to go to
	oDoAvoid = true;                             // go to
  }

  if (oStrategy->StartPitEntry(Ratio))           // If entrering pit
  {
	if (!oDoAvoid)                               // If no avoiding needed
	{
      Target = Ratio * PitSide();                // Bring us to the correct
	  oDoAvoid = true;                           // side to make pit stop
	}
  } 
  else if (oStrategy->StopPitEntry(Param.Pit.oExitLength)) // If coming back to track
  {
	if (!oDoAvoid)                                // If no avoiding needed
	{
      Target = PitSide();                        // Bring us to the correct
	  oDoAvoid = true;                           // side to make pit stop
	}
  } 

  oTargetSpeed =                                 // Adjust target speed
	MIN(oTargetSpeed, Coll.TargetSpeed);

  IsClose = (Coll.Flags & F_CLOSE) != 0;         // Set flag, if opponent is close by

  if (oFlying)                                   // Sorry, but we can't do anything
    return;                                      //   right now, we are flying!

  double HalfWidth = oTrackDesc.Width() / 2;     // Half width of track
  double Scale =                                 // Scale reaction
	oAvoidScale /(oAvoidWidth + HalfWidth);

  Runaround(Scale,Target,oDoAvoid);              // runaround most urgent obstacle
}
//==========================================================================*

//==========================================================================*
// ABS-r 
//--------------------------------------------------------------------------*
double TDriver::FilterABS(double Brake)
{
  if(CarSpeedLong < 10)
	return Brake;

  double Slip = 0.0;

  for (int I = 0; I < 4; I++)
	Slip += WheelSpinVel(I) * WheelRad(I);

  Slip = 4.0 * CarSpeedLong / Slip;

  if (Slip > ABS_DELTA)
  {
	Brake *= ABS_SCALE;
  }

  return Brake;
}
//==========================================================================*

//==========================================================================*
// Filter Brake while drifting
//--------------------------------------------------------------------------*
double TDriver::FilterBrake(double Brake)
{
  // If braking, decrease braking force while drifting
  if((CarSpeedLong > SLOWSPEED) && (Brake > 0.0))
  {
    double DriftAngle = MAX(MIN(oDriftAngle * 2, PI),-PI);
    Brake *= MAX(0.1, cos(DriftAngle));
  }
  return Brake;
}
//==========================================================================*

//==========================================================================*
// Filter Traction Control
//--------------------------------------------------------------------------*
double TDriver::FilterTCL(double Accel)
{
  if(fabs(CarSpeedLong) < 0.001)                 // Only if driving faster
	return Accel;

  double Spin = 0;                               // Initialize spin
  double Wr = 0;                                 // wheel radius
  int Count = 0;                                 // count impellers

  if(HasDriveTrainFront)                         // If front wheels     
  {                                              //   are impellers
	Spin += WheelSpinVel(FRNT_LFT);              // Summarize spin
	Spin += WheelSpinVel(FRNT_RGT);              // of both wheels
	Wr += WheelRad(FRNT_LFT)+WheelRad(FRNT_RGT); // measure radius
	Count += 2;                                  // and count both
  }

  if(HasDriveTrainRear)                          // If rear wheels     
  {                                              //   are impellers
	Spin += WheelSpinVel(REAR_LFT);              // Summarize spin
	Spin += WheelSpinVel(REAR_RGT);              // of both wheels
	Wr += WheelRad(REAR_LFT)+WheelRad(REAR_RGT); // measure radius
	Count += 2;                                  // and count both
  }
  Spin /= Count;                                 // Calculate spin
  Wr /= Count;                                   // and radius

  double Slip = Spin * Wr - CarSpeedLong;        // Calculate slip
  if (Slip > oTclSlip)                           // Decrease accel if needed
	Accel -= MIN(Accel, (Slip - oTclSlip)/oTclRange);

  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Let Pass
//--------------------------------------------------------------------------*
double TDriver::FilterLetPass(double Accel)
{
  // If we should let an other bot pass, decrease accelleration
  if (oLetPass)
  {
    if (oTreatTeamMateAsLapper)
      Accel = MIN(Accel, 0.7);
	else
      Accel = MIN(Accel, 0.5);
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Drifting
//--------------------------------------------------------------------------*
double TDriver::FilterDrifting(double Accel)
{
  // Decrease accelleration while drifting
  if((CarSpeedLong > SLOWSPEED) && (fabs(oDriftAngle) > 0.2))
  {
	Accel *= (float) 
	  (0.25 + 0.75 * MAX(0.0,cos(oDriftAngle)));
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Track
//--------------------------------------------------------------------------*
double TDriver::FilterTrack(double Accel)
{
  if (DistanceRaced > oStartDistance)            // Except while starting
  {
    //if (Qualification) 
    //  return Accel;

	if (fabs(oDeltaOffset) > oTolerance)         // Check offset difference
	  Accel *= (float)                           //   Decrease acceleration
	    (MAX(1.0 - (fabs(oDeltaOffset) - oTolerance) * 0.2, 0.4));
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Detect obstacles
//--------------------------------------------------------------------------*
bool TDriver::IsStuck()
{
  TV2D Tmp;                                      // Holds Coordinates 
  float Diff;                                    // Distance from old point

  if ((oStuckCounter > 3) && (oStuckCounter < 6))// Less then six ticks 
	oCar->_brakeCmd = 1.0;                       //   left? stop driving back
  else
	oCar->_brakeCmd = 0.0;                       //

  if (oStuckCounter > 0)                         // Driving back?
  {                                              //   If so, clear buffer
    oSysFooStuckX->Reset();
    oSysFooStuckY->Reset();
    oStuckCounter--;                             //   decrement counter
    //GfOut("Driving back! %d\n",oStuckCounter);
    return true;                                 //   and drive  
  }

  TV2D MyPos;                                    // Current position
  MyPos.x = CarPubGlobPosX;                      //
  MyPos.y = CarPubGlobPosY;                      //

  // Check motion
  Tmp.x = oSysFooStuckX->Faltung(float(MyPos.x));// X coordinate
  Tmp.y = oSysFooStuckY->Faltung(float(MyPos.y));// Y coordinate

  Diff = Dist(Tmp,MyPos);                        // Distance from old pos
  //GfOut("Dif: %g\n",Diff);
  if (Diff < 0.3)                                // If distance is to small
//  if (Diff < 5.0)                                // If distance is to small
  {                                              //   assume obstacle
	if (oStuckCounter == 0)
	{
      oStuckCounter = -UNSTUCK_COUNTER;          // Set counter
	  //GfOut("Set! %d\n",oStuckCounter);
	}

	if (oStanding)                               // But if flag is set
	{                                            //   it is planned!
	  //GfOut("Standing! %d\n",oStuckCounter);
	  oSysFooStuckX->Reset();                    // Clear buffers
	  oSysFooStuckY->Reset();                    //   of motion survey
      return false;                              //   and signal ok
	}
    else if (oUnstucking)                        // But if flag is set
	{                                            //   it is possible
      if (oStuckCounter < 0)                     // 
	  {
        oSysFooStuckX->Reset();
        oSysFooStuckY->Reset();
        oStuckCounter++;                         // Increment counter
		if (oStuckCounter == 0)
	    {
          oStuckCounter = UNSTUCK_COUNTER;       // Set counter
  	      //GfOut("Stuck1! %d\n",oStuckCounter);
          return true;                           // give signal stuck
	    }
	    //GfOut("Unstucking! %d\n",oStuckCounter);
        return false;                            //   and signal ok
	  }
	  else                                       // still stuck
	  {
        oStuckCounter = UNSTUCK_COUNTER;         // Set counter
  	    //GfOut("Stuck1! %d\n",oStuckCounter);
        return true;                             // give signal stuck
	  }
	}
    else                                         // if not
	{
      oStuckCounter = UNSTUCK_COUNTER;           // Set counter
	  //GfOut("Stuck! %d\n",oStuckCounter);
      return true;                               // give signal stuck
	}
  }                                              
  else
    oStanding = false;                           // Reset flag

  return false;                                  // No obstacle
}
//==========================================================================*

//==========================================================================*
// Unstuck
//--------------------------------------------------------------------------*
void TDriver::Unstuck()
{
  TLanePoint AheadPointInfo;
  oAngle = UnstuckSteerAngle(oLanePoint,AheadPointInfo) / 100.0;

  CarSteerCmd = (float) -(oAngle/CarSteerLock);  // Steering
  CarGearCmd = -1;                               // Reverse gear
  CarBrakeCmd = 0.0;                             // Unlock brake
  CarAccelCmd = 1.0;                             // Open the throttle
  CarClutchCmd = 0.0;                            // Release clutch
  oUnstucking = true;                            // Set flag
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitdriver.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
