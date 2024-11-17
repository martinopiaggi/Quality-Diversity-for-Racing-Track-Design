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
	plt.xlabel("Positions gained [1]")
	plt.ylabel("Number of drivers [1]")
	plt.grid(True, "major", "y")
	
	histPlot = pylab.subplot(1,1,1)
	
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



def plotPositionsVariationsBySkillLevel(variations, filename, skillLevels, botsNamesBySkill):
	plt.xlabel("Drivers in decreasing skill order")
	plt.ylabel("Positions gained [1]")
	plt.grid(True, "major", "y")
	
	histPlot = pylab.subplot(1,1,1)

	bars = []
	for index in range(len(variations)):
		bars.append(variations[skillLevels.index(index)])
	
	plt.bar(range(len(variations)), bars, align = "center", color = "green")
	plt.xticks(range(len(variations)))
	histPlot.set_xticklabels(botsNamesBySkill, rotation = 45, rotation_mode = "anchor", ha = "right")
	
	plt.savefig(filename, bbox_inches = "tight")
	print("  -> Created " + filename.split("/")[-2] + "/" + filename.split("/")[-1])

	plt.clf()



def makePositionsVariationsPlotsFromLogList(folder, logList, trackLength, lapPercentage, startingPositions, plotAllRacesOnly, botSkills):
	if lapPercentage == 0:
		posVarFolder = folder + "/positions-variations"
		if not os.path.exists(posVarFolder):
			os.makedirs(posVarFolder)
	else:
		posVarFolder = folder + "/start-" + str(int(lapPercentage * 100))
		if not os.path.exists(posVarFolder):
			os.makedirs(posVarFolder)

	skillLevels = []
	i = 0
	while i < len(startingPositions):
		skillLevels.append([name for (name, lapTime) in botSkills].index(startingPositions[i]))
		i += 1

	botNamesBySkill = [name for (name, lapTime) in botSkills]
		
	allVariations = numpy.array([], dtype = numpy.int)

	for index, log in enumerate(logList):
		print("==> Analyzing " + log + " (" + str(index + 1) + "/" + str(len(logList)) + ")")
		
		if lapPercentage == 0:
			# make a list made of len(startingPositions) 'bot's (e.g. ['bot', 'bot', 'bot', 'bot', 'bot'])
			positions = ["bot"] * len(startingPositions)
			# prevent broken pipe errors by using the system default behaviour
			signal.signal(signal.SIGPIPE, signal.SIG_DFL)
			
			for driver in startingPositions:
				tac = subprocess.Popen('tac "' + folder + "/" + log + '"', stdout = subprocess.PIPE, shell = True)
				grep = subprocess.Popen('grep "' + driver + '" -m 2', stdin = tac.stdout, stdout = subprocess.PIPE, shell = True)
				# close tac when grep exits (without reading all the pipe)
				tac.stdout.close()
				output = grep.communicate()[0]

				positions[int((output.decode("ascii").split("\n")[1]).split(",")[13]) - 1] = str((output.decode("ascii").split("\n")[1]).split(",")[1])
		else:
			# explicitly check that the driver is in the first lap to throw away the records at the start of the race => $13==1
			if lapPercentage == 1:
				# this could be slightly before the end of the lap but is safe with respect to one-lap races (one could simply take $13==2)
				#
				# -2.5 has been introduced to consider the cars which are very close to the end of the lap (but behind trackLength) and
				# overtake trackLength at the next simulation step (for instance, I got logs with 3773.56 at Alpine 2 whose length is 3773.57).
				# 2.5 is enough as in order to travel for 2.5 meters in a simulation step you would have to go at 2.5m / 20ms = 125m/s = 450km/h !
				time = subprocess.check_output('awk -F "," \'$1>0 && $12>=' + str(lapPercentage * trackLength - 2.5) + ' && $13==1\' "' + folder + "/" + log + '" | head -n 1', shell = True).decode('ascii').split(",")[0]
			else:
				time = subprocess.check_output('awk -F "," \'$1>0 && $12>=' + str(lapPercentage * trackLength) + ' && $13==1\' "' + folder + "/" + log + '" | head -n 1', shell = True).decode('ascii').split(",")[0]

			positions = []
			output = subprocess.check_output('awk -F "," \'$1==' + time + ' && $2!="overtake"\' "' + folder + "/" + log + '"', shell = True).splitlines()

			for line in output:
				positions.append(str(line.decode("ascii").split(",")[1]))

		# the variations of positions of the bots in the starting grid order
		variations = []
		i = 0
		while i < len(startingPositions):
			position = positions.index(startingPositions[i])
			variations.append(i - position)
			i += 1
		
		allVariations = numpy.append(allVariations, variations)

		if len(logList) <= 1 or not plotAllRacesOnly:
			positionsVariationsDistribution = plotPositionsVariations(variations, posVarFolder + "/" + log + ".svg")
			plotPositionsVariationsBySkillLevel(variations, posVarFolder + "/" + log + "-by-skill.svg", skillLevels, botNamesBySkill)
		
	if len(logList) > 1:
		print("==> Analyzing all the races together")
		positionsVariationsDistribution = plotPositionsVariations(allVariations, posVarFolder + "/" + "positions-variations.svg")
		# allVariations is the concatenation of the "variations" lists.
		# That is fine if only the variations are considered but, if I have to take skill into consideration, what I need is a list with an element for each bot.
		allVariationsSummed = [sum([allVariations[x] for x in range(len(allVariations)) if x % len(startingPositions) == botIndex]) for botIndex in range(len(startingPositions))]
		plotPositionsVariationsBySkillLevel(allVariationsSummed, posVarFolder + "/" + "positions-variations-by-skill.svg", skillLevels, botNamesBySkill)

	return positionsVariationsDistribution
