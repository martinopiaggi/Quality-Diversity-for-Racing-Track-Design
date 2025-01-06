#!/usr/bin/env python

"""Analyzer of the data collected during the races"""
import math
import argparse
import csv
import json
import os
import numpy as np
import entropy
import logging
import blocks
import gaps
import overtakes
import positions
import track
import utils

logging.basicConfig(level=logging.INFO, 
                   format='%(asctime)s - %(levelname)s - %(message)s')

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("-B", "--max-block-len", type=int, default=200, 
                    help="the maximum block's length (used for the heatmaps and the segment data file, default: %(default)s)")
parser.add_argument("--json-output", action="store_true", 
                    help="Output raw metrics in JSON format")
parser.add_argument("paths", nargs="+", 
                    help="the list of paths which contain the simulation logs")
parser.add_argument("--no-plots", action="store_true", 
                    help="skip plot generation")
args = parser.parse_args()

for path in args.paths:
    folder_name = os.path.relpath(path).rstrip("/")
    print("\033[94m>>> Analyzing " + folder_name + "...\033[0m")
    
    utils.printHeading("Reading initialization data")

    log_list = utils.getLogList(folder_name)
    if not log_list:
        print(f"No logs found in {folder_name}, skipping...")
        continue

    try:
        drivers_list = utils.getDriversList(os.path.join(folder_name, log_list[0]))
    except Exception as e:
        print(f"Error reading drivers list: {e}")
        continue

    utils.printHeading("Analyzing track and track dynamics")
    # Get track data if available

    log_filename = os.path.basename(log_list[0])  # e.g., 'output_20230425.csv'
    track_name = log_filename.split("_")[0].split(".")[0] # 'output'

    track_path = os.path.join(utils.torcsTrackDirectory, f"{track_name}.csv")
    print(f"Track path: {track_path}")
    track_data = None
    if os.path.exists(track_path):
        try:
            track_data = track.analyzeTrack(
                track_path,
                os.path.join(folder_name, "track"),
                generate_plots=not args.no_plots
            )
        except Exception as e:
            print(f"Warning: Error analyzing track: {e}")
    else:
        print("Note: Track data not found. Continuing with limited analysis.")

    # Get block metrics if track data available
    block_data = None
    if track_data is not None:
        try:
            block_data = blocks.makeMetricsPlots(
                os.getcwd() + "/" + folder_name,
                utils.torcsTrackDirectory,
                track_name,
                args.max_block_len,
                generate_plots=not args.no_plots
            )
        except Exception as e:
            print(f"Warning: Error processing block metrics: {e}")

    # These analyses work without track data
    utils.printHeading("Analyzing positions and gaps")
    
    # Handle track length consistently
    track_length = None
    try:
        track_length = utils.getTrackLength(track_name)
    except Exception as e:
        print(f"Warning: Error getting track length, using default: {e}")
        track_length = 1000.0  # fallback value

    positions_variations = positions.makePositionsVariationsPlotsFromLogList(
        os.path.join(os.getcwd(), folder_name),
        log_list[0],  # Single log file
        track_length,
        0,            # lappercentage as float (0 means end of race)
        drivers_list,
        generate_plots=not args.no_plots
    )

    utils.printHeading("Analyzing gaps distribution")
    gaps_distribution = gaps.makeGapsPlotsFromLogList(
        os.getcwd() + "/" + folder_name,
        log_list,
        drivers_list,
        generate_plots=not args.no_plots
    )

    utils.printHeading("Analyzing overtakes")
    total_overtakes = overtakes.makeOvertakePlotsAndSegmentDataFile(
        os.getcwd() + "/" + folder_name,
        log_list,
        utils.torcsTrackDirectory,
        track_name,
        args.max_block_len,
        generate_plots=not args.no_plots
    )

    # Race progress analysis
    utils.printHeading("Analyzing race progress")
    start30 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folder_name, log_list[0], track_length, 0.3, drivers_list, not args.no_plots)
    start50 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folder_name, log_list[0], track_length, 0.5, drivers_list, not args.no_plots)
    start100 = positions.makePositionsVariationsPlotsFromLogList(
        os.getcwd() + "/" + folder_name, log_list[0], track_length, 1, drivers_list, not args.no_plots)

    def get_entropy_metrics(block_data):
        """Compute all entropy metrics with proper error handling."""
        metrics = {}
        
        if block_data is None:
            logging.warning("No block data available - skipping entropy metrics")
            return metrics
            
        entropy_functions = {
            'speed_entropy': entropy.compute_speed_entropy,
            'curvature_entropy': entropy.compute_curvature_entropy,
            'acceleration_entropy': entropy.compute_acceleration_entropy,
            'braking_entropy': entropy.compute_braking_entropy
        }
        
        for metric_name, entropy_func in entropy_functions.items():
            try:
                value = entropy_func(block_data)
                metrics[metric_name] = value if value is not None else 0.0
            except Exception as e:
                logging.error(f"Error computing {metric_name}: {str(e)}")
                metrics[metric_name] = 0.0
                
        return metrics
    
    # JSON output with available metrics
    if args.json_output:
        raw_metrics = {
            'positions_mean': positions_variations[0] if positions_variations else 0,
            'positions_var': positions_variations[1] if len(positions_variations) > 1 else 0,
            'gaps_mean': gaps_distribution[0] if gaps_distribution else 0,
            'gaps_var': gaps_distribution[1] if len(gaps_distribution) > 1 else 0,
            'total_overtakes': total_overtakes
        }

        # Add track metrics if available
        if track_data is not None:
            raw_metrics.update({
                'left_bends': track_data[2],
                'right_bends': track_data[3],
                'straight_sections': track_data[4],
                'avg_radius_mean': np.mean(track_data[6]) if len(track_data) > 6 and track_data[6] else 0,
                'avg_radius_var': np.var(track_data[6]) if len(track_data) > 6 and track_data[6] else 0,
            })
            
            # Add entropy metrics
            entropy_metrics = get_entropy_metrics(block_data)
            raw_metrics.update(entropy_metrics)
            
        print("===JSON_START===")
        print(json.dumps(raw_metrics, indent=2))
        print("===JSON_END===")
