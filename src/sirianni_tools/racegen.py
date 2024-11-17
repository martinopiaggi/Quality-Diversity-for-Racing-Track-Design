"""Generate a Torcs race setup in quickrace.xml."""

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



defaultBotList = []
defaultBotList.append(("bt", "1"))
defaultBotList.append(("tita", "4"))
defaultBotList.append(("damned", "3"))
defaultBotList.append(("olethros", "2"))
defaultBotList.append(("mluc_blueeagle", "0"))
defaultBotList.append(("simplix_trb1a", "0"))
defaultBotList.append(("wdbee_robotics", "0"))
defaultBotList.append(("mluc_2014", "0"))

trackLogger = []
trackLogger.append(("simplix_trb1a", "0"))

trackexporterBotList = []
trackexporterBotList.append(("trackexporter", "1"))



def racegen(trackName, trackType, botList, numLaps, path):
	# Generate drivers section

	i = 0
	driversSection = ""
	while i < len(botList):
		driversSection += '''
		<section name="''' + str(i+1) + '''">
			<attnum name="idx" val="''' + botList[i][1] + '''"/>
			<attstr name="module" val="''' + botList[i][0] + '''"/>
		</section>'''
		i += 1

	# Generate race file

	with open(path, "w") as raceFile:
		raceFile.write(
'''<?xml version="1.0" encoding="UTF-8"?>

<!DOCTYPE params SYSTEM "../libs/tgf/params.dtd">

<params name="Quick Race" type="param" mode="mw">

	<section name="Tracks">
		<section name="1">
			<attstr name="name" val="''' + trackName + '''"/>
			<attstr name="category" val="''' + trackType + '''"/>
		</section>
	</section>

	<section name="Races">
		<section name="1">
			<attstr name="name" val="Quick Race"/>
		</section>
	</section>

	<section name="Quick Race">
		<attnum name="laps" val="''' + str(numLaps) + '''"/>
		<attstr name="type" val="race"/>
		<attstr name="starting order" val="random"/>
	</section>

	<section name="Drivers">'''
	+ driversSection + '''
	</section>

</params>''')
