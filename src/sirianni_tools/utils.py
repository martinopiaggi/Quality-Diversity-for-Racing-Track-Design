"""Utility functions"""

import enum
import os
import subprocess
import sys

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



torcsCommand = "/usr/local/bin/torcs"
torcsLogPath = "/root/.torcs/logs/"
torcsTrackDirectory = "/root/.torcs/tracks/"
maxBendRadius = 1200

@enum.unique
class SegmentType(enum.Enum):
    none = 0
    left = 1
    right = 2
    straight = 3

@enum.unique
class SegmentData(enum.Enum):
    # Vertex data (s = start, e = end, l = left, r = right)
    slx = 0
    sly = 1
    slz = 2
    srx = 3
    sry = 4
    srz = 5
    erx = 6
    ery = 7
    erz = 8
    elx = 9
    ely = 10
    elz = 11
    # Other data
    length = 12
    type = 13
    centerX = 14
    centerY = 15
    angleZS = 16
    radius = 17

def getLogList(folder):
    """Get list of log files from folder"""
    try:
        return sorted([
            file for file in os.listdir(folder) 
            if file.endswith('.csv') and file.startswith('2')
        ])
    except Exception as e:
        print(f"Warning: Error getting log list: {e}")
        return []

def getTrackLength(trackName):
    """Get track length from track file"""
    track_file = os.path.join(torcsTrackDirectory, f"{trackName}.csv")
    try:
        with open(track_file, 'r') as f:
            return float(f.readline().strip().split(',')[0])
    except Exception as e:
        print(f"Warning: Error getting track length: {e}")
        return 0

def getDriversList(log):
    try:
        with open(log, 'r') as f:
            drivers = []
            for _ in range(10):  # Process first 10 lines
                line = f.readline().strip()
                parts = line.split(',')
                if len(parts) >= 2 and parts[0].startswith('-1.8'):
                    driver = parts[1].strip()
                    if driver not in drivers:
                        drivers.append(driver)
            return sorted(drivers)
    except Exception as e:
        print(f"Error reading drivers list: {e}")
        return []

def getTrackLength(trackName):
    if not os.path.exists(torcsTrackDirectory + trackName + ".csv"):
        sys.exit("ERROR: track data file does not exist!")
    else:
        with open(torcsTrackDirectory + trackName + ".csv", "r") as f:
            return float(f.readline().strip().rsplit(",")[0])



def printHeading(title):
    print("\033[92m>>> " + title + "...\033[0m")
