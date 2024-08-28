/***************************************************************************

    file                 : trackexporter.cpp
    created              : gio 5 mar 2015, 11.07.57, CET
    copyright            : (C) 2015 Jacopo Sirianni

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <math.h>
#include <vector>

#include <tgf.h> 
#include <track.h> 
#include <car.h> 
#include <raceman.h> 
#include <robottools.h>
#include <robot.h>

static tTrack	*curTrack;

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s); 
static void newrace(int index, tCarElt* car, tSituation *s); 
static void drive(int index, tCarElt* car, tSituation *s); 
static void endrace(int index, tCarElt *car, tSituation *s);
static void shutdown(int index);
static int  InitFuncPt(int index, void *pt); 


/* 
 * Module entry point  
 */ 
extern "C" int 
trackexporter(tModInfo *modInfo) 
{
    memset(modInfo, 0, 10*sizeof(tModInfo));

    modInfo->name    = strdup("trackexporter");	/* name of the module (short) */
    modInfo->desc    = strdup("Export (part of) the track data structures to a CSV file");	/* description of the module (can be long) */
    modInfo->fctInit = InitFuncPt;		/* init function */
    modInfo->gfId    = ROB_IDENT;		/* supported framework version */
    modInfo->index   = 1;

    return 0; 
} 

/* Module interface initialization. */
static int 
InitFuncPt(int index, void *pt) 
{ 
    tRobotItf *itf  = (tRobotItf *)pt; 

    itf->rbNewTrack = initTrack; /* Give the robot the track view called */ 
				 /* for every track change or new race */ 
    itf->rbNewRace  = newrace; 	 /* Start a new race */
    itf->rbDrive    = drive;	 /* Drive during race */
    itf->rbPitCmd   = NULL;
    itf->rbEndRace  = endrace;	 /* End of the current race */
    itf->rbShutdown = shutdown;	 /* Called before the module is unloaded */
    itf->index      = index; 	 /* Index used if multiple interfaces */
    
    return 0; 
} 

/* Called for every track change or new race. */
static void  
initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s) 
{ 
    curTrack = track;
    *carParmHandle = NULL;

    std::string outputFolder(getenv("HOME"));
    outputFolder.append("/.torcs/tracks/");

    /* GfCreateDir wants a non-const char* as input */
    std::vector<char> outputFolderChars(outputFolder.begin(), outputFolder.end());
    outputFolderChars.push_back('\0');

    if (GfCreateDir(&outputFolderChars[0]) == GF_DIR_CREATED) {
        std::string outputFile(outputFolder);
        outputFile.append(track->internalname);
        outputFile.append(".csv");
 	
        std::ofstream trackCSV;
        trackCSV.open(outputFile.c_str(), std::ios::out);

        /* Export data */
        
        /* Track's global data on the first line */

        trackCSV << track->length << "," << track->width << std::endl;
        
        /* Segments' data on the following lines */
    
        tTrackSeg* startSeg = track->seg;
        tTrackSeg* seg = track->seg;

        do {
            trackCSV << seg->vertex[TR_SL].x << "," << seg->vertex[TR_SL].y << "," << seg->vertex[TR_SL].z << ","
                     << seg->vertex[TR_SR].x << "," << seg->vertex[TR_SR].y << "," << seg->vertex[TR_SR].z << ","
                     << seg->vertex[TR_ER].x << "," << seg->vertex[TR_ER].y << "," << seg->vertex[TR_ER].z << ","
                     << seg->vertex[TR_EL].x << "," << seg->vertex[TR_EL].y << "," << seg->vertex[TR_EL].z << ","
                     << seg->length << "," << seg->type << "," 
                     << seg->center.x << "," << seg->center.y << ","
                     << seg->angle[0] << "," << seg->radius << std::endl;
            seg = seg->next;
        } while (seg != startSeg);

        /* End of export data */

        trackCSV.close();
    } else {
        std::cerr << "ERROR: Track folder not created!" << std::endl;
        exit(-1);
    }
} 

/* Start a new race. */
static void  
newrace(int index, tCarElt* car, tSituation *s) 
{
    /* Retire immediately */
    car->_state = RM_CAR_STATE_ELIMINATED;
} 

/* Drive during race. */
static void  
drive(int index, tCarElt* car, tSituation *s) 
{
}

/* End of the current race */
static void
endrace(int index, tCarElt *car, tSituation *s)
{
}

/* Called before the module is unloaded */
static void
shutdown(int index)
{
}

