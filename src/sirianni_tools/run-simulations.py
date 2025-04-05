#!/usr/bin/env python

"""Run track export or race simulations multiple times, then average analyses."""

import argparse
import os
import sys
import subprocess
import math
import json

import racegen
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"


def run_track_export(folder_name):
    """Run a single lap with only trackexporter."""
    print("==> Running track export...")
    race_config = os.path.join(folder_name, "track_export.xml")
    
    # Generate config with only trackexporter
    racegen.generate_track_export_xml(race_config)
    
    cmd = f"{utils.torcsCommand} -r {os.path.join(os.getcwd(), race_config)}"
    subprocess.check_call(cmd, shell=True)
    os.remove(race_config)


def run_race_simulation(folder_name, num_laps):
    """Run main race simulation with all bots."""
    print(f"==> Running race simulation for {num_laps} laps...")
    race_config = os.path.join(folder_name, "race_sim.xml")
    
    # Generate config with all racing bots
    racegen.generate_race_xml(race_config, num_laps)
    
    cmd = f"{utils.torcsCommand} -r {os.path.join(os.getcwd(), race_config)}"
    subprocess.check_call(cmd, shell=True)
    print("Race simulation completed.")
    
def run_analysis(no_plots=True, json_output=True):
    """
    Run analysis script on the logs, returning output as string.
    By default uses JSON output from analyze-simulations.py.
    """
    print("==> Analyzing logs...")
    cmd = [
        "python3",
        "/usr/local/lib/sirianni_tools/analyze-simulations.py",
        "-B", "200"
    ]

    if no_plots:
        cmd.append("--no-plots")

    if json_output:
        cmd.append("--json-output")
    # We assume logs are in utils.torcsLogPath, e.g. /root/.torcs/logs
    cmd.append(utils.torcsLogPath)

    # Capture output
    result = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    decoded = result.decode("utf-8", errors="ignore")
    print("Analysis completed.")
    return decoded


def parse_analysis_json(analysis_output):
    """
    Extract the JSON from the '===JSON_START===' / '===JSON_END===' markers
    and parse it into a dictionary.
    """
    start_marker = "===JSON_START==="
    end_marker = "===JSON_END==="
    start_idx = analysis_output.find(start_marker)
    end_idx = analysis_output.find(end_marker, start_idx)
    if start_idx == -1 or end_idx == -1:
        print("Warning: JSON markers not found in analysis output.")
        return {}
    json_str = analysis_output[start_idx + len(start_marker):end_idx].strip()
    try:
        data = json.loads(json_str)
        return data
    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}")
        return {}


def average_metrics(all_results):
    """
    Given a list of dictionaries with numeric values,
    compute the average for each key across all dicts,
    while ignoring NaN or infinite values.
    """
    if not all_results:
        return {}

    # Collect all unique metric keys
    keys = set()
    for d in all_results:
        keys.update(d.keys())

    averages = {}

    for key in keys:
        total = 0.0
        valid_count = 0
        for d in all_results:
            val = d.get(key, None)
            if isinstance(val, (int, float)):
                # Check if val is finite (not NaN or inf)
                if not math.isnan(val) and math.isfinite(val):
                    total += val
                    valid_count += 1
                # else: we skip NaN/inf
        # Only compute average if we had at least one valid numeric value
        if valid_count > 0:
            averages[key] = total / valid_count
        else:
            averages[key] = 0.0 

    return averages


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-r", "--num_laps", type=int, default=0,
                        help="number of laps to simulate (default: 0 means no race).")
    parser.add_argument("--track-export", action="store_true",
                        help="run single-lap track export (with trackexporter bot).")
    parser.add_argument("--json", action="store_true",
                        help="run analysis afterwards and print JSON to stdout.")
    parser.add_argument("--plots", action="store_true", help="enable plots (default: no plots)")
    parser.add_argument("--repetitions", type=int, default=1,
                        help="Number of times to run the race+analysis (default=1).")
    args = parser.parse_args()

    # The usual torcs raceman directory from utils:
    folder_name = utils.torcsRacemanDirectory

    # 1) Possibly do track export first
    if args.track_export:
        try:
            run_track_export(folder_name)
        except subprocess.CalledProcessError as e:
            print(f"Error running track export: {e}")
            sys.exit(1)
    
    # 2) run multiple races and accumulate the results
    aggregated_results = []
    for i in range(args.repetitions):
        print(f"\n=== Simulation iteration {i+1}/{args.repetitions} ===")
        try:
            run_race_simulation(folder_name, args.num_laps)
        except subprocess.CalledProcessError as e:
            print(f"Error running race simulation: {e}")
            sys.exit(1)

        # Possibly do analysis
        if args.json:
            try:
                analysis_output = run_analysis(
                    no_plots=(not args.plots),
                    json_output=True
                )
                # Parse the JSON from the output
                parsed = parse_analysis_json(analysis_output)
                if parsed:
                    aggregated_results.append(parsed)
            except subprocess.CalledProcessError as e:
                print(f"Error running analysis: {e}")
                sys.exit(1)

    # 3) After finishing all races+analyses, average the results
    if aggregated_results:
        avg_res = average_metrics(aggregated_results)
        # Print final JSON
        final_json = json.dumps(avg_res, indent=2)
        print("===FINAL_JSON_START===")
        print(final_json)
        print("===FINAL_JSON_END===")
    else:
        print("Warning: No valid analysis results to average.")
    
    print("All repetitions done! Exiting now.")

if __name__ == "__main__":
    main()
