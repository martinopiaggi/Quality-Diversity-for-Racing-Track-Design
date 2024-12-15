"""Plot variations of positions without using bot skills."""

import os
import subprocess

import matplotlib.pyplot as plt
import numpy
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

    histPlot = pylab.subplot(1, 1, 1)

    try:
        params = scipy.stats.norm.fit(variations)
        x = numpy.linspace(min(variations), max(variations), 100)

        n, bins, patches = plt.hist(
            variations,
            max(variations) - min(variations) + 1,
            (min(variations) - 0.5, max(variations) + 0.5),
            color="green"
        )
        density = scipy.stats.norm.pdf(x, *params)

        plt.xticks(range(min(variations), max(variations) + 1))
        # Remove duplicates and ensure 0 is visible on the Y axis
        plt.yticks(list(set(list(n) + [0.0])))
        plt.grid(True, "major")

        densityPlot = histPlot.twinx()
        densityPlot.set_ylabel("Density (gaussian distribution) [1]")
        plt.plot(x, density, "b-", linewidth=2)

        mean, var = scipy.stats.norm.stats(*params, moments="mv")
        skew = scipy.stats.skew(variations)

        line = plt.Line2D(range(10), range(10), color='blue', linewidth=2)
        legend = plt.legend(
            [line],
            [f"mean = {mean}\nvar = {var}\nskew = {skew}"],
            loc=8,
            bbox_to_anchor=(0.5, -0.3)
        )

        plt.savefig(filename, bbox_extra_artists=(legend,), bbox_inches="tight")
        print("  -> Created " + "/".join(filename.split("/")[-2:]))

        plt.clf()

        return [mean, var, skew]
    except Exception as e:
        print(f"Warning: Could not process variations data: {e}")
        return [0, 0, 0]  # return default values


def makePositionsVariationsPlotsFromLogList(folder, log, trackLength, lapPercentage, driversList, generate_plots=True):
    """
    Compute and plot position variations of drivers either at the end of the race (lapPercentage=0)
    or at a certain portion of the lap (0<lapPercentage<=1).
    """
    
    output_dir = os.path.join(folder, 
                           "positions-variations" if lapPercentage == 0 
                           else f"start-{int(lapPercentage * 100)}")
    os.makedirs(output_dir, exist_ok=True)  # Use exist_ok instead of checking

    def get_final_positions():
        """Extract final positions from log file"""
        with open(os.path.join(folder, log), 'r') as f:
            lines = f.readlines()
        
        return [(driver, next((int(line.split(',')[13])
                for line in reversed(lines)
                if len(line.split(',')) > 13 and line.split(',')[1] == driver), 
                0))
                for driver in driversList]

    def get_partial_positions():
        """Extract positions at given lap percentage"""
        distance = trackLength - 2.5 if lapPercentage == 1 else lapPercentage * trackLength
        
        cmd_time = f'awk -F "," \'$1>0 && $12>={distance} && $13==1\' "{os.path.join(folder, log)}" | head -n 1'
        try:
            time = subprocess.check_output(cmd_time, shell=True).decode('ascii').strip().split(',')[0]
            cmd_pos = f'awk -F "," \'$1=={time} && $2!="overtake"\' "{os.path.join(folder, log)}"'
            return [line.decode("ascii").split(",")[1] 
                   for line in subprocess.check_output(cmd_pos, shell=True).splitlines()]
        except (subprocess.CalledProcessError, IndexError):
            return []

    # Get positions and calculate variations
    variations = []
    if lapPercentage == 0:
        final_positions = get_final_positions()
        variations = [i - (pos - 1) for i, (driver, pos) in enumerate(final_positions) if pos > 0]
    else:
        positions = get_partial_positions()
        variations = [i - positions.index(driver) 
                     for i, driver in enumerate(driversList) 
                     if driver in positions]

    if not variations:
        return [0, 0, 0]

    return (plotPositionsVariations(variations, os.path.join(output_dir, f"{log}.svg")) 
            if generate_plots 
            else [numpy.mean(variations), numpy.var(variations), scipy.stats.skew(variations)])