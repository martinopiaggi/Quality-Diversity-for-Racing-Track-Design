#!/usr/bin/env python

"""Creator of the blocks data file without overtakes for the overtakes data-mining analysis."""

import argparse
import os
import shutil
import subprocess

import overtakes
import racegen
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



parser = argparse.ArgumentParser(description = __doc__)

parser.add_argument("-t", "--track", required = True, help = "the name of the track used for the simulation (the same as the Torcs filenames, e.g. forza and not Forza)")
parser.add_argument("-B", "--max-block-len", type = int, default = 200, help = "the maximum block's length (default: %(default)s)")

args = parser.parse_args()



print("==> Checking blocks folder existence")
blocksFolder = "blocks"
if not os.path.exists(blocksFolder):
    os.makedirs(blocksFolder)
    print("  -> Created directory " + blocksFolder)

print("==> Analyzing track")
if not os.path.exists(utils.torcsTrackDirectory + args.track + ".csv"):
    # Export track data using trackexporter
    racegen.racegen(args.track, racegen.trackexporterBotList, 1, "quickrace.xml")
    subprocess.check_call(utils.torcsCommand + " -r " + os.getcwd() + "/quickrace.xml", shell = True)
    os.remove("quickrace.xml")
    os.remove(utils.torcsLogPath + utils.getLogList(utils.torcsLogPath)[-1])

print("==> Analyzing track dynamics")
if not os.path.exists(utils.torcsTrackDirectory + "/" + args.track + "_dynamics.csv"):
    racegen.racegen(args.track, racegen.trackLogger, 1, "quickrace.xml")
    subprocess.check_call(utils.torcsCommand + " -r " + os.getcwd() + "/" + "/quickrace.xml", shell=True)
    os.remove("quickrace.xml")
    shutil.move(utils.torcsLogPath + utils.getLogList(utils.torcsLogPath)[-1], utils.torcsTrackDirectory + "/" + args.track + "_dynamics.csv")

overtakes.makeBlocksDataWithoutOvertakes(blocksFolder, utils.torcsTrackDirectory, args.track, args.max_block_len)
