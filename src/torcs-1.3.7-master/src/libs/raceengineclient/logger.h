/***************************************************************************

    Copyright (C) 2015 Jacopo Sirianni                               

    Header file of a logger for TORCS.

    The goal of this logger is to log drivers' actions and status during
    a race and to store everything inside a text file named
    ~/.torcs/logs/yyyy-mm-dd hh:mm:ss trackName.csv
    Make sure that that directory exists before using this logger.
    See the .cpp file to see/customize what is logged by this logger.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef _LOGGER_H_
#define _LOGGER_H_



#include <fstream>
#include <map>

#include <raceman.h>


class Logger
{
	std::ofstream logFile;
	std::map<char*,int> oldPositions;
	std::map<char*,bool> raceEndedMap;
	
	public:
		Logger(tSituation *s, const char *trackName);
		~Logger();
		void log(tSituation *s);
};



#endif /* _LOGGER_H_ */ 
