#!/usr/bin/env python

"""Analyzer of the data collected during the races"""

import argparse
import csv
import json
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
    # Get track data if available
    trackName = logList[0][20:-4]
    track_path = os.path.join(utils.torcsTrackDirectory, f"{trackName}.csv")
    
    trackData = None
    if os.path.exists(track_path):
        try:
            trackData = track.analyzeTrack(
                track_path,
                os.path.join(folderName, "track"),
                generate_plots=not args.no_plots
            )
        except Exception as e:
            print(f"Warning: Error analyzing track: {e}")
    else:
        print("Note: Track data not found. Continuing with limited analysis.")

    # Get block metrics if track data available
    blockData = None
    if trackData is not None:
        try:
            blockData = blocks.makeMetricsPlots(
                os.getcwd() + "/" + folderName,
                utils.torcsTrackDirectory,
                trackName,
                args.max_block_len,
                generate_plots=not args.no_plots
            )
        except Exception as e:
            print(f"Warning: Error processing block metrics: {e}")

    # These analyses work without track data
    utils.printHeading("Analyzing positions and gaps")
    try:
        trackLength = utils.getTrackLength(trackName) if os.path.exists(track_path) else 1000  # Default length if no track data
    except Exception as e:
        print(f"Warning: Error getting track length: {e}")
        trackLength = 1000  # Default length

    positionsVariations = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName,
        logList,
        0,
        driversList,
        False,
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

    # Race progress analysis
    utils.printHeading("Analyzing race progress")
    start30 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, 0.3, driversList, False, not args.no_plots)
    start50 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, 0.5, driversList, False, not args.no_plots)
    start100 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folderName, logList, 1, driversList, False, not args.no_plots)

    utils.printHeading("Collecting overall results")
    results_path = os.path.join(os.getcwd(), folderName, f"{trackName}-results.csv")
    with open(results_path, "w", newline='') as csvfile:
        writer = csv.writer(csvfile, delimiter=";")
        # Keep all original headers
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

    # JSON output with available metrics
    if args.json_output:
        raw_metrics = {
            'track_length': trackLength,
            'positions_mean': positionsVariations[0] if positionsVariations else 0,
            'positions_var': positionsVariations[1] if len(positionsVariations) > 1 else 0,
            'gaps_mean': gapsDistribution[0] if gapsDistribution else 0,
            'gaps_var': gapsDistribution[1] if len(gapsDistribution) > 1 else 0,
            'total_overtakes': totalOvertakes
        }

        # Add track metrics if available
        if trackData:
            raw_metrics.update({
                'left_bends': trackData[2],
                'right_bends': trackData[3],
                'straight_sections': trackData[4],
                'avg_radius_mean': np.mean(trackData[6]) if len(trackData) > 6 and trackData[6] else 0,
                'avg_radius_var': np.var(trackData[6]) if len(trackData) > 6 and trackData[6] else 0,
            })
            
        print(json.dumps(raw_metrics, indent=2))