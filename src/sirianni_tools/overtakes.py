"""Plot overtakes and analyze blocks."""

import csv
import math
import os
import sys
import subprocess

import matplotlib
import matplotlib.pyplot as plt
import numpy

import blocks
import utils
from utils import SegmentType

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



def getTrackWidth(trackName):
    if not os.path.exists(utils.torcsTrackDirectory + trackName + ".csv"):
        sys.exit("ERROR: track data file does not exist!")
    else:
        with open(utils.torcsTrackDirectory + trackName + ".csv", "r") as f:
            return float(f.readline().strip().rsplit(",")[1])


def fillOvertakes(trackData, logFile):
    try:
        with open(logFile, 'r') as f:
            lines = f.readlines()
            overtake_count = 0
            for line in lines:
                if 'overtake' in line:
                    parts = line.strip().split(',')
                    if len(parts) >= 3:
                        try:
                            length = float(parts[2])  # Distance where overtake occurred
                            for seg in trackData:
                                if seg[blocks.ENDLENGTH] >= length:
                                    seg[blocks.OVERTAKES] += 1
                                    overtake_count += 1
                                    break
                        except ValueError:
                            print(f"Warning: Invalid length value in line: {line.strip()}")
                    else:
                        print(f"Warning: Not enough data fields in line: {line.strip()}")
            print(f"Processed {overtake_count} overtakes in {logFile}")
        return trackData, overtake_count
    except Exception as e:
        print(f"Error processing overtakes: {e}")
        return trackData, 0



def computeBlockOvertakes(trackData):
    """Compute overtakes per block"""
    block = -1
    blockOvertakes = []
    
    for seg in trackData:
        if seg[blocks.BLOCK] == block:
            blockOvertakes[block] += seg[blocks.OVERTAKES]
        else:
            block += 1
            blockOvertakes.append(seg[blocks.OVERTAKES])
            
    return blockOvertakes

def drawCarPositions(trackData, logFile, trackName):
    records = []
    width = getTrackWidth(trackName)
    playerFound = False
    
    try:
        with open(logFile, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            if 'overtake' in line:
                parts = line.strip().split(',')
                if len(parts) >= 15:  # Ensure we have enough fields
                    record = []
                    record.append(parts[0])  # timestamp
                    record.append(parts[3])  # overtaker
                    record.append(parts[8])  # overtaken
                    records.append(record)

        # Process records for position plotting
        for rec in records:
            timestamp = rec[0]
            overtaker = rec[1]
            overtaken = rec[2]
            
            overtaker_line = None
            overtaken_line = None
            
            for line in lines:
                parts = line.strip().split(',')
                if len(parts) > 11:
                    if parts[1] == overtaker and abs(float(parts[0]) - float(timestamp)) < 0.1:
                        overtaker_line = parts
                    elif parts[1] == overtaken and abs(float(parts[0]) - float(timestamp)) < 0.1:
                        overtaken_line = parts
                
                if overtaker_line and overtaken_line:
                    break
            
            if overtaker_line:
                try:
                    xy = rtTrackLocal2Global(
                        dist2Seg(float(overtaker_line[11]), trackData),
                        float(overtaker_line[6]),
                        float(overtaker_line[8]),
                        width
                    )
                    
                    if overtaker_line[1] == "player":
                        plt.plot(xy[0], xy[1], color="#007700", marker="+", markersize=5)
                        playerFound = True
                    else:
                        plt.plot(xy[0], xy[1], color="#5555ff", marker="+", markersize=5)
                except Exception as e:
                    print(f"Warning: Could not plot overtaker position: {e}")

            if overtaken_line:
                try:
                    xy = rtTrackLocal2Global(
                        dist2Seg(float(overtaken_line[11]), trackData),
                        float(overtaken_line[6]),
                        float(overtaken_line[8]),
                        width
                    )
                    
                    if overtaken_line[1] == "player":
                        plt.plot(xy[0], xy[1], color="#666666", marker="+", markersize=5)
                        playerFound = True
                    else:
                        plt.plot(xy[0], xy[1], "k+", markersize=5)
                except Exception as e:
                    print(f"Warning: Could not plot overtaken position: {e}")

        return playerFound
        
    except Exception as e:
        print(f"Warning: Error processing log file {logFile}: {e}")
        return False



def copy_overtakes_data(source_data, target_data):
    """
    Copy overtakes data from source track data to target track data.
    
    Args:
        source_data: List of track segments with overtakes data
        target_data: List of track segments to copy overtakes to
        
    Returns:
        Updated target_data with copied overtakes
    """
    if len(source_data) != len(target_data):
        print(f"Warning: Source ({len(source_data)}) and target ({len(target_data)}) segment counts don't match")
        return target_data
        
    for i in range(len(source_data)):
        # Ensure we have enough fields in both source and target segments
        while len(source_data[i]) <= blocks.OVERTAKES:
            source_data[i].append(0)
        while len(target_data[i]) <= blocks.OVERTAKES:
            target_data[i].append(0)
            
        # Copy the overtakes count
        target_data[i][blocks.OVERTAKES] = source_data[i][blocks.OVERTAKES]
        
    return target_data

def makeOvertakePlotsAndSegmentDataFile(folder, logList, trackDirectory, trackName, maxBlockLength, generate_plots=True):
    """Main function to analyze and plot overtakes"""
    overtakesFolder = os.path.join(folder, "overtakes")
    if not os.path.exists(overtakesFolder):
        os.makedirs(overtakesFolder)

    # Get base track data
    allBlockData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    allBlockData = blocks.readDynamics(trackName, allBlockData)

    # Process each log file
    totalOvertakes = 0
    for index, log in enumerate(logList):
        print(f"==> Analyzing {log} ({index + 1}/{len(logList)})")
        
        # Get fresh track data for this log
        trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
        if trackData is None:
            continue
            
        # Fill in overtakes data for this log
        trackData, overtake_count = fillOvertakes(trackData, os.path.join(folder, log))
        totalOvertakes += overtake_count
        
        # Copy overtakes data to the cumulative allBlockData
        allBlockData = copy_overtakes_data(trackData, allBlockData)

        # Create plots for this log
        plotOvertakes(trackData, folder, [log], 
                     os.path.join(overtakesFolder, log), trackName, generate_plots)
        
    # Process combined data if we have multiple logs
    if len(logList) > 1:
        print("==> Analyzing races together")
        plotOvertakes(allBlockData, folder, logList, os.path.join(overtakesFolder, "overtakes"), trackName, generate_plots)

    return totalOvertakes


def plotOvertakes(blockData, folder, logList, path, trackName, generate_plots=True):
    if not generate_plots:
        return
    blocks.plotMetric(blockData, computeBlockOvertakes(blockData), "Number of overtakes", path + ".svg", True, False)


def makeBlocksDataWithoutOvertakes(folder, trackDirectory, trackName, maxBlockLength):
    trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    trackData = blocks.readDynamics(trackName, trackData)

def rtTrackLocal2Global(seg, toStart, toMiddle, width):
    """Convert local track coordinates to global coordinates.
    The segment types in track data are:
    1 = LEFT turn
    2 = RIGHT turn
    3 = STRAIGHT
    """
    import math
    
    try:
        seg_type = int(seg[blocks.TYPE])
        
        # Straight segment (type 3)
        if seg_type == SegmentType.straight.value:
            CosA = math.cos(float(seg[blocks.ANGLE_ZS]))
            SinA = math.sin(float(seg[blocks.ANGLE_ZS]))
            tr = toMiddle + width / 2
            x = float(seg[blocks.SRX]) + toStart * CosA - tr * SinA
            y = float(seg[blocks.SRY]) + toStart * SinA + tr * CosA
            
        # Left turn (type 1)
        elif seg_type == 1:
            a = float(seg[blocks.ANGLE_ZS]) + toStart / float(seg[blocks.RADIUS])
            r = float(seg[blocks.RADIUS]) - toMiddle
            x = float(seg[blocks.CENTER_X]) + r * math.sin(a)
            y = float(seg[blocks.CENTER_Y]) - r * math.cos(a)
            
        # Right turn (type 2)
        elif seg_type == 2:
            a = float(seg[blocks.ANGLE_ZS]) - toStart / float(seg[blocks.RADIUS])
            r = float(seg[blocks.RADIUS]) + toMiddle
            x = float(seg[blocks.CENTER_X]) - r * math.sin(a)
            y = float(seg[blocks.CENTER_Y]) + r * math.cos(a)
        
        return (x, y)
        
    except Exception as e:
        print(f"Error in RtTrackLocal2Global: {str(e)}")
        return (0, 0)


def dist2Seg(dist, trackData):
    try:
        segIndex = 0
        curDist = trackData[segIndex][blocks.LENGTH]
        
        # Keep accumulating distance until we find the right segment
        # or reach the end of the track
        while dist > curDist and segIndex < len(trackData) - 1:
            segIndex += 1
            curDist += trackData[segIndex][blocks.LENGTH]
            
        return trackData[segIndex]
    except Exception as e:
        print(f"Warning: Error in dist2seg: {e}")
        return None