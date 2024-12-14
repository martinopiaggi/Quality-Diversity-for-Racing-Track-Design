"""Plot variations of positions."""

import array
import math
import signal
import subprocess

import matplotlib.pyplot as plt
import numpy
import os
import pylab
import scipy.stats

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"

def get_driver_position(folder, log_file, driver):
    try:
        with open(os.path.join(folder, log_file), 'r') as f:
            for line in reversed(f.readlines()):
                parts = line.strip().split(',')
                if len(parts) > 1 and parts[1] == driver:
                    return int(parts[13])  # Position field
    except Exception as e:
        print(f"Error getting position for {driver}: {e}")
    return None

def plotPositionsVariations(variations, filename):
    if not variations:  # Handle empty sequence
        return [0, 0, 0]  # return default values
   
    plt.xlabel("Positions gained [1]")
    plt.ylabel("Number of drivers [1]")
    plt.grid(True, "major", "y")
    
    histPlot = pylab.subplot(1,1,1)
    
    try:
        params = scipy.stats.norm.fit(variations)
        x = numpy.linspace(min(variations), max(variations), 100)
        
        n, bins, patches = plt.hist(variations, max(variations) - min(variations) + 1, (min(variations) - 0.5, max(variations) + 0.5), color = "green")
        density = scipy.stats.norm.pdf(x, *params)

        plt.xticks(range(min(variations), max(variations) + 1))
        # Duplicated numbers look like bold (written multiple times), set() does not allow duplicates
        # 0 is added to the list in order to be visible also in the cumulative plot
        # The internal list() should not be necessary, but without it the concatenation does not work in this specific case (I do not know why)
        plt.yticks(list(set(list(n) + [0.0])))
        plt.grid(True, "major")
        
        densityPlot = histPlot.twinx()
        densityPlot.set_ylabel("Density (gaussian distribution) [1]")
        plt.plot(x, density, "b-", linewidth = 2)
        
        mean, var = scipy.stats.norm.stats(*params, moments = "mv")
        skew = scipy.stats.skew(variations)
        
        line = plt.Line2D(range(10), range(10), color = 'blue', linewidth = 2)
        legend = plt.legend([line], ["mean = " + str(mean) + "\nvar = " + str(var) + "\nskew = " + str(skew)], loc = 8, bbox_to_anchor=(0.5, -0.3))
        
        plt.savefig(filename, bbox_extra_artists = (legend,), bbox_inches = "tight")
        print("  -> Created " + filename.split("/")[-2] + "/" + filename.split("/")[-1])

        plt.clf()

        return [mean, var, skew]
    except Exception as e:
        print(f"Warning: Could not process variations data: {e}")
        return [0, 0, 0]  # return default values


def plotPositionsVariationsBySkillLevel(variations, filename, skillLevels, botsNamesBySkill):
    plt.xlabel("Drivers (decreasing skill order)")
    plt.ylabel("Positions gained [1]")
    plt.grid(True, "major", "y")

    histPlot = pylab.subplot(1,1,1)
    
    bars = []
    for index in range(len(variations)):
        bars.append(variations[skillLevels.index(index)])
    
    # Set ticks first
    plt.xticks(range(len(variations)))  # This creates the tick locations
    histPlot.set_xticklabels(botsNamesBySkill, rotation=45, rotation_mode="anchor", ha="right")

    # Draw bars
    plt.bar(range(len(variations)), bars, align="center", color="green")
    
    plt.savefig(filename, bbox_inches="tight")
    print(f" -> Created {filename.split('/')[-2]}/{filename.split('/')[-1]}")
    plt.clf()

    return [numpy.mean(variations), numpy.var(variations), scipy.stats.skew(variations)]



def makePositionsVariationsPlotsFromLogList(folder, logList, trackLength, lapPercentage, driversList, plotAllRacesOnly, generate_plots=True):
    allVariations = numpy.array([], dtype=numpy.int32)

    for index, log in enumerate(logList):
        print(f"==> Analyzing {log}")
        variations = []
        final_positions = []

        if lapPercentage == 0:
            # Get final positions
            for driver in driversList:
                found = False
                try:
                    # Open the log file and read lines
                    with open(os.path.join(folder, log), 'r') as f:
                        lines = f.readlines()
                    # Reverse the lines to start from the end
                    for line in reversed(lines):
                        data = line.strip().split(',')
                        if len(data) > 1 and data[1] == driver:
                            if len(data) >= 14:
                                position = int(data[13])  # Adjust index if necessary
                                final_positions.append((driver, position))
                                found = True
                            else:
                                print(f"Warning: Not enough data fields for {driver} in {log}")
                            break
                    if not found:
                        print(f"Driver {driver} not found in final positions.")
                except Exception as e:
                    print(f"Error processing {driver} in {log}: {e}")
                    continue


            if not final_positions:
                print(f"Warning: No positions were successfully captured in {log}")
                continue

            # Calculate variations from starting positions
            for i, driver in enumerate(driversList):
                try:
                    final_position = next(pos for drv, pos in final_positions if drv == driver)
                    variation = i - (final_position - 1)  # Adjusting for 1-based indexing in positions
                    variations.append(variation)
                except StopIteration:
                    print(f"Driver {driver} not found in final positions.")
                    continue

            if variations:
                if generate_plots:
                    plotPositionsVariations(variations, os.path.join(folder, f"{log}.svg"))
                allVariations = numpy.append(allVariations, variations)
            else:
                print(f"No position variations to plot for {log}")

    # If multiple logs, process overall variations
    if len(allVariations) > 0:
        print("==> Analyzing positions across all logs")
        mean = numpy.mean(allVariations)
        var = numpy.var(allVariations)
        skew = scipy.stats.skew(allVariations)
        return [mean, var, skew]
    else:
        print("No position variations found across all logs.")
        return [0, 0, 0]
