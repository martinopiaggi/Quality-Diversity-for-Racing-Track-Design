"""Utility functions"""

import enum
import os
import subprocess
import sys

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



torcsCommand = "~/tesi/bin/bin/torcs"
torcsLogPath = "./logs_analysis/"
torcsTrackDirectory = "./logs_analysis/"
maxBendRadius = 1200

@enum.unique
class SegmentType(enum.Enum):
	none = 0
	right = 1
	left = 2
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
	return [file for file in os.listdir(folder) if file[-4:] == ".csv" and file[0] == "2"]



def getDriversList(log):
	return subprocess.check_output('awk -F "," \'{if($1 == -1.998) print $2; else exit 0}\' "' + log + '"', shell = True).splitlines()



def getTrackLength(trackName):
	if not os.path.exists(torcsTrackDirectory + trackName + ".csv"):
		sys.exit("ERROR: track data file does not exist!")
	else:
		with open(torcsTrackDirectory + trackName + ".csv", "r") as f:
			return float(f.readline().strip().rsplit(",")[0])



def printHeading(title):
	print("\033[92m>>> " + title + "...\033[0m")
