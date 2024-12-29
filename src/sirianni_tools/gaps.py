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

def fillGaps(logFile):
    """
    Similar to fillOvertakes in overtake.py:
    - Read lines from logFile,
    - If line starts with 'timeBehindPrev,DriverName,<gap>',
      parse that gap and store it in a dictionary keyed by driverName
    - Return the dictionary of final gaps
    """

    gaps_dict = {}  # e.g., {'driverName': 12.058, ...}
    try:
        with open(logFile, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith("timeBehindPrev,"):
                    # Example line: "timeBehindPrev,tita 4,0"
                    parts = line.split(',')
                    if len(parts) >= 3:
                        driver_name = parts[1].strip()
                        try:
                            gap_val = float(parts[2].strip())
                            # Only keep if gap_val is > 0 and < MAX_GAP_TIME
                            if 0 < gap_val < MAX_GAP_TIME:
                                gaps_dict[driver_name] = gap_val
                        except ValueError:
                            print(f"Warning: invalid gap value in line: {line}")
                    else:
                        print(f"Warning: not enough fields in line: {line}")
    except Exception as e:
        print(f"Error reading logFile {logFile}: {e}")

    return gaps_dict

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
    """
    Rewritten in the "style" of overtake.py:
    - For each log, call fillGaps(logFile)
    - Accumulate the final gaps in an array
    - Then plot them
    """

    gapsDirectory = os.path.join(folder, "gaps")
    cumulativeGapsDirectory = os.path.join(folder, "cumulative-gaps")
    os.makedirs(gapsDirectory, exist_ok=True)
    os.makedirs(cumulativeGapsDirectory, exist_ok=True)

    all_gaps = []

    for log in logList:
        print(f"==> Analyzing {log}")
        logFilePath = os.path.join(folder, log)

        # Get final gap lines from fillGaps
        final_gaps_for_log = fillGaps(logFilePath)  # returns dict {driverName: gapVal}
        if not final_gaps_for_log:
            print(f"No final gap lines found in {log}")
            continue

        # Filter only drivers you care about, e.g. driverList
        # or accept all
        single_log_gaps = []
        for driver_name, gap_val in final_gaps_for_log.items():
            # If you only want specific drivers:
            if driver_name in driverList:
                single_log_gaps.append(gap_val)

        single_log_gaps = [g for g in single_log_gaps if 0 < g < MAX_GAP_TIME]
        if len(single_log_gaps) == 0:
            print(f"No valid (nonzero) final gaps found in {log}")
            continue

        # Plot per-log
        single_log_gaps = np.array(single_log_gaps, dtype=float)
        all_gaps.extend(single_log_gaps)
        
        if generate_plots:
            # Plot normal histogram
            outPath = os.path.join(gapsDirectory, f"{log}.svg")
            plotGaps(False, single_log_gaps, outPath)
            # Plot cumulative
            outPath2 = os.path.join(cumulativeGapsDirectory, f"{log}.svg")
            plotGaps(True, single_log_gaps, outPath2)

    # End for each log
    if len(all_gaps) == 0:
        print("No final gaps found across all logs")
        return [0,0,0]
    
    # Otherwise, analyze all together
    print("==> Analyzing final gaps across all logs")
    all_gaps = np.array(all_gaps, dtype=float)
    outPath = os.path.join(gapsDirectory, "all_gaps.svg")
    mean_var_skew = plotGaps(False, all_gaps, outPath)
    outPath2 = os.path.join(cumulativeGapsDirectory, "all_gaps.svg")
    plotGaps(True, all_gaps, outPath2)
    return mean_var_skew
