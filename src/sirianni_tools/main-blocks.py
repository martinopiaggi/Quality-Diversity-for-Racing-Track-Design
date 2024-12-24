#!/usr/bin/env python

"""Creator of the blocks data file without overtakes for the overtakes data-mining analysis."""

import argparse
import os
import overtakes
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

print("==> Overtakes data-mining analysis")

overtakes.makeBlocksDataWithoutOvertakes(blocksFolder, utils.torcsTrackDirectory, args.track, args.max_block_len)
