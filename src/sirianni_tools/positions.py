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



def makePositionsVariationsPlotsFromLogList(folder, logList, trackLength, lapPercentage, driversList, plotAllRacesOnly, botSkills):
    print("\nDebug positions:")
    print(f"Drivers list: {driversList}")
    print(f"Bot skills: {botSkills}")
    
    allVariations = numpy.array([], dtype=numpy.int32)
    
    skillLevels = []
    i = 0
    while i < len(driversList):
        # Find skill level for each driver
        skillLevels.append([name for name, skill in botSkills].index(driversList[i]))
        i += 1
    
    botNamesBySkill = [name for name, skill in botSkills]
    
    for index, log in enumerate(logList):
        print(f"Processing log {log}")
        variations = []
        positions = []  # Initialize empty positions list

        if lapPercentage == 0:
            # Get final positions
            for driver in driversList:
                try:
                    cmd = f'tac "{folder}/{log}" | grep "{driver}" -m 1'
                    print(f"Running command: {cmd}")
                    output = subprocess.check_output(cmd, shell=True).decode('ascii')
                    print(f"Output: {output}")
                    positions.append(str(output.split(",")[1]))
                except Exception as e:
                    print(f"Error getting position for {driver}: {e}")
                    continue
            
        print(f"Positions collected: {positions}")
        
        # Only proceed if we have positions
        if positions:
            i = 0
            while i < len(driversList):
                try:
                    position = positions.index(driversList[i])
                    variations.append(i - position)
                except ValueError as e:
                    print(f"Could not find position for driver {driversList[i]}")
                i += 1
            
            print(f"Variations calculated: {variations}")
            
            if variations:  # Only plot if we have variations
                plotPositionsVariations(variations, posVarFolder + "/" + log + ".svg")
                plotPositionsVariationsBySkillLevel(variations, posVarFolder + "/" + log + "-by-skill.svg", 
                                                  skillLevels, botNamesBySkill)
                
            if len(variations) > 0:
                allVariations = numpy.append(allVariations, variations)
    
    # Return default values if no variations were collected
    if len(allVariations) == 0:
        print("Warning: No variations collected")
        return [0, 0, 0]

    return [numpy.mean(allVariations), numpy.var(allVariations), scipy.stats.skew(allVariations)]