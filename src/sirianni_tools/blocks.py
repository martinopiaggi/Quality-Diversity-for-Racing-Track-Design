"""Compute and plot blocks' metrics."""

import csv
import math
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy

import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni and Daniele Loiacono"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"


# Segment's indexes (see readTrack(), different from utils.SegmentData)

SLX = 0
SLY = 1
SRX = 2
SRY = 3
ERX = 4
ERY = 5
ELX = 6
ELY = 7
LENGTH = 8
TYPE = 9
CENTER_X = 10
CENTER_Y = 11
ANGLE_ZS = 12
RADIUS = 13
ENDLENGTH = 14
OVERTAKES = 15
BLOCK = 16
GAIN = 17
WIDTH = 18
ACCEL = 19
BRAKE = 20
STEER = 21
OFFSET = 22
GEAR = 23
SPEED = 24

# Weights for the radiuses average
# The number of blocks considered is equal to the number of elements of this list

radiusWeights = [0.9, 0.8, 0.5, 0.2, 0.1]



# Read the segment's data
def readTrack(trackFolder, trackName, maxBlockLength):
    with open(trackFolder + trackName + ".csv", "r") as csvfile:
        trackData = []
        endLength = 0
        block = -1
        blockLength = 0
        oldSeg = [0, 0, 0, 0, 0, 0, 0, 0, 0, utils.SegmentType.none.value, 0, 0, 0, 0, 0, 0, 0]
        skipLine = True

        for row in csv.reader(csvfile):
            if skipLine:
                # Skip the first line (track length only)
                skipLine = False
            else:
                seg = []
                seg.append(float(row[utils.SegmentData.slx.value]))
                seg.append(float(row[utils.SegmentData.sly.value]))
                seg.append(float(row[utils.SegmentData.srx.value]))
                seg.append(float(row[utils.SegmentData.sry.value]))
                seg.append(float(row[utils.SegmentData.erx.value]))
                seg.append(float(row[utils.SegmentData.ery.value]))
                seg.append(float(row[utils.SegmentData.elx.value]))
                seg.append(float(row[utils.SegmentData.ely.value]))
                seg.append(float(row[utils.SegmentData.length.value]))
                if int(row[utils.SegmentData.type.value]) != utils.SegmentType.straight.value and float(row[utils.SegmentData.radius.value]) < utils.maxBendRadius:
                    seg.append(int(row[utils.SegmentData.type.value]))
                else:
                    seg.append(int(utils.SegmentType.straight.value))
                seg.append(float(row[utils.SegmentData.centerX.value]))
                seg.append(float(row[utils.SegmentData.centerY.value]))
                seg.append(float(row[utils.SegmentData.angleZS.value]))
                seg.append(float(row[utils.SegmentData.radius.value]))
            
                seg.append(endLength + float(row[utils.SegmentData.length.value]))  # endlength
                seg.append(0)                   # overtakes
            
                if blockLength + seg[LENGTH] > maxBlockLength or seg[TYPE] != oldSeg[TYPE]:
                    blockLength = 0
                    block += 1
                seg.append(block)               # block
            
                trackData.append(seg)
        
                endLength += float(row[utils.SegmentData.length.value])
                blockLength += seg[LENGTH]
                oldSeg = seg

                seg.append ( (float(row[utils.SegmentData.elz.value]) + float(row[utils.SegmentData.erz.value]) - float(row[utils.SegmentData.slz.value]) - float(row[utils.SegmentData.srz.value]))/2)

                width = math.sqrt (  (float(row[utils.SegmentData.slx.value]) - float(row[utils.SegmentData.srx.value]))**2 + (float(row[utils.SegmentData.sly.value]) - float(row[utils.SegmentData.sry.value]))**2 )
                width += math.sqrt (  (float(row[utils.SegmentData.elx.value]) - float(row[utils.SegmentData.erx.value]))**2 + (float(row[utils.SegmentData.ely.value]) - float(row[utils.SegmentData.ery.value]))**2 )
                width /= 2
                seg.append (width)

    return trackData



def computeBlockEndLengths(trackData):
    block = -1
    blockEndLengths = []

    for seg in trackData:
        if seg[BLOCK] == block:
            blockEndLengths[block] = seg[ENDLENGTH]
        else:
            block += 1
            blockEndLengths.append(seg[ENDLENGTH])

    return blockEndLengths



def computeBlockStartLengths(trackData):
    block = -1
    blockStartLengths = []

    for seg in trackData:
        if seg[BLOCK] != block:
            block += 1
            blockStartLengths.append(seg[ENDLENGTH] - seg[LENGTH])

    return blockStartLengths



def computeBlockLengths(trackData):
    block = -1
    blockLengths = []

    for seg in trackData:
        if seg[BLOCK] == block:
            blockLengths[block] += seg[LENGTH]
        else:
            block += 1
            blockLengths.append(seg[LENGTH])

    return blockLengths



# Compute the inverse of the radius of a block by averaging the inverse of the radius of its segments
def computeBlockInverseRadiuses(trackData):
    block = -1
    blockRadiuses = []
    segmentsNumber = 0

    for seg in trackData:
        if seg[BLOCK] == block:
            if seg[RADIUS] != 0:
                blockRadiuses[block] += 1 / seg[RADIUS]
            segmentsNumber += 1
        else:
            # Compute the mean of the old block
            if block != -1:
                blockRadiuses[block] /= segmentsNumber

            # Start to analyze the new block
            block += 1
            segmentsNumber = 1
            if seg[RADIUS] != 0:
                blockRadiuses.append(1 / seg[RADIUS])
            else:
                blockRadiuses.append(0)

    # Compute the mean of the last block (the computation above is not executed)
    blockRadiuses[block] /= segmentsNumber

    return blockRadiuses



def computePrevBlockRadius(blockRadiuses):
    return blockRadiuses[-1:] + blockRadiuses[:-1]



def computeNextBlockRadius(blockRadiuses):
    return blockRadiuses[1:] + blockRadiuses[:1]



def computePrevBlockRadiusesMeans(blockRadiuses):
    prevBlockRadiusesMeans = []
    blockIndex = 0
    while blockIndex < len(blockRadiuses):
        i = 1
        prevRadiusesWeightedSum = 0
        while i <= len(radiusWeights):
            prevRadiusesWeightedSum += radiusWeights[i - 1] * blockRadiuses[blockIndex - i]
            i += 1

        prevBlockRadiusesMeans.append(prevRadiusesWeightedSum / len(radiusWeights))

        blockIndex += 1

    return prevBlockRadiusesMeans



def computeNextBlockRadiusesMeans(blockRadiuses):
    computeNextBlockRadiusesMeans = []
    blockIndex = 0
    while blockIndex < len(blockRadiuses):
        i = 1
        nextRadiusesWeightedSum = 0
        while i <= len(radiusWeights):
            nextRadiusesWeightedSum += radiusWeights[i - 1] * blockRadiuses[(blockIndex + i) % len(blockRadiuses)]
            i += 1

        computeNextBlockRadiusesMeans.append(nextRadiusesWeightedSum / len(radiusWeights))

        blockIndex += 1

    return computeNextBlockRadiusesMeans



def computePrevSegsRads(trackData, thr):
    block = -1
    segmentsNumber = 0

    blockStartLengths = computeBlockStartLengths(trackData)

    prevSegRads=[0]*len(blockStartLengths)
    trackLenght = computeBlockEndLengths(trackData)[-1]

    alpha = 10**(-2.0/thr)
    norm = (alpha**thr-1)

    for seg in trackData:
        for i in range (len(prevSegRads)):
            dist = blockStartLengths[i] - seg[ENDLENGTH]
            if dist<0:
                dist += trackLenght
            if dist < thr:
                if seg[LENGTH] + dist < thr:
                    a = dist
                    b = dist + seg[LENGTH]
                else:
                    a = dist
                    b = thr

                invRad = 0 if seg[RADIUS] == 0 else (1 / seg[RADIUS]) * (alpha ** b - alpha ** a) / norm
                prevSegRads[i] += invRad

    return prevSegRads



def computeNextSegsRads(trackData, thr):
    block = -1
    segmentsNumber = 0

    blockEndLenghts = computeBlockEndLengths(trackData)

    nextSegRads=[0]*len(blockEndLenghts)
    trackLenght = blockEndLenghts[-1]

    alpha = 10**(-2.0/thr)
    norm = (alpha**thr-1)

    for seg in trackData:
        for i in range (len(nextSegRads)):
            dist = seg[ENDLENGTH] - seg[LENGTH] - blockEndLenghts[i]
            if dist<0:
                dist += trackLenght
            if dist < thr:
                if seg[LENGTH] + dist < thr:
                    a = dist
                    b = dist + seg[LENGTH]
                else:
                    a = dist
                    b = thr

                invRad = 0 if seg[RADIUS] == 0 else (1 / seg[RADIUS]) * (alpha ** b - alpha ** a) / norm
                nextSegRads[i] += invRad

    return nextSegRads



def computeBlockTypes(trackData):
    block = -1
    blockTypes = []

    for seg in trackData:
        if seg[BLOCK] != block:
            block += 1
            blockTypes.append(seg[TYPE])

    return blockTypes



def computeBlockGrade(trackData):
    block = -1
    blockGrades = []
    lengths = computeBlockLengths(trackData)

    for seg in trackData:
        if seg[BLOCK] == block:
            blockGrades[block] += seg[GAIN]/lengths[block]
        else:
            # Start to analyze the new block
            block += 1
            blockGrades.append(seg[GAIN]/lengths[block])

    return blockGrades



def computeBlockMetric(trackData, IDX):
    block = -1
    metric = []
    norm = 0

    for seg in trackData:
        if seg[BLOCK] == block:
            metric[block] += seg[IDX] * seg[LENGTH]
            norm += seg[LENGTH]
        else:
            if block!=-1:
                metric[block] /= norm

            # Start to analyze the new block
            block += 1
            norm = seg[LENGTH]
            metric.append(seg[IDX] * seg[LENGTH])

    metric[block] /= norm

    return metric



def computeBlockAggregateMetric(trackData, IDX):
    block = -1
    data = []
    metric = []

    for seg in trackData:
        if seg[BLOCK] == block:
            if isinstance(seg[IDX], list):
                data.extend(seg[IDX])

        else:
            if block != -1:
                # Handle empty data case
                if len(data) > 0:
                    metric[block].extend([numpy.mean(data)])
                    metric[block].extend([numpy.std(data)])
                    metric[block].extend(numpy.percentile(data, [25, 50, 75]))
                else:
                    # Default values for empty data
                    metric[block].extend([0, 0, 0, 0, 0])  # mean, std, q1, q2, q3

            # Start new block
            block += 1
            data = []
            if isinstance(seg[IDX], list):
                data.extend(seg[IDX])
            metric.append([])

    # Handle last block
    if len(data) > 0:
        metric[block].extend([numpy.mean(data)])
        metric[block].extend([numpy.std(data)])
        metric[block].extend(numpy.percentile(data, [25, 50, 75]))
    else:
        metric[block].extend([0, 0, 0, 0, 0])

    return metric


def plotMetric(trackData, blockMetric, label, imagePath, integersInColorbar = False, clearFigure = True, customLevels=None):
    # Add safety check at the start
    if not trackData or not list(blockMetric):
        print(f"Warning: Empty track data or metrics for {imagePath}, skipping plot")
        return

    metricMax = max(blockMetric)
    
    try:
        # Filter non-zero values and find minimum, with safety check
        nonZeroMetrics = list(filter(lambda x: x != 0, blockMetric))
        metricMin = min(nonZeroMetrics) if nonZeroMetrics else 0
    except ValueError:
        # If all values are zero or sequence is empty
        metricMin = 0
        metricMax = 0

    plt.axis("off")                         # disable axis drawing
    plt.gca().set_aspect("equal", adjustable = "box")       # set the same scale for x and y

    for seg in trackData:
        if blockMetric[seg[BLOCK]] != 0 or metricMin < 0:
            if metricMax != metricMin:
                heat = int(255 * abs(blockMetric[seg[BLOCK]] - metricMin) / abs(metricMax - metricMin))     # used as red channel
            else:
                # Prevent a division by zero
                # If metricMax == metricMin then all the blocks have the same number of overtakes, let us paint them all in green
                heat = 0        # used as red channel
            # Make a bright yellow by computing red and green values maximizing the greater one.
            if heat >= 128:
                red = 255
                green = int((255-heat)*255/heat)
            else:
                red = int(heat*255/(255-heat))
                green = 255
            plt.fill(seg[SLX:ELX+1:2], seg[SLY:ELY+1:2], "#" + format(red, "02x") + format(green, "02x") + "00", edgecolor = "none")
        else:
            plt.fill(seg[SLX:ELX+1:2], seg[SLY:ELY+1:2], "k", edgecolor = "none")
    
    cmap = matplotlib.colors.LinearSegmentedColormap.from_list(name = "cmap", colors = ["#00ff00", "#ffff00", "#ff0000"], N = 1000)


    if metricMax>metricMin and customLevels is None:
        plt.contourf([[metricMin, metricMax], [metricMin, metricMax]], cmap = cmap, levels = numpy.linspace(metricMin, metricMax, 1000))
    if customLevels is not None:
        plt.contourf([[metricMin, metricMax], [metricMin, metricMax]], cmap=cmap, levels=customLevels)
    if metricMax==metricMin and customLevels is None:
        plt.contourf([[metricMin, metricMax], [metricMin, metricMax]], cmap=cmap)

    if label:
        colorBar = plt.colorbar()
        colorBar.locator = matplotlib.ticker.MaxNLocator(nbins = 19, integer = integersInColorbar)
        colorBar.update_ticks()
    
        if metricMax != metricMin:
            colorBar.set_label(label)
        else:
            colorBar.set_label(label + " = " + str(metricMax))

    plt.savefig(imagePath)
    print("  -> Created " + imagePath.split("/")[-2] + "/" + imagePath.split("/")[-1])
    
    if clearFigure:
        plt.clf()


def readDynamics(trackName, trackData):
    # Debug check for first segment
    print(f"\nDebug: Initial segment length: {len(trackData[0])}")

    # Initialize missing slots in all segments
    for seg in trackData:
        while len(seg) <= SPEED:
            seg.append([])
            
    print(f"Debug: After init segment length: {len(trackData[0])}")

    accel = []
    brake = []
    steer = []
    offset = []
    gear = []
    speed = []

    seg = 0

    with open(os.path.join(utils.torcsLogPath, sorted([f for f in os.listdir(utils.torcsLogPath) 
        if f.startswith(trackName) and f.endswith('_dynamics.csv')])[-1]), 'r') as csvfile:
        data = csv.reader(csvfile,delimiter=',')
        for row in data:
            if len(row)!=26:
                continue
            lap = int(row[12])
            dist = float(row[11])
            end = trackData[seg][ENDLENGTH]

            if lap == 1 and dist <= end:
                accel.append(float(row[23]))
                brake.append(float(row[24]))
                steer.append(float(row[22]))
                offset.append(float(row[8]))
                gear.append(float(row[25]))
                speed.append(float(row[5]))
            elif lap == 1 and dist > end:
                trackData[seg][ACCEL] = accel
                trackData[seg][BRAKE] = brake
                trackData[seg][STEER] = steer
                trackData[seg][OFFSET] = offset
                trackData[seg][GEAR] = gear
                trackData[seg][SPEED] = speed

                seg +=1

                accel=[float(row[23])]
                brake=[float(row[24])]
                steer=[float(row[21])] 
                offset=[float(row[8])]
                gear=[float(row[25])]
                speed=[float(row[5])]

        # Handle last segment
        trackData[seg][ACCEL] = accel
        trackData[seg][BRAKE] = brake
        trackData[seg][STEER] = steer
        trackData[seg][OFFSET] = offset
        trackData[seg][GEAR] = gear
        trackData[seg][SPEED] = speed

    return trackData

def makeMetricsPlots(folder, trackDirectory, trackName, maxBlockLength, generate_plots=True):
    try:
        trackMetricsFolder = os.path.join(folder, "track")
        if not os.path.exists(trackMetricsFolder):
            os.makedirs(trackMetricsFolder)
            
        trackData = readTrack(trackDirectory, trackName, maxBlockLength)
        if trackData is None:
            return None
            
        trackData = readDynamics(trackName, trackData)
        if not generate_plots:
            return trackData
                
        # Essential radius-based plot
        blockInverseRadiuses = computeBlockInverseRadiuses(trackData)
        plotMetric(trackData, blockInverseRadiuses, "Block inverse radiuses", 
                  trackMetricsFolder + "/block-radiuses.svg")
        
        # Essential track structure plots
        plotMetric(trackData, computeBlockLengths(trackData), "Block lengths",  
                  trackMetricsFolder + "/block-lengths.svg", True)
        plotMetric(trackData, list(map(lambda x: x%2, range(len(trackData)))), "",  
                  trackMetricsFolder + "/blocks.svg")

        # Essential metrics
        grades = computeBlockGrade(trackData)
        plotMetric(trackData, grades, "Block Grade", 
                  trackMetricsFolder + "/block-grade.svg")

        width = computeBlockMetric(trackData, WIDTH)
        plotMetric(trackData, width, "Block Width", 
                  trackMetricsFolder + "/block-width.svg")

        # Essential vehicle dynamics
        
        # Speed - most important metric
        speed = computeBlockAggregateMetric(trackData, SPEED)
        speed_avg = [i[0] for i in speed]
        plotMetric(trackData, speed_avg, "Average block speed [m/s]", 
                  trackMetricsFolder + "/block-speed-avg.svg")
        
        # Acceleration
        accel = computeBlockAggregateMetric(trackData, ACCEL)
        accel_avg = [i[0] for i in accel]
        plotMetric(trackData, accel_avg, "Avg Block Acceleration", 
                  trackMetricsFolder + "/block-accel-avg.svg")
        
        # Braking
        brake = computeBlockAggregateMetric(trackData, BRAKE)
        brake_avg = [i[0] for i in brake]
        plotMetric(trackData, brake_avg, "Avg Block Brake", 
                  trackMetricsFolder + "/block-brake-avg.svg")
        
        # Steering
        steer = computeBlockAggregateMetric(trackData, STEER)
        steer_avg = [i[0] for i in steer]
        plotMetric(trackData, steer_avg, "Avg Block Steer", 
                  trackMetricsFolder + "/block-steer-avg.svg")
        
        return trackData

    except Exception as e:
        print(f"Error in makeMetricsPlots: {e}")
        return None