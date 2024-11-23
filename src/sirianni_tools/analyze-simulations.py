#!/usr/bin/env python

"""Analyzer of the data collected during the races"""

import argparse
import csv
import os
import string
import subprocess
import shutil

import blocks
import gaps
import overtakes
import positions
import racegen
import track
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



parser = argparse.ArgumentParser(description = __doc__)

parser.add_argument("-B", "--max-block-len", type = int, default = 200, help = "the maximum block's length (used for the heatmaps and the segment data file, default: %(default)s)")
parser.add_argument("-c", "--plot-cars", action="store_true", help = "make additional overtakes plots with cars positions (default: %(default)s)")
parser.add_argument("paths", nargs = "+", help = "the list of paths which contain the simulation logs")

args = parser.parse_args()



for path in args.paths:
	folderName = os.path.relpath(path).rstrip("/")

	print("\033[94m>>> Analyzing " + folderName + "...\033[0m")
	utils.printHeading("Reading initialization data")

	# hardcoded configuration:
	botSkills = [
		("damned", "medium"),
		("bt", "medium"),
		("tita", "medium"),
		("olethros", "medium"),
		("simplix", "medium")
	]

	logList = utils.getLogList(folderName)
	driversList = utils.getDriversList(folderName + "/" + logList[0])

	utils.printHeading("Analyzing track and track dynamics")
	trackName = logList[0][20:-4]
	
	if not os.path.exists(utils.torcsTrackDirectory + trackName + ".csv"):
    	subprocess.check_call(utils.torcsCommand + " -r /usr/local/share/games/torcs/config/raceman/simplix-test.xml", shell = True)

	trackData = track.analyzeTrack(utils.torcsTrackDirectory + trackName + ".csv", folderName + "/track")

	if not os.path.exists(utils.torcsTrackDirectory + "/" + trackName + "_dynamics.csv"):
		subprocess.check_call(utils.torcsCommand + " -r /usr/local/share/games/torcs/config/raceman/simplix-test.xml", shell=True)
		if utils.torcsLogPath and utils.getLogList(utils.torcsLogPath):
			shutil.move(utils.torcsLogPath + utils.getLogList(utils.torcsLogPath)[-1], utils.torcsTrackDirectory + "/" + trackName + "_dynamics.csv")

	blocks.makeMetricsPlots(os.getcwd() + "/" + folderName, utils.torcsTrackDirectory, trackName, args.max_block_len)
	
	trackLength = utils.getTrackLength(trackName)

	utils.printHeading("Analyzing the dependency between staring and arrival orders")
	positionsVariations = positions.makePositionsVariationsPlotsFromLogList(os.getcwd() + "/" + folderName, logList, trackLength, 0, driversList, False, botSkills)

	utils.printHeading("Analyzing gaps distribution")
	gapsDistribution = gaps.makeGapsPlotsFromLogList(os.getcwd() + "/" + folderName, logList, driversList)

	utils.printHeading("Analyzing overtakes")
	overtakes.makeOvertakePlotsAndSegmentDataFile(os.getcwd() + "/" + folderName, logList, utils.torcsTrackDirectory, trackName, args.max_block_len, args.plot_cars)

	utils.printHeading("Analyzing start (after 30% of the first lap)")
	start30 = positions.makePositionsVariationsPlotsFromLogList(os.getcwd() + "/" + folderName, logList, trackLength, 0.3, driversList, False, botSkills)
	utils.printHeading("Analyzing start (after 50% of the first lap)")
	start50 = positions.makePositionsVariationsPlotsFromLogList(os.getcwd() + "/" + folderName, logList, trackLength, 0.5, driversList, False, botSkills)
	utils.printHeading("Analyzing start (after 100% of the first lap)")
	start100 = positions.makePositionsVariationsPlotsFromLogList(os.getcwd() + "/" + folderName, logList, trackLength, 1, driversList, False, botSkills)

	utils.printHeading("Collecting overall results")
	with open(os.getcwd() + "/" + folderName + "/" + trackName + "-results.csv", "w") as csvfile:
		writer = csv.writer(csvfile, delimiter = ";")
		writer.writerow(["Track", "Length", "Width",
				"LeftBendsCount", "RightBendsCount", "StraightsCount", "StraightsLength", "Radiuses mean", "Radiuses var", "Radiuses skew", "Heights mean", "Heights var", "Heights skew",
				"Length (1/3 lap)", "LeftBendsCount (1/3 lap)", "RightBendsCount (1/3 lap)", "StraightsCount (1/3 lap)", "StraightsLength (1/3 lap)", "Radiuses mean (1/3 lap)", "Radiuses var (1/3 lap)", "Radiuses skew (1/3 lap)", "Heights mean (1/3 lap)", "Heights var (1/3 lap)", "Heights skew (1/3 lap)",
				"Length (half lap)", "LeftBendsCount (half lap)", "RightBendsCount (half lap)", "StraightsCount (half lap)", "StraightsLength (half lap)", "Radiuses mean (half lap)", "Radiuses var (half lap)", "Radiuses skew (half lap)", "Heights mean (half lap)", "Heights var (half lap)", "Heights skew (half lap)",
				"Positions variations mean", "Positions variations var", "Positions variations skew",
				"Gaps mean", "Gaps var", "Gaps skew",
				"Positions variations (1/3 lap) mean", "Positions variations (1/3 lap) var", "Positions variations (1/3 lap) skew",
				"Positions variations (half lap) mean", "Positions variations (half lap) var", "Positions variations (half lap) skew",
				"Positions variations (1 lap) mean", "Positions variations (1 lap) var", "Positions variations (1 lap) skew"])
		writer.writerow([trackName] + trackData + positionsVariations + gapsDistribution + start30 + start50 + start100)
