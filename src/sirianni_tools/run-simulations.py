#!/usr/bin/env python

"""Run a sequence of simulated races"""

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

parser = argparse.ArgumentParser(description=__doc__)

parser.add_argument("-r", "--num-races", type=int, required=True, help="the number of races to simulate")
parser.add_argument("-l", "--num-laps", type=int, required=True, help="the number of laps of each simulation")
parser.add_argument("-b", "--num-bots", type=int, required=True, help="the number of bots to use for the simulation")
parser.add_argument("-t", "--track-type", default="road", help="the type of tracks (default is road)")
parser.add_argument("tracks", nargs="+",
                    help="the name of the tracks used for the simulations (e.g. forza)")

args = parser.parse_args()

for track in args.tracks:
    utils.printHeading("Setting up the simulation")

    print("==> Generating simulation folder")
    # Torcs does not find the race xml file if whitespaces are used
    toolFolderName = time.strftime("%Y%m%d-%H%M%S-") + track
    os.makedirs(toolFolderName)
    print("  -> Created directory " + toolFolderName)

    # Remove the entire bot skill determination logic:
    # No skill measurements, no skills.csv, no sorting by lap times.

    print("==> Generating race setup")
    racegen.racegen(track, args.track_type, racegen.defaultBotList[0:args.num_bots], args.num_laps, toolFolderName + "/quickrace.xml")
    print("  -> Generated quickrace.xml")

    utils.printHeading("Running the simulation")

    print("==> Executing TORCS...")
    for i in range(1, args.num_races + 1):
        print("  -> Race " + str(i) + "/" + str(args.num_races))
        subprocess.check_call(utils.torcsCommand + " -r " + os.getcwd() + "/" + toolFolderName + "/quickrace.xml",
                              shell=True)

    os.remove(toolFolderName + "/quickrace.xml")

    print("==> Collecting the simulation results...")
    if len(os.listdir(utils.torcsLogPath)) - 1 > args.num_races:
        print("WARNING: log folder is not empty!")
    for index, log in enumerate(os.listdir(utils.torcsLogPath)):
        shutil.move(utils.torcsLogPath + log, toolFolderName)
        print("  -> Moved " + log + " (" + str(index + 1) + "/" + str(args.num_races) + ")")
