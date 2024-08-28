//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitmain.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0/1.3.1
// Interface zu TORCS
//
// Datei    : unitmain.cpp
// Erstellt : 27.01.2008
// Stand    : 22.11.2008
// Copyright: � 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.000
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
#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"
#define NUMSEGS 16

#include "sensors.h"
Sensors *trackSens = NULL;

#define __SPEED_LIMIT__ -1
//==========================================================================*
// Konstanten, Typen und Variablen, die au�erhalb dieser Unit sichtbar sind
//--------------------------------------------------------------------------*

// Namen der Roboter und
// Beschreibung der Roboter
#if defined(mysimplix_EXPORTS)  
static char* BotName[NBBOTS] = {
  "car1-trb1_0", 
  "car1-trb1_1"
};
static char* BotDesc[NBBOTS] = {
  "car1-trb1_0", 
  "car1-trb1_1"
};
#else
static char* BotName[NBBOTS] = {
  "car1-trb1_0", 
  "car1-trb1_1"
};
static char* BotDesc[NBBOTS] = {
  "car1-trb1_0", 
  "car1-trb1_1"
};
#endif

//  Roboter des Moduls
static TDriver *cRobot[NBBOTS];
static TCommonData gCommonData;
static double cTicks;
static double cMinTicks;
static double cMaxTicks;
static int cTickCount;
static int cLongSteps;
static int cCriticalSteps;
static int cUnusedCount;

//Logger
static tdble values[50];
char trackname[50];
int logtimer=0;
//char drivername[MAXNAMELEN];
//char trackname[256];
//struct tm *tpointer = NULL;
float fwsegs[NUMSEGS];
float bwsegs[NUMSEGS];
void calcSegs(tCarElt* car,float* segs);
void calcSegsBack(tCarElt* car,float* segs);

//==========================================================================*

//==========================================================================*
// Puffer
// Buffers
//--------------------------------------------------------------------------*
#define BIGBUFLEN 256
static char FilenameBuffer[BIGBUFLEN];           // for path and filename
#define BUFLEN 20
static char Drivers[BUFLEN * NBBOTS];            // Driver names
static bool Footprint = false;                   // Never called yet
static int InitDriver = -1;                      // None initialized yet
//==========================================================================*

//==========================================================================*
// Prototypen der Routinen(Funktionen/Prozeduren), die wir f�r die
// Kommunikation mit TORCS bereitstellen
//--------------------------------------------------------------------------*
static void InitTrack
  (int index,
  tTrack* track,
  void *carHandle,
  void **carParmHandle,
  tSituation *s);
static void NewRace
  (int index,
  tCarElt* car,
  tSituation *s);
static void Drive
  (int index,
  tCarElt* car,
  tSituation *s);
static int PitCmd
  (int index,
  tCarElt* car,
  tSituation *s);
static void Shutdown
  (int index);
static int InitFuncPt
  (int index,
  void *pt);
static void EndRace
  (int index,
  tCarElt *car,
  tSituation *s);
static void Shutdown
  (int index);
//==========================================================================*
//==========================================================================*
// Logger
//--------------------------------------------------------------------------*
void printCA(tCarElt* car){
	const char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	float rearwingarea = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*) NULL, 0.0f);
	float rearwingangle = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*) NULL, 0.0f);
	float wingca = 1.23f*rearwingarea*sin(rearwingangle);

	float cl = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*) NULL, 0.0f) +
			   GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*) NULL, 0.0f);
	float h = 0.0f;
	int i;
	for (i = 0; i < 4; i++)
		h += GfParmGetNum(car->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*) NULL, 0.20f);
	h*= 1.5f; h = h*h; h = h*h; h = 2.0f * exp(-3.0f*h);
	float CA = h*cl + 4.0f*wingca;
	printf("CA: %f\n",CA);
}
float getDistToSegStart(tCarElt* car)
{
    if (car->_trkPos.seg->type == TR_STR) {
        return car->_trkPos.toStart;
    } else {
        return (car->_trkPos.toStart)*car->_trkPos.seg->radius;
    }
}
void getPoint(tCarElt* car,float tomiddle,float* x,float* y)
{
    tTrackSeg *seg = car->_trkPos.seg;
    float length = getDistToSegStart(car);

    tomiddle=(tomiddle + 1)/2;
    float sx = (seg->vertex[TR_SL].x*tomiddle + seg->vertex[TR_SR].x*(1-tomiddle));
    float sy = (seg->vertex[TR_SL].y*tomiddle + seg->vertex[TR_SR].y*(1-tomiddle));
    if ( seg->type == TR_STR) {
        float dx = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x)/seg->length;
        float dy = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y)/seg->length;
        //return s + d*length;
        *x= sx + dx*length;
        *y= sy + dy*length;
    } else {
        float cx = seg->center.x;
        float cy = seg->center.y;
        float arc = length/seg->radius;
        //float arc = (tomiddle==1) ? length/seg->radiusl : length/seg->radiusr;
        float arcsign = (seg->type == TR_RGT) ? -1 : 1;
        arc = arc*arcsign;
        //return s.rotate(c, arc);
        float dx = sx -cx;
        float dy = sy -cy;
        float sina = sin(arc), cosa = cos(arc);
        *x = cx + dx*cosa-dy*sina;
        *y = cy + dx*sina+dy*cosa;
        
    }
}

static void logger(tCarElt* car){
    	
	float dist_to_middle = 2*car->_trkPos.toMiddle/(car->_trkPos.seg->width);// computing distance to middle
	float angle =  RtTrackSideTgAngleL(&(car->_trkPos)) - car->_yaw; // computing the car angle wrt the track axis
	NORM_PI_PI(angle); // normalize the angle between -PI and + PI

	// update the value of track sensors only as long as the car is inside the track
	if (dist_to_middle<=1.0 && dist_to_middle >=-1.0 ){
		trackSens->sensors_update();
		for (int i = 0; i < 19; ++i)
		    values[i] = trackSens->getSensorOut(i);
	}
	else{
		for (int i = 0; i < 19; ++i)
		    values[i] = -1;
	}
	
	tTrackSeg* seg = car->_trkPos.seg;
	float mu= seg->surface->kFriction;

    	values[19]= car->_gearCmd;
	values[20]= dist_to_middle;
	values[21]= angle;
	values[22]= seg->type;
	values[23]= seg->radius;
	values[24]= car->_speed_x*car->_speed_x / (2.0 * mu * 9.81); //BrakeDistance
	values[25]= sqrt(mu * 9.81 * seg->radius); //allowed speed
	values[26]= car->_laps;
	
	values[27]= car->_pos_X;
	values[28]= car->_pos_Y;
	getPoint(car,-1,&values[29],&values[30]);
	getPoint(car,1,&values[31],&values[32]);
	
	values[33]=mu;
	
	calcSegs(car,fwsegs);
	calcSegsBack(car,bwsegs);

	if (logtimer==0){
		
		printf("Logging Started...\n");
		RtTelemInit(-10, 10);
		RtTelemNewChannel("Lap", &values[26], 0, 0); //Current Lap
		RtTelemNewChannel("Ax", &car->_accel_x, 0, 0); //Accelerations
		RtTelemNewChannel("Ay", &car->_accel_y, 0, 0);
		RtTelemNewChannel("Steer", &car->_steerCmd, 0, 0); //Car Commands
		RtTelemNewChannel("Accel", &car->_accelCmd, 0, 0);
		RtTelemNewChannel("Brake",  &car->_brakeCmd, 0, 0);
		RtTelemNewChannel("Gear", &values[19], 0, 0);
		RtTelemNewChannel("Speed", &car->_speed_x, 0, 0); //Speed
		RtTelemNewChannel("FromStart", &car->_distFromStartLine, 0, 0);//Dist
		RtTelemNewChannel("RPM", &car->_enginerpm, 0, 0);//RPM
		RtTelemNewChannel("ToMiddle", &values[20], 0, 0); //ToMiddle
		RtTelemNewChannel("Angle", &values[21], 0, 0); //Angle
		RtTelemNewChannel("SegType", &values[22], 0, 0); //SegType
		RtTelemNewChannel("SegRadius", &values[23], 0, 0); //SegRadius
		RtTelemNewChannel("BrakeDistance", &values[24], 0, 0); //BrakeDistance
		RtTelemNewChannel("MaxSpeed", &values[25], 0, 0); //MaxSpeed
		
		RtTelemNewChannel("PosX", &values[27], 0, 0);
		RtTelemNewChannel("PosY", &values[28], 0, 0);
		RtTelemNewChannel("RightX", &values[29], 0, 0);
		RtTelemNewChannel("RightY", &values[30], 0, 0);
		RtTelemNewChannel("LeftX", &values[31], 0, 0);
		RtTelemNewChannel("LeftY", &values[32], 0, 0);
		
		RtTelemNewChannel("Friction", &values[33], 0, 0);
		
		
		for (int i = 0; i < 19; ++i){
		    	RtTelemNewChannel("TrackSens", &values[i], 0, 0);
		}
		
		for (int i=0;i<NUMSEGS;i++){
			RtTelemNewChannel("Radius-fw", &fwsegs[i], 0, 0);
		}
		
		for (int i=0;i<NUMSEGS;i++){
			RtTelemNewChannel("Radius-bw", &bwsegs[i], 0, 0);
		}
		
		printCA(car);
		
		char bufid[50];
		//sprintf(bufid,"log-%s-%s-%d:%d",drivername,trackname,tpointer -> tm_hour , tpointer -> tm_min);
		sprintf(bufid,"%s.simplix-log",trackname);
		printf("file: %s.dat\n",bufid);
		RtTelemStartMonitoring(bufid);
	}
	if (car->_laps >0 && car->_remainingLaps > 0)
	{
		RtTelemUpdate(car->_curLapTime);
		//if (logtimer%25==0) printf("%f %f %f |#| %f %f %f\n",bwsegs[2],bwsegs[1],bwsegs[0],fwsegs[0],fwsegs[1],fwsegs[2]);
	}

	if (car->_remainingLaps == 0)
	{
	  RtTelemShutdown();
	  printf("Logging Finished at lap %d!\n", car->_laps);
	}
	logtimer++;
}

//==========================================================================*
// Aufbereitung der Namen
// Prepare names
//--------------------------------------------------------------------------*
bool Prepare()
{
  InitDriver++;      // Count initialized drivers
  if (Footprint)     // Check wether we have done it before
  {
    return false;    //   If so, return false,
  }
  Footprint = true;  // else set flag

  // Initialize the base param path.
  char BaseParamPath[] = ROBOT_DIR;              // Depends on robots name
  char* PathFilename = FilenameBuffer;           // Pointer to buffer

  memset(&Drivers[0],0,BUFLEN * NBBOTS);         // Clear buffer

  snprintf(FilenameBuffer,BIGBUFLEN,             // Build path to
    "drivers/%s/%s.xml"                          // own robot dll from
	,MyBotName,MyBotName);                       // name of own robot

  void* RobotSettings = GfParmReadFile           // Open team setup file
    (PathFilename,GFPARM_RMODE_STD);

  if (RobotSettings)                             // If file opened
  {
    char SectionBuffer[256];                     // Buffer for section name
    char* Section = SectionBuffer;               // Adjust Pointer 

    for (int I = 0; I < NBBOTS; I++)             // Loop all drivers
    {
      snprintf(SectionBuffer,BUFLEN,             // Build name of
        "%s/%s/%d"                               // section from
	    ,ROB_SECT_ROBOTS,ROB_LIST_INDEX,I);      // Index of driver
 
	  char* DriverName = (char*) GfParmGetStr            // Get pointer to
        (RobotSettings                           // drivers name
        , Section                                // defined in corresponding
        , ROB_ATTR_NAME, BotName[I]);            // section, BotName[I] as default

	  snprintf(&Drivers[I*BUFLEN],BUFLEN-1,DriverName);
    }
  }
  else
  { // This should never happen! But give user a chance to read it!
	GfOut("\n\n\n FATAL ERROR: File '%s' not found\n\n",PathFilename);
	for (int I = 0; I < NBBOTS; I++)
      snprintf(&Drivers[I*BUFLEN],BUFLEN-1,BotName[I]);
  }
  return Footprint;
};
//==========================================================================*

//==========================================================================*
// Namen des Fahrers/Roboters holen
// Get name of driver/robot 
//--------------------------------------------------------------------------*
char* GetBotName(int Index)
{
  return &Drivers[Index*BUFLEN];
};
//==========================================================================*

//==========================================================================*
// Teilt TORCS mit, wer wir sind, wie wir angesprochen werden wollen und
// was wir k�nnen.
//--------------------------------------------------------------------------*
#if defined(mysimplix_EXPORTS)  
extern "C" int mysimplix(tModInfo *ModInfo)
#else
extern "C" int simplix_V1_10(tModInfo *ModInfo)
#endif
{
  if (Prepare()) // Check Footprint and prepare names
  { // Run once only!
    // Clear memory provided 
    memset(ModInfo, 0, 10 * sizeof(tModInfo)); 
  }

  int I;

  for (I = 0; I < NBBOTS; I++) 
  {
    ModInfo[I].name    = GetBotName(I);          // Tell customisable name
    ModInfo[I].desc    = BotDesc[I];
    ModInfo[I].fctInit = InitFuncPt;
    ModInfo[I].gfId    = ROB_IDENT;
    ModInfo[I].index   = I;
  }
  return 0;
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Initialisierung
//
// Nach Kl�rung der generellen Ansprache (Aufruf dieser Fkt), teilen wir
// TORCS nun noch mit, mit welchen Funktionen wir die angeforderten
// Leistungen erbringen werden:
//
// Die geforderten Leistungen m�ssen erbracht werden ...
// RbNewTrack: ... wenn Torcs eine neue Rennstrecke bereitstellt
// RbNewRace:  ... wenn Torcs ein neues Rennen startet
// RbDrive:    ... wenn das Rennen gefahren wird
// RbPitCmd:   ... wenn wir einen Boxenstop machen
// RbEndRace:  ... wenn das Rennen ist beendet
// RbShutDown: ... wenn der ggf. angefallene Schrott beseitigt werden muss
//--------------------------------------------------------------------------*
static int InitFuncPt(int Index, void *Pt)
{
  tRobotItf *Itf = (tRobotItf *)Pt;

  Itf->rbNewTrack = InitTrack;
  Itf->rbNewRace  = NewRace;
  Itf->rbDrive    = Drive;
  Itf->rbPitCmd   = PitCmd;
  Itf->rbEndRace  = EndRace;
  Itf->rbShutdown = Shutdown;
  Itf->index      = Index;

  cRobot[Index] = new TDriver(Index);
  cRobot[Index]->SetBotName(GetBotName(Index));

  return 0;
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Neue Rennstrecke
//--------------------------------------------------------------------------*
static void InitTrack(int Index,
  tTrack* Track,void *CarHandle,void **CarParmHandle, tSituation *S)
{
  cRobot[Index]->SetCommonData(&gCommonData);
  cRobot[Index]->InitTrack(Track,CarHandle,CarParmHandle, S);
  //trackname = strrchr(Track->filename, '/') + 1;
   char *s1,*s2;
   s1 = strrchr(Track->filename, '/') + 1;
   s2 = strchr(s1, '.');
   strncpy(trackname, s1, s2-s1);
   trackname[s2-s1] = 0;
   printf("trackname=%s\n",trackname);
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Neues Rennen beginnt
//--------------------------------------------------------------------------*
static void NewRace(int Index, tCarElt* Car, tSituation *S)
{
  cTicks = 0.0;
  cMinTicks = FLT_MAX;
  cMaxTicks = 0.0;
  cTickCount = 0;
  cLongSteps = 0;
  cCriticalSteps = 0;
  cUnusedCount = 0;

	// Initialization of track sensors
	trackSens = new Sensors(Car, 19);
	trackSens->setSensor(0,-90,200);
	trackSens->setSensor(1,-80,200);
	trackSens->setSensor(2,-70,200);
	trackSens->setSensor(3,-60,200);
	trackSens->setSensor(4,-50,200);
	trackSens->setSensor(5,-40,200);
	trackSens->setSensor(6,-30,200);
	trackSens->setSensor(7,-20,200);
	trackSens->setSensor(8,-10,200);
	trackSens->setSensor(9,0,200);
	trackSens->setSensor(10,10,200);
	trackSens->setSensor(11,20,200);
	trackSens->setSensor(12,30,200);
	trackSens->setSensor(13,40,200);
	trackSens->setSensor(14,50,200);
	trackSens->setSensor(15,60,200);
	trackSens->setSensor(16,70,200);
	trackSens->setSensor(17,80,200);
	trackSens->setSensor(18,90,200);

  cRobot[Index]->NewRace(Car, S);
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Rennen fahren
//
// Achtung: Diese Prozedur wird sehr h�ufig und schnell nacheinander
// aufgerufen. Deshalb geben wir hier in der Regel keine Debug-Texte aus!
// Zur Fehlersuche kann das aber mal sinnvoll sein.
//--------------------------------------------------------------------------*
static void Drive(int Index, tCarElt* Car, tSituation *S)
{
    //GfOut(">>> TDriver::Drive\n");
    //memset(&car->ctrl, 0, sizeof(tCarCtrl));     // Reset alte Werte

	logger(Car);
	if (cRobot[Index]->CurrSimTime != S->currentTime)
	{
  	  clock_t StartTicks = clock();

	  cRobot[Index]->CurrSimTime = S->currentTime;
      cRobot[Index]->Update(Car,S);
      if (cRobot[Index]->IsStuck())
		cRobot[Index]->Unstuck();
	  else
	    cRobot[Index]->Drive();

	  clock_t StopTicks = clock();
      double Duration = 1000.0 * (StopTicks - StartTicks)/CLOCKS_PER_SEC;

	  if (cTickCount > 0)
	  {
	    if (Duration > 1.0)
          cLongSteps++;
	    if (Duration > 2.0)
          cCriticalSteps++;
	    if (cMinTicks > Duration)
	      cMinTicks = Duration;
	    if (cMaxTicks < Duration)
	      cMaxTicks = Duration;
	  }
	  cTickCount++;
  	  cTicks += Duration;
	}
	else
  	  cUnusedCount++;

	if (__SPEED_LIMIT__ > 0 && Car->_speed_x > __SPEED_LIMIT__)
		//Car->ctrl.accelCmd = (__SPEED_LIMIT__ - Car->_speed_x)/__SPEED_LIMIT__;
		Car->ctrl.accelCmd = 0.0;

    //GfOut("<<< TDriver::Drive\n");
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Boxenstop (Wagen steht in der Box!)
//--------------------------------------------------------------------------*
static int PitCmd(int Index, tCarElt* Car, tSituation *S)
{
  // Dummy: use parameters
  if ((Index < 0) || (Car == NULL) || (S == NULL))
    printf("PitCmd\n");
  return cRobot[Index]->PitCmd();
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Rennen ist beendet
//--------------------------------------------------------------------------*
static void EndRace(int Index, tCarElt *Car, tSituation *S)
{
  // Dummy: use parameters
  if ((Index < 0) || (Car == NULL) || (S == NULL))
    printf("EndRace\n");
  cRobot[Index]->EndRace();
}
//==========================================================================*

//==========================================================================*
// TOCRS-Callback: Aufr�umen
//--------------------------------------------------------------------------*
static void Shutdown(int Index)
{
  cRobot[Index]->Shutdown();
  delete cRobot[Index];


  GfOut("\n\nClock\n");
  GfOut("Total Time used: %g sec\n",cTicks/1000.0);
  //GfOut("Min   Time used: %g msec\n",cMinTicks);
  //GfOut("Max   Time used: %g msec\n",cMaxTicks);
  GfOut("Mean  Time used: %g msec\n",cTicks/cTickCount);
  //GfOut("Long Time Steps: %d\n",cLongSteps);
  //GfOut("Critical Steps : %d\n",cCriticalSteps);
  //GfOut("Unused Steps   : %d\n",cUnusedCount);
  
  GfOut("\n\n");
}
//==========================================================================*
void calcSegs(tCarElt* car,float* segs){
	float segl=10;
	float lastl=0;
	float lastrad=0;
	float tostart=car->_trkPos.toStart;
	int numseg=0;
	
	float lenght;
	float radius;
	
	tTrackSeg* seg = car->_trkPos.seg;
	
	while(numseg<NUMSEGS){
		radius=seg->radius;
		if (radius>300)
			radius = 300;
		if (seg->type==1)
			radius=300 - radius;
		if (seg->type==2)
			radius=radius - 300;
		if (seg->type==3){
			lenght=seg->length-tostart;
		}	
		else{
			lenght=(seg->arc - tostart)*seg->radius;
		}
		//if (logtimer%50==0) printf("#%d lenght: %f - radius: %f\n",seg->id,lenght,radius);
		do{
			if ((lastl + lenght)>= segl){
				float delta= lastl + lenght - segl;
				lastrad= lastrad* (lastl/segl) + radius * ((lenght-delta)/segl);
				segs[numseg]=lastrad;
				numseg++;
				lastrad=radius;
				if (delta < segl){
					lastl=delta;
					lenght=0;
					}
				else{
					lastl=0;
					lenght= delta; //consumo il segmento
					}
			}else{
				lastl= lastl + lenght;	
				lastrad= lastrad* ((lastl-lenght)/lastl) + radius * (lenght/lastl);
				lenght=0;
			}
		}while((lastl + lenght)>= segl && numseg<NUMSEGS);
		

		
		seg=seg->next;
		tostart=0;
	}

}

void calcSegsBack(tCarElt* car,float* segs){
	float segl=10;
	float lastl=0;
	float lastrad=0;
	float tostart=car->_trkPos.toStart;
	int numseg=0;
	
	float lenght;
	float radius;
	
	tTrackSeg* seg = car->_trkPos.seg;
	
	while(numseg<NUMSEGS){
		radius=seg->radius;
		if (radius>300)
			radius = 300;
		if (seg->type==1)
			radius=300 - radius;
		if (seg->type==2)
			radius=radius - 300;
		if (seg->type==3){
			if (tostart==0)
				lenght=seg->length;
			else
				lenght=tostart;
		}	
		else{
			if (tostart==0)
				lenght=(seg->arc)*seg->radius;
			else
				lenght=(tostart)*seg->radius;
		}
		//if (timer%50==0) printf("#%d lenght: %f - radius: %f\n",seg->id,lenght,radius);
		do{
			if ((lastl + lenght)>= segl){
				float delta= lastl + lenght - segl;
				lastrad= lastrad* (lastl/segl) + radius * ((lenght-delta)/segl);
				segs[numseg]=lastrad;
				numseg++;
				lastrad=radius;
				if (delta < segl){
					lastl=delta;
					lenght=0;
					}
				else{
					lastl=0;
					lenght= delta; //consumo il segmento
					}
			}else{
				lastl= lastl + lenght;	
				lastrad= lastrad* ((lastl-lenght)/lastl) + radius * (lenght/lastl);
				lenght=0;
			}
		}while((lastl + lenght)>= segl && numseg<NUMSEGS);

		
		seg=seg->prev;
		tostart=0;
	}

}
//--------------------------------------------------------------------------*
// end of file unitmain.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
