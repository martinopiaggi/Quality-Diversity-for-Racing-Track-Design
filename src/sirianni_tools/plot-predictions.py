#!/usr/bin/env python

"""Plotter of the data mining results. It contains much hard-coded stuff."""

import argparse
import csv

import blocks
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



parser = argparse.ArgumentParser(description = __doc__)
parser.parse_args()

maxBlockLength = 200;
trackDirectory = utils.torcsTrackDirectory;
trackName = "wheel-2";

def readRes():
    with open("/home/jacopo/tesi/risultati-dm/risultati.csv", "r") as csvfile:
        firstRow = True
        res = []

        for row in csv.reader(csvfile):
            if firstRow:
                firstRow = False;
            else:
                if row[0] == "1":
                    res.append(1);
                else:
                    res.append(2);
        return res;



def readOvertakes():
    with open("/home/jacopo/.RapidMiner/repositories/Local Repository/data/blocks/test/wheel-2-blocks-data.csv", "r") as csvfile:
        firstRow = True
        res = []

        for row in csv.reader(csvfile, delimiter=';'):
            if firstRow:
                firstRow = False;
            else:
                res.append(int(row[4]));
        return res;
                


trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)

blocks.plotMetric(trackData, readRes(), "",  "/home/jacopo/tesi/risultati-dm/prova.svg")
#blocks.plotMetric(trackData, readOvertakes(), "Numero di sorpassi",  "/home/jacopo/tesi/risultati-dm/prova.svg")
