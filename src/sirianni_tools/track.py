"""Analyze track topology and plot track's data."""

import csv
import os

import matplotlib.pyplot as plt
import scipy.stats

import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



# Read the overall track's data (not related to the segments but to the track in its entirety)
def readTrack(trackFile):
    with open(trackFile, "r") as csvfile:
        isFirstRow = True
        leftBendsCount = 0
        leftBendsCount30pc = 0
        leftBendsCount50pc = 0
        rightBendsCount = 0
        rightBendsCount30pc = 0
        rightBendsCount50pc = 0
        straightsCount = 0
        straightsCount30pc = 0
        straightsCount50pc = 0
        straightsLength = 0
        straightsLength30pc = 0
        straightsLength50pc = 0
        previousSegmentType = utils.SegmentType.none
        radiuses = []
        radiuses30pc = []
        radiuses50pc = []
        heights = []
        heights30pc = []
        heights50pc = []
        analyzedLength = 0;

        for row in csv.reader(csvfile):
            if isFirstRow:
                # Track's global data
                length = float(row[0])
                width = float(row[1])
                
                isFirstRow = False
            else:
                # Segments' data
                if int(row[utils.SegmentData.type.value]) != utils.SegmentType.straight.value and float(row[utils.SegmentData.radius.value]) < utils.maxBendRadius:
                    segmentType = int(row[utils.SegmentData.type.value])
                else:
                    segmentType = int(utils.SegmentType.straight.value)

                analyzedLength += float(row[utils.SegmentData.length.value])
            
                if segmentType == utils.SegmentType.straight.value:
                    straightsLength += float(row[utils.SegmentData.length.value])
                    if analyzedLength <= length / 3:
                        straightsLength30pc += float(row[utils.SegmentData.length.value])
                        length30pc = analyzedLength
                    if analyzedLength <= length / 2:
                        straightsLength50pc += float(row[utils.SegmentData.length.value])
                        length50pc = analyzedLength
                else:
                    radiuses.append(float(row[utils.SegmentData.radius.value]))
                    if analyzedLength <= length / 3:
                        radiuses30pc.append(float(row[utils.SegmentData.radius.value]))
                    if analyzedLength <= length / 2:
                        radiuses50pc.append(float(row[utils.SegmentData.radius.value]))
                
                if segmentType != previousSegmentType:
                    if segmentType == utils.SegmentType.left.value:
                        leftBendsCount += 1
                        if analyzedLength <= length / 3:
                            leftBendsCount30pc += 1
                        if analyzedLength <= length / 2:
                            leftBendsCount50pc += 1
                    elif segmentType == utils.SegmentType.right.value:
                        rightBendsCount += 1
                        if analyzedLength <= length / 3:
                            rightBendsCount30pc += 1
                        if analyzedLength <= length / 2:
                            rightBendsCount50pc += 1
                    else:
                        straightsCount += 1
                        if analyzedLength <= length / 3:
                            straightsCount30pc += 1
                        if analyzedLength <= length / 2:
                            straightsCount50pc += 1
                    
                previousSegmentType = segmentType
            
                # Append the height of the middle point of the starting (geometric) segment of the track segment
                heights.append((float(row[utils.SegmentData.slz.value]) + float(row[utils.SegmentData.srz.value])) / 2)
                if analyzedLength <= length / 3:
                    heights30pc.append((float(row[utils.SegmentData.slz.value]) + float(row[utils.SegmentData.srz.value])) / 2)
                if analyzedLength <= length / 2:
                    heights50pc.append((float(row[utils.SegmentData.slz.value]) + float(row[utils.SegmentData.srz.value])) / 2)
        
        trackData = []
        
        trackData.append(length)
        trackData.append(width)
        trackData.append(leftBendsCount)
        trackData.append(rightBendsCount)
        trackData.append(straightsCount)
        trackData.append(straightsLength)
        trackData.append(radiuses)
        trackData.append(heights)
        
        trackData.append(length30pc)
        trackData.append(leftBendsCount30pc)
        trackData.append(rightBendsCount30pc)
        trackData.append(straightsCount30pc)
        trackData.append(straightsLength30pc)
        trackData.append(radiuses30pc)
        trackData.append(heights30pc)
        
        trackData.append(length50pc)
        trackData.append(leftBendsCount50pc)
        trackData.append(rightBendsCount50pc)
        trackData.append(straightsCount50pc)
        trackData.append(straightsLength50pc)
        trackData.append(radiuses50pc)
        trackData.append(heights50pc)
                
        return trackData



def analyzeRadiuses(radiuses):
    params = scipy.stats.norm.fit(radiuses)
    
    mean, var = scipy.stats.norm.stats(*params, moments = "mv")
    skew = scipy.stats.skew(radiuses)

    return (float(mean), float(var), skew)



def analyzeHeights(heights):
    params = scipy.stats.norm.fit(heights)
    
    mean, var = scipy.stats.norm.stats(*params, moments = "mv")
    skew = scipy.stats.skew(heights)
    
    return (float(mean), float(var), skew)



def plotSegmentTypes(trackFile, filename):
    plt.axis("off")                     # disable axis drawing
    plt.gca().set_aspect("equal", adjustable = "box")   # set same scale for x and y
    
    with open(trackFile, "r") as csvfile:
        firstRow = True

        for row in csv.reader(csvfile):
            if firstRow:
                firstRow = False
            else:
                if int(row[utils.SegmentData.type.value]) == utils.SegmentType.left.value and float(row[utils.SegmentData.radius.value]) < utils.maxBendRadius:
                    plt.fill(row[int(utils.SegmentData.slx.value) : int(utils.SegmentData.elx.value) + 1 : 3], row[int(utils.SegmentData.sly.value) : int(utils.SegmentData.ely.value) + 1 : 3], "r", edgecolor = "none")
                elif int(row[utils.SegmentData.type.value]) == utils.SegmentType.right.value and float(row[utils.SegmentData.radius.value]) < utils.maxBendRadius:
                    plt.fill(row[utils.SegmentData.slx.value : utils.SegmentData.elx.value + 1 : 3], row[utils.SegmentData.sly.value : utils.SegmentData.ely.value + 1 : 3], "g", edgecolor = "none")
                else:
                    plt.fill(row[utils.SegmentData.slx.value : utils.SegmentData.elx.value + 1 : 3], row[utils.SegmentData.sly.value : utils.SegmentData.ely.value + 1 : 3], "k", edgecolor = "none")
    
        redSquare = plt.Line2D(range(10), range(10), color = "r", linewidth = 2)
        greenSquare = plt.Line2D(range(10), range(10), color = "g", linewidth = 2)
        blackSquare = plt.Line2D(range(10), range(10), color = "k", linewidth = 2)
        legend = plt.legend([redSquare, greenSquare, blackSquare], ["Left", "Right", "Straight"], loc = 8, bbox_to_anchor=(0.5, -0.5))

        plt.savefig(filename, bbox_extra_artists = (legend,), bbox_inches = "tight")
        print("  -> Created " + filename.split("/")[-2] + "/" + filename.split("/")[-1])
        plt.clf()



def plotSegments(trackFile, outputPath, color1, color2):
    plt.axis("off")                     # disable axis drawing
    plt.gca().set_aspect("equal", adjustable = "box")   # set same scale for x and y
    
    with open(trackFile, "r") as csvfile:
        for index, row in enumerate(csv.reader(csvfile)):
            if index > 0:
                if index % 2 == 0:
                    plt.fill(row[utils.SegmentData.slx.value : utils.SegmentData.elx.value + 1 : 3], row[utils.SegmentData.sly.value : utils.SegmentData.ely.value + 1 : 3], color1, edgecolor = "none")
                else:
                    plt.fill(row[utils.SegmentData.slx.value : utils.SegmentData.elx.value + 1 : 3], row[utils.SegmentData.sly.value : utils.SegmentData.ely.value + 1 : 3], color2, edgecolor = "none")

        plt.savefig(outputPath)
        print("  -> Created " + outputPath.split("/")[-2] + "/" + outputPath.split("/")[-1])
        plt.clf()



def analyzeTrack(trackFile, outputFolder):
    trackData = readTrack(trackFile)

    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
    plotSegmentTypes(trackFile, outputFolder + "/segment-types.svg")
    plotSegments(trackFile, outputFolder + "/segments.svg", "k", "#666666")
    plotSegments(trackFile, outputFolder + "/track.svg", "k", "k")

    return trackData[0:6] + list(analyzeRadiuses(trackData[6])) + list(analyzeHeights(trackData[7])) +\
        trackData[8:13] + list(analyzeRadiuses(trackData[13])) + list(analyzeHeights(trackData[14])) +\
        trackData[15:20] + list(analyzeRadiuses(trackData[20])) + list(analyzeHeights(trackData[21]))
