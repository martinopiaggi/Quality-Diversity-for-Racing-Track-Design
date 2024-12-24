/***************************************************************************

    Copyright (C) 2015 Jacopo Sirianni                               

    Implementation of a logger for TORCS.
    
    Notes:
    - the name of the log is hardcoded in the constructor
    - the Logger::log() method does the actual logging given the current
      situation
    - the "verbose" section is **REALLY** verbose (it logs almost
      everything) => make sure to have some free GBs or comment it
    - particular attention has been put to the logging of overtakes

    Changes from the old version:
    - removed logging after the end of the last lap
    - added logging of final gaps in a separate line
    - removed ";" at the end of the line
    - added folder creation and reviewed filename generation

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "logger.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>

Logger::Logger(tSituation *s, const char *trackName) : skipCounter(0) {
    // Get current time for filename
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y%m%d_dynamics", timeinfo);

    // Setup log directory
    std::string outputFolder(getenv("HOME"));
    outputFolder.append("/.torcs/logs/");

    // Convert to C string for GfCreateDir
    char outputPath[1024];
    strncpy(outputPath, outputFolder.c_str(), sizeof(outputPath) - 1);
    outputPath[sizeof(outputPath) - 1] = '\0';

    if (GfCreateDir(outputPath) == GF_DIR_CREATED) {
        std::string filePath(outputFolder);
        filePath.append(trackName);
        filePath.append("_");
        filePath.append(timeString);
        filePath.append(".csv");
        logFile.open(filePath.c_str(), std::ios::out);

        // Initialize position tracking
        for(int i = 0; i < s->_ncars; i++) {
            oldPositions[s->cars[i]->info.name] = s->cars[i]->race.pos;
            raceEndedMap[s->cars[i]->info.name] = false;
        }
    } else {
        std::cerr << "ERROR: Could not create logs folder!" << std::endl;
        exit(-1);
    }
}

Logger::~Logger() {
    logFile.close();
}

void Logger::logRegularData(tSituation *s, int carIndex) {
    if(s->cars[carIndex]->race.laps <= s->raceInfo.totLaps) {
        logFile << s->currentTime << ","
            << s->cars[carIndex]->info.name << "," 
            << s->cars[carIndex]->info.carName << "," 
            << s->cars[carIndex]->info.driverType << "," 
            << s->cars[carIndex]->info.skillLevel << ","
            << s->cars[carIndex]->pub.speed << "," 
            << s->cars[carIndex]->pub.trkPos.toStart << "," 
            << s->cars[carIndex]->pub.trkPos.toRight << "," 
            << s->cars[carIndex]->pub.trkPos.toMiddle << "," 
            << s->cars[carIndex]->pub.trkPos.toLeft << "," 
            << s->cars[carIndex]->pub.state << ","
            << s->cars[carIndex]->race.distFromStartLine << "," 
            << s->cars[carIndex]->race.laps << "," 
            << s->cars[carIndex]->race.pos << "," 
            << s->cars[carIndex]->race.bestLapTime << "," 
            << s->cars[carIndex]->race.curLapTime << "," 
            << s->cars[carIndex]->race.topSpeed << "," 
            << s->cars[carIndex]->race.timeBehindPrev << "," 
            << s->cars[carIndex]->race.timeBeforeNext << ","
            << s->cars[carIndex]->priv.driverIndex << "," 
            << s->cars[carIndex]->priv.dammage << "," 
            << s->cars[carIndex]->priv.fuel << ","
            // Control inputs (needed for blocks.py analysis)
            << s->cars[carIndex]->ctrl.steer << "," 
            << s->cars[carIndex]->ctrl.accelCmd << "," 
            << s->cars[carIndex]->ctrl.brakeCmd << "," 
            << s->cars[carIndex]->ctrl.gear
            << std::endl;
    }
}

void Logger::checkAndLogOvertakes(tSituation *s) {
    for(int i = 0; i < s->_ncars; i++) {
        if(s->cars[i]->race.laps <= s->raceInfo.totLaps) {
            if(s->cars[i]->race.pos < oldPositions[s->cars[i]->info.name]) {
                // Find overtaken car
                int overtakenIndex;
                for(int j = 0; j < s->_ncars; j++) {
                    if(s->cars[j]->race.pos == oldPositions[s->cars[i]->info.name]) {
                        overtakenIndex = j;
                        break;
                    }
                }

                // Log overtake event with full position data
                logFile << s->currentTime << ",overtake," 
                    << s->cars[i]->race.distFromStartLine << ","
                    << s->cars[i]->info.name << "," 
                    << s->cars[i]->info.carName << "," 
                    << s->cars[i]->pub.state << "," 
                    << s->cars[i]->priv.dammage << "," 
                    << s->cars[i]->priv.fuel << "," 
                    << s->cars[i]->race.laps << ","
                    << s->cars[overtakenIndex]->info.name << "," 
                    << s->cars[overtakenIndex]->info.carName << "," 
                    << s->cars[overtakenIndex]->pub.state << "," 
                    << s->cars[overtakenIndex]->priv.dammage << "," 
                    << s->cars[overtakenIndex]->priv.fuel << "," 
                    << s->cars[overtakenIndex]->race.laps << ","
                    // Position matrix data for track visualization
                    << s->cars[i]->pub.posMat[3][0] << "," 
                    << s->cars[i]->pub.posMat[3][2] << "," 
                    << s->cars[overtakenIndex]->pub.posMat[3][0] << "," 
                    << s->cars[overtakenIndex]->pub.posMat[3][2]
                    << std::endl;
            }
            oldPositions[s->cars[i]->info.name] = s->cars[i]->race.pos;
        } else if(!raceEndedMap[s->cars[i]->info.name]) {
            // Log final gap
            logFile << "timeBehindPrev," 
                << s->cars[i]->info.name << "," 
                << s->cars[i]->race.timeBehindPrev 
                << std::endl;
            raceEndedMap[s->cars[i]->info.name] = true;
        }
    }
}

void Logger::log(tSituation *s) {
    // Always check for overtakes
    checkAndLogOvertakes(s);
    
    // Regular logging every LOGGER_SAMPLE_RATE steps
    skipCounter++;
    if (skipCounter >= LOGGER_SAMPLE_RATE) {
        for(int i = 0; i < s->_ncars; i++) {
            logRegularData(s, i);
        }
        skipCounter = 0;
    }
}