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

	plt.ylabel("Number of drivers [1]")
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



def makeGapsPlotsFromLogList(folder, logList, driverList):
	gapsDirectory = folder + "/gaps/"
	cumulativeGapsDirectory = folder + "/cumulative-gaps/"

	if not os.path.exists(gapsDirectory):
		os.makedirs(gapsDirectory)
	if not os.path.exists(cumulativeGapsDirectory):
		os.makedirs(cumulativeGapsDirectory)
		
	allGaps = np.array([])

	# Prevent broken pipe errors by using the system default behaviour
	signal.signal(signal.SIGPIPE, signal.SIG_DFL)

	for index, log in enumerate(logList):
		print("==> Analyzing " + log + " (" + str(index + 1) + "/" + str(len(logList)) + ")")
		
		gaps = np.array([])
		
		for driver in driverList:
			tac = subprocess.Popen('tac "' + folder + "/" + log + '"', stdout = subprocess.PIPE, shell = True)
			# The last line of the log contains the timeBehindPrev => -m 1 
			grep = subprocess.Popen('grep "' + driver + '" -m 1', stdin = tac.stdout, stdout = subprocess.PIPE, shell = True)
			# Close tac when grep exits (without reading all the pipe)
			tac.stdout.close()
			output = grep.communicate()[0]
			
			# If the driver does not end the race, output.decode("ascii") contains the usual data and not timeBehindPrev
			# Getting the gap by analyzing the "normal" log string would be wrong because the gap is not meaningful for retired drivers
			try:
				gaps = np.append(gaps, float(output.decode("ascii").split(",")[2]))
			except ValueError:
				continue
		
		gapsDistribution = plotGaps(False, gaps, gapsDirectory + log + ".svg")
		plotGaps(True, gaps, cumulativeGapsDirectory + log + ".svg")
		
		allGaps = np.append(allGaps, gaps)
		
	if len(logList) > 1:
		print("==> Analyzing all the races together")
		gapsDistribution = plotGaps(False, allGaps, gapsDirectory + "gaps.svg")
		plotGaps(True, allGaps, cumulativeGapsDirectory + "gaps.svg")

	return gapsDistribution
