/***************************************************************************

    Copyright (C) 2015 Jacopo Sirianni                               

    Header file of a logger for TORCS.

    The goal of this logger is to log drivers' actions and status during
    a race and to store everything inside a text file named
    ~/.torcs/logs/yyyy-mm-dd hh:mm:ss trackName.csv
    Make sure that that directory exists before using this logger.
    
    This version includes optimizations to reduce file size while maintaining
    data quality and compatibility with analysis tools.

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

// Log every 2 seconds (100 simulation steps at 20ms each)
const int LOGGER_SAMPLE_RATE = 100;

class Logger {
private:
    std::ofstream logFile;
    std::map<char*, int> oldPositions;
    std::map<char*, bool> raceEndedMap;
    int skipCounter;

    void logRegularData(tSituation *s, int carIndex);
    void checkAndLogOvertakes(tSituation *s);

public:
    Logger(tSituation *s, const char *trackName);
    ~Logger();
    void log(tSituation *s);
};

#endif