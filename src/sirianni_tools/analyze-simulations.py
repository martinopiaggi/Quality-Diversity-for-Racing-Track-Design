#!/usr/bin/env python

"""Analyzer of the data collected during the races"""

import argparse
import csv
import json  # Added missing import
import os
import subprocess
import shutil
import numpy as np
import scipy.stats

import blocks
import gaps
import overtakes
import positions
import track
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"

parser = argparse.ArgumentParser(description = __doc__)
parser.add_argument("-B", "--max-block-len", type=int, default=200, 
                    help="the maximum block's length (used for the heatmaps and the segment data file, default: %(default)s)")
parser.add_argument("-c", "--plot-cars", action="store_true", 
                    help="make additional overtakes plots with cars positions (default: %(default)s)")
parser.add_argument("--json-output", action="store_true", 
                    help="Output raw metrics in JSON format")
parser.add_argument("paths", nargs="+", 
                    help="the list of paths which contain the simulation logs")
parser.add_argument("--no-plots", action="store_true", 
                    help="Skip plot generation")

args = parser.parse_args()

for path in args.paths:
    folderName = os.path.relpath(path).rstrip("/")
    print("\033[94m>>> Analyzing " + folderName + "...\033[0m")
    
    utils.printHeading("Reading initialization data")

    # Fixed hardcoded configuration
    botSkills = [
        ("damned", "medium"),
        ("bt", "medium"),
        ("tita", "medium"),
        ("olethros", "medium"),
        ("simplix", "medium")
    ]

    logList = utils.getLogList(folderName)
    if not logList:
        print(f"No logs found in {folderName}, skipping...")
        continue

    try:
        driversList = utils.getDriversList(os.path.join(folderName, logList[0]))
    except Exception as e:
        print(f"Error reading drivers list: {e}")
        continue

    utils.printHeading("Analyzing track and track dynamics")
    trackName = logList[0][20:-4]
    
    # Ensure track data exists
    track_path = os.path.join(utils.torcsTrackDirectory, f"{trackName}.csv")
    if not os.path.exists(track_path):
        try:
            subprocess.check_call(f"{utils.torcsCommand} -r /usr/local/share/games/torcs/config/raceman/simplix-test.xml", 
                                shell=True)
        except subprocess.CalledProcessError as e:
            print(f"Error generating track data: {e}")
            continue

    # Get track data
    try:
        # Existing code
        trackData = track.analyzeTrack(
            track_path,
            os.path.join(folderName, "track"),
            generate_plots=not args.no_plots  # Respect the --no-plots flag
        )
    except Exception as e:
        print(f"Error analyzing track: {e}")
        trackData = None

    # Handle dynamics data
    dynamics_path = os.path.join(utils.torcsTrackDirectory, f"{trackName}_dynamics.csv")
    if not os.path.exists(dynamics_path):
        try:
            subprocess.check_call(f"{utils.torcsCommand} -r /usr/local/share/games/torcs/config/raceman/simplix-test.xml", 
                                shell=True)
            if utils.torcsLogPath and utils.getLogList(utils.torcsLogPath):
                latest_log = utils.getLogList(utils.torcsLogPath)[-1]
                shutil.move(os.path.join(utils.torcsLogPath, latest_log), dynamics_path)
        except Exception as e:
            print(f"Error handling dynamics data: {e}")

    # Process blocks
    blocks.makeMetricsPlots(
        os.getcwd() + "/" + folderName,
        utils.torcsTrackDirectory,
        trackName,
        args.max_block_len,
        generate_plots=not args.no_plots
    )

    try:
        trackLength = utils.getTrackLength(trackName)
    except Exception as e:
        print(f"Error getting track length: {e}")
        continue

    utils.printHeading("Analyzing the dependency between starting and arrival orders")
    positionsVariations = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName,
        logList,
        trackLength,
        0,
        driversList,
        False,
        botSkills,
        generate_plots=not args.no_plots
    )


    utils.printHeading("Analyzing gaps distribution")
    gapsDistribution = gaps.makeGapsPlotsFromLogList(
        os.getcwd() + "/" + folderName,
        logList,
        driversList,
        generate_plots=not args.no_plots
    )


    utils.printHeading("Analyzing overtakes")
    totalOvertakes = overtakes.makeOvertakePlotsAndSegmentDataFile(
        os.getcwd() + "/" + folderName,
        logList,
        utils.torcsTrackDirectory,
        trackName,
        args.max_block_len,
        args.plot_cars
    )

    utils.printHeading("Analyzing race progress")
    start30 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, trackLength, 0.3, driversList, False, botSkills)
    start50 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, trackLength, 0.5, driversList, False, botSkills)
    start100 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, trackLength, 1, driversList, False, botSkills)

    utils.printHeading("Collecting overall results")
    results_path = os.path.join(os.getcwd(), folderName, f"{trackName}-results.csv")
    with open(results_path, "w", newline='') as csvfile:
        writer = csv.writer(csvfile, delimiter=";")
        # Headers remain the same as in your original code
        writer.writerow(["Track", "Length", "Width",
                "LeftBendsCount", "RightBendsCount", "StraightsCount", "StraightsLength",
                "Radiuses mean", "Radiuses var", "Radiuses skew",
                "Heights mean", "Heights var", "Heights skew",
                "Length (1/3 lap)", "LeftBendsCount (1/3 lap)", "RightBendsCount (1/3 lap)",
                "StraightsCount (1/3 lap)", "StraightsLength (1/3 lap)",
                "Radiuses mean (1/3 lap)", "Radiuses var (1/3 lap)", "Radiuses skew (1/3 lap)",
                "Heights mean (1/3 lap)", "Heights var (1/3 lap)", "Heights skew (1/3 lap)",
                "Length (half lap)", "LeftBendsCount (half lap)", "RightBendsCount (half lap)",
                "StraightsCount (half lap)", "StraightsLength (half lap)",
                "Radiuses mean (half lap)", "Radiuses var (half lap)", "Radiuses skew (half lap)",
                "Heights mean (half lap)", "Heights var (half lap)", "Heights skew (half lap)",
                "Positions variations mean", "Positions variations var", "Positions variations skew",
                "Gaps mean", "Gaps var", "Gaps skew",
                "Positions variations (1/3 lap) mean", "Positions variations (1/3 lap) var",
                "Positions variations (1/3 lap) skew",
                "Positions variations (half lap) mean", "Positions variations (half lap) var",
                "Positions variations (half lap) skew",
                "Positions variations (1 lap) mean", "Positions variations (1 lap) var",
                "Positions variations (1 lap) skew"])

        # Handle None values
        if trackData is None:
            trackData = [0] * 35
        if positionsVariations is None:
            positionsVariations = [0, 0, 0]
        if gapsDistribution is None:
            gapsDistribution = [0, 0, 0]
        if start30 is None:
            start30 = [0, 0, 0]
        if start50 is None:
            start50 = [0, 0, 0]
        if start100 is None:
            start100 = [0, 0, 0]

        writer.writerow([trackName] + trackData + positionsVariations + 
                       gapsDistribution + start30 + start50 + start100)

    
    if args.json_output:
        radiuses = trackData[6] if trackData and len(trackData) > 6 else []
        if radiuses:
            avg_radius_mean = np.mean(radiuses)
            avg_radius_var = np.var(radiuses)
        else:
            avg_radius_mean = avg_radius_var = 0
        raw_metrics = {
            'track_length': trackLength,
            'left_bends': trackData[2] if trackData else 0,
            'right_bends': trackData[3] if trackData else 0,
            'straight_sections': trackData[4] if trackData else 0,
            'avg_radius_mean': avg_radius_mean,
            'avg_radius_var': avg_radius_var,
            'gaps_mean': gapsDistribution[0] if gapsDistribution else 0,
            'gaps_var': gapsDistribution[1] if len(gapsDistribution) > 1 else 0,
            'positions_mean': positionsVariations[0] if positionsVariations else 0,
            'positions_var': positionsVariations[1] if len(positionsVariations) > 1 else 0,
            'total_overtakes': totalOvertakes
        }
        print(json.dumps(raw_metrics, indent=2))