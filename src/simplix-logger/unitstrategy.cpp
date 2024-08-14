//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitstrategy.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Boxenstop-Strategie
// (C++-Portierung der Unit UnitStrategy.pas)
//
// Datei    : unitstrategy.cpp
// Erstellt : 20.02.2007
// Stand    : 18.07.2009
// Copyright: © 2007-2009 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.10.001
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter mouse_2006
//    Copyright: (C) 2006 Tim Foden
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
// Additional check for unreleased pit provided by John Isham
//
//--------------------------------------------------------------------------*
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitstrategy.h"

//==========================================================================*
// Konstanten
//--------------------------------------------------------------------------*
const float TSimpleStrategy::cMAX_FUEL_PER_METER = 0.0008f;
const int TSimpleStrategy::cPIT_DAMMAGE = 5000;
//==========================================================================*

//==========================================================================*
// Strategie erzeugen
//--------------------------------------------------------------------------*
TSimpleStrategy::TSimpleStrategy():
	oWasInPit(false),
    oFuelChecked(false),
    oFuelPerM(0.0f),
    oLastPitFuel(0.0f),
    oLastFuel(0.0f),
    oExpectedFuelPerM(0.0f),
    oRaceDistance(0.0f),
    oRemainingDistance(0.0f),
    oReserve(0.0f),
    oTrackLength(0.0f),
    oMaxFuel(100.0f),
    oMinLaps(3)
{
}
//==========================================================================*

//==========================================================================*
// Aufräumen
//--------------------------------------------------------------------------*
TSimpleStrategy::~TSimpleStrategy()
{
  if (oPit != NULL)
	delete oPit;
}
//==========================================================================*

//==========================================================================*
// Strategie initialisieren
//--------------------------------------------------------------------------*
void TSimpleStrategy::Init(TDriver *Driver)
{
  oDriver = Driver;
  oPit = new TPit(Driver);
}
//==========================================================================*

//==========================================================================*
// Ist die Box frei?
//--------------------------------------------------------------------------*
bool TSimpleStrategy::IsPitFree()
{
  if (CarPit != NULL)
  {
	TTeamManager::TTeam* Team = oDriver->GetTeam();

    // V1.10.001: Additional check provided by John Isham >>>
    if ((Team->PitState != CarDriverIndex) 
	  && (Team->PitState != PIT_IS_FREE))
    {
      // Find the car/teamMember that has the pit blocked 
      CarElt* CarInPit = Team->Cars[Team->PitState];

      //GfOut("%s %d: PIT BLOCKED BY CAR %d (%s)\n",
	  //  MyBotName,CarDriverIndex,Team->PitState, CarInPit->_name);

      // If it is out of the race, release the pit!
      // NO_SIMU can be returned if the car truly is pitting!! 
      if ((CarInPit->_state & RM_CAR_STATE_DNF) 
		|| (CarInPit->_dammage >= 10000))
      {
        {
          GfOut("%s %d: Releasing pit blocked by %s (out of race)\n",
            MyBotName,CarDriverIndex,CarInPit->_name);
          Team->PitState = PIT_IS_FREE;
          CarPit->pitCarIndex = TR_PIT_STATE_FREE;
        }
      }
	}
    // <<< Additional check provided by John Isham

	if ((CarPit->pitCarIndex == TR_PIT_STATE_FREE)
      && ((Team->PitState == CarDriverIndex) || (Team->PitState == PIT_IS_FREE)))
      return true;
  }
  return false;
}
//==========================================================================*

//==========================================================================*
// Entscheiden, ob Boxenstopp nötig ist
//--------------------------------------------------------------------------*
bool TSimpleStrategy::NeedPitStop()
{
  double FuelConsum;                             // Spritverbrauch (pro m)
  double FuelNeeded;                             // Für nächste Runde nötig

  bool Result = false;                           // Annahme: Kein Boxenstopp

  if (CarPit == NULL)                            // Ist eine Box vorhanden?
    return Result;                               //   Wenn nicht, Pech!

  if (!IsPitFree())                              // Ist Box frei?
    return Result;                               //   Wenn nicht, Pech!

  oRemainingDistance =                           // Restliche Strecke des
    oRaceDistance - DistanceRaced;               //   Rennens ermitteln

  oRemainingDistance -=                          // Verkürzt um Rückstand
	oTrackLength * CarLapsBehindLeader;          //   auf den Führenden

  if (oRemainingDistance > oTrackLength + 100)   // Wenn noch mehr km
  {                                              //   zu fahren sind
    if (oFuelPerM == 0.0)                        // Spritverbrauch pro m
      FuelConsum = oExpectedFuelPerM;            //   schätzen
    else                                         // oder den gemessenen
      FuelConsum = oFuelPerM;                    //   Wert nehmen

    FuelNeeded =                                 // Bedarf an Treibstoff
      MIN(oTrackLength+oReserve,                 // Bis z. n. Boxenstopp oder
	  oRemainingDistance+oReserve) * FuelConsum; // bis zum Ende des Rennens

    if (CarFuel < FuelNeeded)                    // Wenn der Tankinhalt nicht
	{
      Result = true;                             //   reicht, tanken
      GfOut("Pitstop by fuel\n");
	}
	else                                         // Ansonsten prüfen, für
	{                                            //   welche Anzahl von Runden
	  TTeamManager::TTeam* Team =                //   alle Teammitglieder
		oDriver->GetTeam();                      //   noch Treibstoff haben
	                                            
      FuelNeeded = FuelConsum * oTrackLength;    // Treibstoff für eine Runde 
	  int FuelForLaps =                          // Eigene Reichweite
        Team->FuelForLaps[CarDriverIndex] =      
	    (int) (CarFuel / FuelNeeded - 1);        
	  int MinLaps = Team->GetMinLaps(oCar);      // Mindestreichweite der anderen

	  // Wenn Tanken, dann der, der weniger Runden weit kommen würde
      if (FuelForLaps < MinLaps) 
      {                                      
        if ((MinLaps < oMinLaps)
		&& (RemainingLaps > FuelForLaps))
	    {                                        // Nur Tanken, wenn nötig!
          FuelNeeded = (oRemainingDistance + oReserve) * FuelConsum;
          if (CarFuel < FuelNeeded)              // Wenn der Tankinhalt nicht
	      {                                      // reicht, tanken
            Result = true;
            GfOut("Pitstop by Teammate\n");
	      }
        }
	  }
	}

    if (RepairWanted(cPIT_DAMMAGE) > 0)          // Wenn die Schäden zu hoch
	{                                            //   reparieren lassen
       Result = true;
	}
  };

  if (Result)
  {
	TTeamManager::TTeam* Team = oDriver->GetTeam();
	Team->PitState = CarDriverIndex;             // Box reserviert
  }
  return Result;
};
//==========================================================================*

//==========================================================================*
// Box freigeben
//--------------------------------------------------------------------------*
void TAbstractStrategy::PitRelease()
{
  TTeamManager::TTeam* Team = oDriver->GetTeam();
  if (Team->PitState == CarDriverIndex)          // Box für mich reserviert?
  {
    Team->PitState = PIT_IS_FREE;
    oCar->ctrl.raceCmd = 0;
  }
};
//==========================================================================*

//==========================================================================*
// Tanken
//--------------------------------------------------------------------------*
float TSimpleStrategy::PitRefuel()
{
  float FuelConsum;                              // Spritverbrauch kg/m
  float Fuel;                                    // Menge in kg

  if (oFuelPerM == 0.0)                          // Wenn kein Messwert
    FuelConsum = oExpectedFuelPerM;              //   vorliegt, schätzen
  else                                           // ansonsten
    FuelConsum = oFuelPerM;                      //   Messwert nehmen

  FuelConsum *= 1.10f;                           // ggf. ohne Windschatten!

  oRemainingDistance =                           // Restliche Strecke des
    oRaceDistance - DistanceRaced;               //   Rennens ermitteln

  Fuel =                                         // Bedarf an Treibstoff
    (oRemainingDistance + oReserve) * FuelConsum;// für restliche Strecke

  if (Fuel > oMaxFuel)                           // Wenn mehr als eine Tank-
  {                                              //   füllung benötigt wird
    if (Fuel / 2 < oMaxFuel)                     // Bei zwei Tankfüllungen
      Fuel = Fuel / 2;                           //   die Hälfte tanken
    else if (Fuel / 3 < oMaxFuel)                // Bei drei Tankfüllungen.
      Fuel = Fuel / 3;                           //   ein Drittel tanken
    else if (Fuel / 4 < oMaxFuel)                // Bei vier Tankfüllungen.
      Fuel = Fuel / 4;                           //   ein Viertel tanken
    else                                         // Bei fünf Tankfüllungen
      Fuel = Fuel / 5;                           //   ein Fünftel tanken
  };

  if (Fuel > oMaxFuel - CarFuel)                 // Menge ggf. auf freien
    Fuel = oMaxFuel - CarFuel;                   // Tankinhalt begrenzen
  else                                           // ansonsten Bedarf
    Fuel = Fuel - CarFuel;                       // abzügl. Tankinhalt

  oLastPitFuel = (float) MAX(Fuel,0.0);          // Wenn genug da ist = 0.0

  return oLastPitFuel;                           // Menge an TORCS melden
};
//==========================================================================*

//==========================================================================*
// Umfang der Reparaturen festlegen
//--------------------------------------------------------------------------*
int TSimpleStrategy::RepairWanted(int AcceptedDammage)
{
  if (oCar->_dammage < AcceptedDammage)
	return 0;
  else if (oRemainingDistance > 5.5 * oTrackLength)
    return oCar->_dammage;
  else if (oRemainingDistance > 4.5 * oTrackLength)
    return MAX(0,oCar->_dammage - cPIT_DAMMAGE);
  else if (oRemainingDistance > 3.5 * oTrackLength)
    return MAX(0,oCar->_dammage - cPIT_DAMMAGE - 1000);
  else if (oRemainingDistance > 2.5 * oTrackLength)
    return MAX(0,oCar->_dammage - cPIT_DAMMAGE - 2000);
  else
    return MAX(0,oCar->_dammage - cPIT_DAMMAGE - 3000);
}
//==========================================================================*

//==========================================================================*
// Umfang der Reparaturen festlegen
//--------------------------------------------------------------------------*
int TSimpleStrategy::PitRepair()
{
  oState = PIT_EXIT;
  oWasInPit = true;
  return RepairWanted(0);
}
//==========================================================================*

//==========================================================================*
// Tankfüllung beim Start bestimmen
//--------------------------------------------------------------------------*
double TSimpleStrategy::SetFuelAtRaceStart
  (PTrack Track, PCarSettings *CarSettings, PSituation Situation, float Fuel)
{
  oTrack = Track;                                // Save TORCS pointer

  oTrackLength = oTrack->length;                 // Länge der Strecke merken
  oRaceDistance =                                // Gesamtlänge des Rennens
    oTrackLength * Situation->_totLaps;          //   berechnen
  oRemainingDistance =                           // Restliche Strecke des
    oRaceDistance + oReserve;                    //   Rennens ermitteln
  Fuel = (float) 
    (Fuel * oRemainingDistance / 100000.0);      // Gesamtbedarf in kg

  oExpectedFuelPerM = Fuel / oRemainingDistance; // Verbrauch in kg/m

  oMaxFuel =            
	GfParmGetNum(*CarSettings,SECT_PRIV,         // Maximal möglicher
	PRV_MAX_FUEL,(char*) NULL,oMaxFuel);         //   Tankinhalt
  GfOut("oMaxFuel (private) = %.1f\n",oMaxFuel);
  oMinLaps = (int)           
	GfParmGetNum(*CarSettings,SECT_PRIV,         // Mindestanzahl an Runden
	PRV_MIN_LAPS,(char*) NULL,(float) oMinLaps); //   die mit dem Tankinhalt
  GfOut("oMinLaps (private) = %d\n",oMinLaps);   //   möglich sein müssen

  if (Fuel == 0)                                 // Wenn nichts bekannt ist,
    Fuel = oMaxFuel;                             // Volltanken

  oLastFuel = Fuel;                              // Erforderlicher Treibstoff
  if (Fuel > oMaxFuel)                           // Wenn mehr als eine Tank-
  {                                              //   füllung benötigt wird
    if (Fuel / 2 < oMaxFuel)                     // Bei zwei Tankfüllungen
      oLastFuel = Fuel / 2;                      //   die Hälfte tanken
    else if (Fuel / 3 < oMaxFuel)                // Bei drei Tankfüllungen.
      oLastFuel = Fuel / 3;                      //   ein Drittel tanken
    else if (Fuel / 4 < oMaxFuel)                // Bei vier Tankfüllungen.
      oLastFuel = Fuel / 4;                      //   ein Viertel tanken
    else                                         // Bei fünf Tankfüllungen
      oLastFuel = Fuel / 5;                      //   ein Fünftel tanken
  };

  oLastFuel = MIN(oLastFuel, oMaxFuel);          // Überlaufen verhindern
  GfParmSetNum(*CarSettings, SECT_CAR, PRM_FUEL, // Gewünschte Tankfüllung
    (char*) NULL, oLastFuel);                    //   an TORCS melden

  return oLastFuel;
};
//==========================================================================*

//==========================================================================*
// Go to pit
//--------------------------------------------------------------------------*
bool TSimpleStrategy::GoToPit()
{
  return ((oState >= PIT_ENTER) && (oState <= PIT_GONE));
};
//==========================================================================*

//==========================================================================*
// Start entry procedure to pit?
//--------------------------------------------------------------------------*
bool TSimpleStrategy::StartPitEntry(float& Ratio)
{
  float DLong, DLat;                             // Dist to Pit
  RtDistToPit(oCar,oTrack,&DLong,&DLat);
  if (GoToPit() && (DLong < oPit->oPitLane->PitDist()))
  {
    Ratio = (float) (1.0 - MAX(0.0,(DLong-100)/oPit->oPitLane->PitDist()));
    return true;
  }
  else
    return false;
};
//==========================================================================*

//==========================================================================*
// Stop entry procedure to pit?
//--------------------------------------------------------------------------*
bool TSimpleStrategy::StopPitEntry(float Offset)
{
  float DLong, DLat;                             // Dist to Pit
  RtDistToPit(oCar,oTrack,&DLong,&DLat);
  if (oWasInPit && (DLong - oTrackLength) > -Offset)
  {
    return true;
  }
  else
  {
    oWasInPit = false;
    return false;
  }
};
//==========================================================================*

//==========================================================================*
// Update Data
//--------------------------------------------------------------------------*
void TSimpleStrategy::Update(PtCarElt Car)
{
  double CurrentFuelConsum;                      // Current fuel consum
  float DL;                                      // Distance longitudinal
  float DW;                                      // Distance lateral  

  oCar = Car;                                    // Save pointer

  RtDistToPit                                    // Get distance to pit
   (Car,oTrack,&DL,&DW);                         

  if (DL < 0)                                    // Make DL be >= 0.0
	DL = DL + oTrack->length;                    // to have it to front

  if ((DL < oDistToSwitch) && (DL > 50) && (!oFuelChecked))
  { // We passed the line to check our fuel consume!
    if (CarLaps > 1)                             // Start at lap 2
    {                                            //   to get values
      CurrentFuelConsum =                        // Current consume =
        (oLastFuel                               // Last tank capacity
        + oLastPitFuel                           // + refueled
        - oCar->priv.fuel)                       // - current capacity
        / oTrackLength;                          // related to track length

      if (oFuelPerM == 0.0)                      // At first time we use
        oFuelPerM = (float) CurrentFuelConsum;    //   our initial estimation
      else                                       // later
        oFuelPerM = (float)                      //   we get the mean
          ((oFuelPerM*CarLaps+CurrentFuelConsum) //   of what we needed till now
		  / (CarLaps + 1));

    };

    oLastFuel = oCar->priv.fuel;                 // Capacity at this estimation
    oLastPitFuel = 0.0;                          // Refueled
    oFuelChecked = true;                         // We did the estimation in this lap

    if (!oGoToPit)                               // If decision isn'd made
  	  oGoToPit = NeedPitStop();                  // Check if we sholud have a pitstop
  }
  else if (DL < 50)                              // I we are out of the window
  {                                              // to estimate
    oFuelChecked = false;                        // reset flag
  };
};
//==========================================================================*

//==========================================================================*
// State (Sequential logic system)
//--------------------------------------------------------------------------*
void TSimpleStrategy::CheckPitState(float PitScaleBrake)
{
  if (oPit == NULL)                              // No Pit no service 
    return;

  double TrackPos = RtGetDistFromStart(oCar);    // Distance to pit

  switch(oState)                                 // Check state
  {
	case PIT_NONE:
      // We are somewhere on the track, nothing has happend yet
	  if ((!oPit->oPitLane[0].InPitSection(TrackPos)) && oGoToPit)
	  { // if we are not parallel to the pits and get the flag,
		// let's stop in the pits.
		oState = PIT_BEFORE;
	  }
	  break;

	case PIT_BEFORE:
      // We are somewhere on the track and got the flag to go to pit
	  if (oPit->oPitLane[0].InPitSection(TrackPos) && oGoToPit)
	  { // If we reache pit entry and flag is still set
	    // switch to the pitlane
		oState = PIT_ENTER;
	  }
	  break;

	case PIT_ENTER:
      // We are on the pitlane and drive to out pit
 	  if (!oPit->oPitLane[0].CanStop(TrackPos))
	  { // We have to wait, till we reached the point to stop
		break;
	  }

	  // We reached the poit to stopp
	  oState = PIT_ASKED;

 	  // falls through...

	case PIT_ASKED:
	  // We are still going to the pit
	  if (oPit->oPitLane[0].CanStop(TrackPos))
	  { // If we can stop a this position we start pit procedure
		oDriver->oStanding = true;               // For motion survey!
        oPitTicker = 0;                          // Start service timer
	    oCar->ctrl.accelCmd = 0;                 // release throttle
	    oCar->ctrl.brakeCmd = 0.5f*PitScaleBrake;// Start braking
	    oCar->ctrl.raceCmd = RM_CMD_PIT_ASKED;   // Tell TORCS to service us! To test oPitTicker comment out
	    oState = PIT_SERVICE;                    
	  }
	  else
	  { // We can't stop here (to early or to late)
	    if (oPit->oPitLane[0].Overrun(TrackPos))
		{ // if to late
		  GfOut("Overrun 1: %g\n",TrackPos);
		  PitRelease();
	      oState = PIT_EXIT;
	      // pit stop finished, need to exit pits now.
		}
	  }
	  break;

	case PIT_SERVICE:
      // Wait to reach standstill to get service from TORCS
	  oDriver->oStanding = true;                 // Keep motion survey quiet
	  oPitTicker++;                              // Check time to start service
	  if (oPitTicker > 150)                      // Check Timer
	  { // If we have to wait to long
		GfOut("oPitTicker: %d\n",oPitTicker);
	    PitRelease();                            // Something went wrong, we have 
	    oState = PIT_EXIT;                       // to leave and release pit for teammate
	  }
	  else if (oPit->oPitLane[0].Overrun(TrackPos))
	  { // If we couldn't stop in place
		GfOut("Overrun 2: %g\n",TrackPos);
	    PitRelease();                            // We have to release the pit
	    oState = PIT_EXIT;                       // for teammate
	  }
	  else
	  { // There is nothing that hampers TORCS to service us
        oCar->ctrl.accelCmd = 0;                 // No throttle
	    oCar->ctrl.brakeCmd = 1.0f*PitScaleBrake;// Still braking
	    oCar->ctrl.raceCmd = RM_CMD_PIT_ASKED;   // Tell TORCS to service us! To test oPitTicker comment out
        // oState is set to next state in PitRepair()!
		// If TORCS doesn't service us, no call to PitRepair() is done!
		// We run into timeout! (oPitTicker)
	  }
	  break;

	case PIT_EXIT:
      // We are still in the box
	  oDriver->oStanding = true;                 // Keep motion survey quiet
  	  oGoToPit = false;                          // Service is finished, lets go
	  oState = PIT_GONE;                          
	  oCar->ctrl.accelCmd = 0.5;                 // Press throttle
	  oCar->ctrl.brakeCmd = 0;                   // Release brake
	  PitRelease();                              // Release pit for teammate
	  break;

	case PIT_GONE:
      // We are on our path back to the track
	  if (!oPit->oPitLane[0].InPitSection(TrackPos))
	  { // If we reached the end of the pitlane
		oState = PIT_NONE;                       // Switch to default mode 
	  }
 	  break;
  }
}
//==========================================================================*
// end of file unitstrategy.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
