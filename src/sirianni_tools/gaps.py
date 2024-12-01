"""Functions to make gaps distribution plots."""

import math
import os
import signal
import subprocess

import matplotlib.pyplot as plt
import numpy as np
import pylab
import scipy.stats

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"

MAX_GAP_TIME = 120 # Maximum realistic gap in seconds


def plotGaps(cumulative, gaps, filename):
    # The first player has gap 0 from himself, do not consider it
    # gaps.remove(0) returns an error if 0 is not in the list, so I use a list comprehension
    gaps = [x for x in gaps if x != 0]

    gapsUpperbound = math.ceil(max(gaps))
    
    histPlot = pylab.subplot(1,1,1)
    
    params = scipy.stats.norm.fit(gaps)
    x = np.linspace(0, gapsUpperbound, 100)
    
    if cumulative:
        n, bins, patches = plt.hist(gaps, gapsUpperbound, (0, gapsUpperbound), cumulative = cumulative, histtype = "stepfilled", color = "green")
        density = scipy.stats.norm.cdf(x, *params)
        
        plt.xlabel("Cumulative gaps [s]")
        plt.xticks([t for t in range(len(n)) if n[t] != n[t-1]])
    else:
        n, bins, patches = plt.hist(gaps, gapsUpperbound, (0, gapsUpperbound), cumulative = cumulative, color = "green")
        density = scipy.stats.norm.pdf(x, *params)
        
        plt.xlabel("Gaps [s]")

    plt.ylabel("Number of drivers")
    # Duplicated numbers look like bold (written multiple times), set() does not allow duplicates
    # 0 is added to the list in order to be visible also in the cumulative plot
    # The internal list() should not be necessary, but without it the concatenation does not work in this specific case (I do not know why)
    plt.yticks(list(set(list(n) + [0.0])))
    plt.grid(True, "major")
    
    densityPlot = histPlot.twinx()
    densityPlot.set_ylabel("Density (gaussian distribution) [1]")
    plt.plot(x, density, "b-", linewidth = 2)
    
    mean, var = scipy.stats.norm.stats(*params, moments = "mv")
    skew = scipy.stats.skew(gaps)
    
    line = plt.Line2D(range(10), range(10), color = "blue", linewidth = 2)
    legend = plt.legend([line], ["mean = " + str(mean) + "\nvar = " + str(var) + "\nskew = " + str(skew)], loc = 8, bbox_to_anchor=(0.5, -0.3))
    
    plt.savefig(filename, bbox_extra_artists = (legend,), bbox_inches = "tight")
    print("  -> Created " + filename.split("/")[-2] + "/" + filename.split("/")[-1])
    plt.clf()

    return [mean, var, skew]


def makeGapsPlotsFromLogList(folder, logList, driverList, generate_plots=True):
    gapsDirectory = os.path.join(folder, "gaps")
    cumulativeGapsDirectory = os.path.join(folder, "cumulative-gaps")
    os.makedirs(gapsDirectory, exist_ok=True)
    os.makedirs(cumulativeGapsDirectory, exist_ok=True)

    allGaps = np.array([])

    for log in logList:
        print(f"==> Analyzing {log}")
        gaps = np.array([])
        for driver in driverList:
            cmd = f'tac "{os.path.join(folder, log)}" | grep -v "overtake" | grep "^[^,]*,{driver}," -m 1'
            try:
                output = subprocess.check_output(cmd, shell=True).decode('ascii').strip()
                if output:
                    data = output.split(',')
                    if len(data) >= 16:
                        time_ahead = float(data[15])  # Using time to car ahead
                        if 0 < time_ahead < MAX_GAP_TIME:
                            gaps = np.append(gaps, time_ahead)
            except Exception as e:
                print(f"Warning: Could not get gap for {driver}: {e}")
                continue

        if len(gaps) > 0:
            if generate_plots:
                gapsDistribution = plotGaps(False, gaps, os.path.join(gapsDirectory, f"{log}.svg"))
                plotGaps(True, gaps, os.path.join(cumulativeGapsDirectory, f"{log}.svg"))
            allGaps = np.append(allGaps, gaps)
        else:
            print(f"No valid gaps found in {log}")


    # Analyze all gaps together
    if len(allGaps) > 0:
        print("==> Analyzing gaps across all logs")
        gapsDistribution = plotGaps(False, allGaps, os.path.join(gapsDirectory, "gaps.svg"))
        plotGaps(True, allGaps, os.path.join(cumulativeGapsDirectory, "gaps.svg"))
        return gapsDistribution
    else:
        print("No valid gaps found across all logs")
        return [0, 0, 0]
