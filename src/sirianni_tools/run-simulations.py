#!/usr/bin/env python

"""Run track export or race simulations"""

import argparse
import os
import shutil
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
    racegen.generate_race_xml(race_config,num_laps)
    
    cmd = f"{utils.torcsCommand} -r {os.path.join(os.getcwd(), race_config)}"
    subprocess.check_call(cmd, shell=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-r", "--num_laps", type=int, default=10,
                        help="Number of laps to simulate (default: 10)")
    parser.add_argument("--track-export", action="store_true",
                        help="Run track export only")
    
    args = parser.parse_args()
    
    try:
        if args.track_export:
            run_track_export(utils.torcsRacemanDirectory)
        else:
            run_race_simulation(utils.torcsRacemanDirectory, args.num_laps)
    except subprocess.CalledProcessError as e:
        print(f"Error running TORCS: {e}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        print("Done!")


if __name__ == "__main__":
    main()
