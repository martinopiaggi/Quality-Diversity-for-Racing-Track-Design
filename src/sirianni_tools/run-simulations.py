#!/usr/bin/env python

"""Run track export or race simulations"""

import argparse
import os
import sys
import subprocess
import time

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
    print("==> Running race simulation...")
    race_config = os.path.join(folder_name, "race_sim.xml")
    
    # Generate config with all racing bots
    racegen.generate_race_xml(race_config, num_laps)
    
    cmd = f"{utils.torcsCommand} -r {os.path.join(os.getcwd(), race_config)}"
    subprocess.check_call(cmd, shell=True)
    print("Race simulation completed.")
    
def run_analysis(folder_name, json_output=True):
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
    if json_output:
        cmd.append("--json-output")
    # We assume logs are in utils.torcsLogPath, e.g. /root/.torcs/logs
    cmd.append(utils.torcsLogPath)

    # Capture output
    result = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    decoded = result.decode("utf-8", errors="ignore")
    print("Analysis completed.")
    return decoded

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-r", "--num_laps", type=int, default=0,
                        help="number of laps to simulate (default: 0 means no race).")
    parser.add_argument("--track-export", action="store_true",
                        help="run single-lap track export (with trackexporter bot).")
    parser.add_argument("--json", action="store_true",
                        help="run analysis afterwards and print JSON to stdout.")
    args = parser.parse_args()

    # The usual torcs raceman directory from utils:
    folder_name = utils.torcsRacemanDirectory

    # 1) Possibly do the track export
    if args.track_export:
        try:
            run_track_export(folder_name)
        except subprocess.CalledProcessError as e:
            print(f"Error running track export: {e}")
            sys.exit(1)
    else:
    # 2) Possibly do the real race if requested
    if args.num_laps > 0:
        try:
            run_race_simulation(folder_name, args.num_laps)
        except subprocess.CalledProcessError as e:
            print(f"Error running race simulation: {e}")
            sys.exit(1)

    # 3) Possibly do analysis in JSON mode
    if args.json:
        try:
            analysis_output = run_analysis(folder_name, json_output=True)
            print(analysis_output)  # So the caller can capture / parse it
        except subprocess.CalledProcessError as e:
            print(f"Error running analysis: {e}")
            sys.exit(1)

if __name__ == "__main__":
    main()
