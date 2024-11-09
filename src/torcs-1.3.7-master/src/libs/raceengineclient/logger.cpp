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
#include <time.h>
#include <vector>



Logger::Logger(tSituation *s, const char *trackName)
{
	// Get the current time
	
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
 	char timeString[20];
 	strftime(timeString, 20, "%F %T", timeinfo);

	// Make the log folder

	std::string outputFolder(getenv("HOME"));
	outputFolder.append("/.torcs/logs/");

	/* GfCreateDir wants a non-const char* as input */
	std::vector<char> outputFolderChars(outputFolder.begin(), outputFolder.end());
	outputFolderChars.push_back('\0');

	if (GfCreateDir(&outputFolderChars[0]) == GF_DIR_CREATED)
	{
		// Construct the (absolute) filename (~/.torcs/logs/yyyy-mm-dd hh:mm:ss trackName.csv)

		std::string filePath(outputFolder);
		filePath.append(timeString);
		filePath.append(" ");
		filePath.append(trackName);
		filePath.append(".csv");
	
		// Create the log file
	
		logFile.open(filePath.c_str(), std::ios::out);

		// Initialize starting positions
	
		for(int i = 0; i < s->_ncars; i++)
		{
			oldPositions.insert(std::pair<char*,int>(s->cars[i]->info.name, s->cars[i]->race.pos));
			raceEndedMap.insert(std::pair<char*,bool>(s->cars[i]->info.name, false));
		}
	}
	else
	{
		std::cerr << "ERROR: Logs folder not created!" << std::endl;
		exit(-1);
	}
}



Logger::~Logger()
{
	logFile.close();
}



void Logger::log(tSituation *s)
{
	for(int i = 0; i < s->_ncars; i++)
	{
		if(s->cars[i]->race.laps <= s->raceInfo.totLaps)
		{
			// (Really) verbose logging

			logFile << s->currentTime << ","
				<< s->cars[i]->info.name << "," << s->cars[i]->info.carName << "," << s->cars[i]->info.driverType << "," << s->cars[i]->info.skillLevel << ","
				<< s->cars[i]->pub.speed << "," << s->cars[i]->pub.trkPos.toStart << "," << s->cars[i]->pub.trkPos.toRight << "," << s->cars[i]->pub.trkPos.toMiddle << "," << s->cars[i]->pub.trkPos.toLeft << "," << s->cars[i]->pub.state << ","
				<< s->cars[i]->race.distFromStartLine << "," << s->cars[i]->race.laps << "," << s->cars[i]->race.pos << "," << s->cars[i]->race.bestLapTime << "," << s->cars[i]->race.curLapTime << "," << s->cars[i]->race.topSpeed << "," << s->cars[i]->race.timeBehindPrev << "," << s->cars[i]->race.timeBeforeNext << "," 
				<< s->cars[i]->priv.driverIndex << "," << s->cars[i]->priv.dammage << "," << s->cars[i]->priv.fuel << ","
				<< s->cars[i]->ctrl.steer << "," << s->cars[i]->ctrl.accelCmd << "," << s->cars[i]->ctrl.brakeCmd << "," << s->cars[i]->ctrl.gear
				<< std::endl;

			if(s->cars[i]->race.pos < oldPositions[s->cars[i]->info.name])
			{
				// i has overtaken, find the overtaken car

				int overtakenIndex;

				for(int j = 0; j < s->_ncars; j++)
				{
					if(s->cars[j]->race.pos == oldPositions[s->cars[i]->info.name])
					{
						overtakenIndex = j;
						break;
					}
				}

				// Log the overtake

				logFile << s->currentTime << ",overtake," << s->cars[i]->race.distFromStartLine << ","
					<< s->cars[i]->info.name << "," << s->cars[i]->info.carName << "," << s->cars[i]->pub.state << "," << s->cars[i]->priv.dammage << "," << s->cars[i]->priv.fuel << "," << s->cars[i]->race.laps << ","
					<< s->cars[overtakenIndex]->info.name << "," << s->cars[overtakenIndex]->info.carName << "," << s->cars[overtakenIndex]->pub.state << "," << s->cars[overtakenIndex]->priv.dammage << "," << s->cars[overtakenIndex]->priv.fuel << "," << s->cars[overtakenIndex]->race.laps
					<< s->cars[i]->pub.posMat[3][0] << "," << s->cars[i]->pub.posMat[3][2] << "," << s->cars[i]->pub.posMat[overtakenIndex][0] << "," << s->cars[overtakenIndex]->pub.posMat[3][2]
					<< std::endl;
			}

			oldPositions[s->cars[i]->info.name] = s->cars[i]->race.pos;
		}
		else
		{
			// If the race has ended log the gap from the previous driver
			// The current gap was not available at the previuos step of the simulation (e.g. if the race lasts for three laps, the gap is computed at the first step of lap four)

			if(!raceEndedMap[s->cars[i]->info.name])
			{
				logFile << "timeBehindPrev," << s->cars[i]->info.name << "," << s->cars[i]->race.timeBehindPrev << std::endl;
				raceEndedMap[s->cars[i]->info.name] = true;
			}
		}
	}
}
