"""Functions to make gaps distribution plots."""

import math
import os
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
    Read lines from logFile and extract gap times.
    Returns a dictionary of gaps keyed by driver name.
    """
    gaps_dict = {}
    try:
        with open(logFile, 'r') as f:
            for line in f:
                if line.startswith("timeBehindPrev,"):
                    parts = line.strip().split(',')
                    if len(parts) >= 3:
                        driver_name = parts[1].strip()
                        try:
                            gap_val = float(parts[2].strip())
                            # Only keep if gap_val is > 0 and < MAX_GAP_TIME
                            if 0 <= gap_val < MAX_GAP_TIME:
                                gaps_dict[driver_name] = gap_val
                        except ValueError:
                            print(f"Warning: invalid gap value in line: {line}")
    except Exception as e:
        print(f"Error reading logFile {logFile}: {e}")

    return gaps_dict

def plotGaps(cumulative, gaps, filename):
    """
    Create histogram and distribution plots of gap times.
    Returns [mean, variance, skewness] of the data.
    """
    # Remove gaps of 0 as they represent the lead driver
    gaps = [x for x in gaps if x != 0]
    
    # Handle insufficient data points
    if len(gaps) < 2:
        print("Not enough data points for distribution fitting")
        return [0, 0, 0]

    gapsUpperbound = math.ceil(max(gaps))
    histPlot = pylab.subplot(1,1,1)
    
    # Create histogram
    if cumulative:
        n, bins, patches = plt.hist(gaps, gapsUpperbound, (0, gapsUpperbound),
                                  cumulative=True, histtype="stepfilled",
                                  color="green")
        plt.xlabel("Cumulative gaps [s]")
        plt.xticks([t for t in range(len(n)) if n[t] != n[t-1]])
    else:
        n, bins, patches = plt.hist(gaps, gapsUpperbound, (0, gapsUpperbound),
                                  cumulative=False, color="green")
        plt.xlabel("Gaps [s]")

    # Fit normal distribution
    params = scipy.stats.norm.fit(gaps)
    x = np.linspace(0, gapsUpperbound, 100)
    
    # Plot density
    densityPlot = histPlot.twinx()
    densityPlot.set_ylabel("Density (gaussian distribution) [1]")
    density = scipy.stats.norm.cdf(x, *params) if cumulative else scipy.stats.norm.pdf(x, *params)
    plt.plot(x, density, "b-", linewidth=2)
    
    # Calculate statistics
    mean, var = scipy.stats.norm.stats(*params, moments="mv")
    skew = scipy.stats.skew(gaps)
    
    # Add legend with statistics
    line = plt.Line2D(range(10), range(10), color="blue", linewidth=2)
    legend = plt.legend([line],
                       [f"mean = {mean:.3f}\nvar = {var:.3f}\nskew = {skew:.3f}"],
                       loc=8, bbox_to_anchor=(0.5, -0.3))
    
    # Finalize plot
    plt.ylabel("Number of drivers")
    plt.yticks(list(set(list(n) + [0.0])))
    plt.grid(True, "major")
    
    plt.savefig(filename, bbox_extra_artists=(legend,), bbox_inches="tight")
    print(f"  -> Created {'/'.join(filename.split('/')[-2:])}")
    plt.clf()

    return [mean, var, skew]

def makeGapsPlotsFromLogList(folder, logList, driverList, generate_plots=True):
    """
    Process multiple log files and create gap distribution plots.
    Returns statistics [mean, variance, skewness] across all logs.
    """
    gapsDirectory = os.path.join(folder, "gaps")
    cumulativeGapsDirectory = os.path.join(folder, "cumulative-gaps")
    os.makedirs(gapsDirectory, exist_ok=True)
    os.makedirs(cumulativeGapsDirectory, exist_ok=True)

    all_gaps = []

    for log in logList:
        print(f"==> Analyzing {log}")
        logFilePath = os.path.join(folder, log)

        # Get gaps from log file
        final_gaps_for_log = fillGaps(logFilePath)
        if not final_gaps_for_log:
            print(f"No gaps found in {log}")
            continue

        # Filter for requested drivers
        single_log_gaps = [
            gap for driver, gap in final_gaps_for_log.items()
            if driver in driverList
        ]

        if not single_log_gaps:
            print(f"No valid gaps found in {log}")
            continue

        all_gaps.extend(single_log_gaps)
        
        # Generate individual log plots if requested
        if generate_plots:
            single_log_gaps = np.array(single_log_gaps, dtype=float)
            outPath = os.path.join(gapsDirectory, f"{log}.svg")
            plotGaps(False, single_log_gaps, outPath)
            outPath2 = os.path.join(cumulativeGapsDirectory, f"{log}.svg")
            plotGaps(True, single_log_gaps, outPath2)

    # Handle no valid gaps across all logs
    if not all_gaps:
        print("No valid gaps found across all logs")
        return [0, 0, 0]
    
    # Generate aggregate plots
    print(f"==> Analyzing {len(all_gaps)} gaps across all logs")
    all_gaps = np.array(all_gaps, dtype=float)
    outPath = os.path.join(gapsDirectory, "all_gaps.svg")
    mean_var_skew = plotGaps(False, all_gaps, outPath)
    outPath2 = os.path.join(cumulativeGapsDirectory, "all_gaps.svg")
    plotGaps(True, all_gaps, outPath2)
    
    return mean_var_skew