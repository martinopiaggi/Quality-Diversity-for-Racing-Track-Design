"""Plot overtakes and analyze blocks."""

import csv
import math
import os
import sys
import subprocess

import matplotlib
import matplotlib.pyplot as plt
import numpy

import blocks
import utils

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"



def getTrackWidth(trackName):
    if not os.path.exists(utils.torcsTrackDirectory + trackName + ".csv"):
        sys.exit("ERROR: track data file does not exist!")
    else:
        with open(utils.torcsTrackDirectory + trackName + ".csv", "r") as f:
            return float(f.readline().strip().rsplit(",")[1])



# Adapted from Torcs (see libs/robottools/rttrack.cpp)
def RtTrackLocal2Global(seg, toStart, toMiddle, width):
    if seg[blocks.TYPE] == utils.SegmentType.straight.value:
        CosA = math.cos(seg[blocks.ANGLE_ZS])
        SinA = math.sin(seg[blocks.ANGLE_ZS])
        tr = toMiddle + width / 2
        x = seg[blocks.SRX] + toStart * CosA - tr * SinA
        y = seg[blocks.SRY] + toStart * SinA + tr * CosA
    elif seg[blocks.TYPE] == utils.SegmentType.left.value:
        a = seg[blocks.ANGLE_ZS] + toStart / seg[blocks.RADIUS]
        r = seg[blocks.RADIUS] - toMiddle
        x = seg[blocks.CENTER_X] + r * math.sin(a)
        y = seg[blocks.CENTER_Y] - r * math.cos(a)
    else:
        a = seg[blocks.ANGLE_ZS] - toStart / seg[blocks.RADIUS]
        r = seg[blocks.RADIUS] + toMiddle
        x = seg[blocks.CENTER_X] - r * math.sin(a)
        y = seg[blocks.CENTER_Y] + r * math.cos(a)
    return (x,y)



def fillOvertakes(trackData, logFile):
    output = subprocess.check_output('grep overtake "' + logFile + '"', shell = True).splitlines()
    
    for line in output:
        if int(line.decode("ascii").split(",")[5]) == 0 and int(line.decode("ascii").split(",")[11]) == 0 and int(line.decode("ascii").split(",")[8]) > 1:
            # Both cars are racing regularly (no one is retired or in pit) and the overtaker is at least at the second lap
            length = float(line.decode("ascii").split(",")[2])      # distFromStartLine
            seg = 0

            while seg < len(trackData) and trackData[seg][blocks.ENDLENGTH] < length:
                seg += 1

            trackData[seg][blocks.OVERTAKES] += 1

    return trackData



def computeBlockOvertakes(trackData):
    block = -1
    blockOvertakes = []

    for seg in trackData:
        if seg[blocks.BLOCK] == block:
            blockOvertakes[block] += seg[blocks.OVERTAKES]
        else:
            block += 1
            blockOvertakes.append(seg[blocks.OVERTAKES])

    return blockOvertakes



def makeBlocksDataFile(trackData, trackName, outputFolder, fillOvertakes, maxBlockLength):
    trackLength = trackData[-1][blocks.ENDLENGTH]
    blockEndLengths = blocks.computeBlockEndLengths(trackData)
    blockLengths = blocks.computeBlockLengths(trackData)
    if fillOvertakes:
        blockOvertakes = computeBlockOvertakes(trackData)
    blockRadiuses = blocks.computeBlockInverseRadiuses(trackData)
    prevRadiusesMeans = blocks.computePrevBlockRadiusesMeans(blockRadiuses)
    nextRadiusesMeans = blocks.computeNextBlockRadiusesMeans(blockRadiuses)
    prevRad400 = blocks.computePrevSegsRads(trackData,400)
    nextRad400 = blocks.computeNextSegsRads(trackData, 400)
    prevRad200 = blocks.computePrevSegsRads(trackData, 200)
    nextRad200 = blocks.computeNextSegsRads(trackData, 200)
    prevRad100 = blocks.computePrevSegsRads(trackData, 100)
    nextRad100 = blocks.computeNextSegsRads(trackData, 100)
    prevRad50 = blocks.computePrevSegsRads(trackData, 50)
    nextRad50 = blocks.computeNextSegsRads(trackData, 50)
    blockTypes = blocks.computeBlockTypes(trackData)
    blockGrade = blocks.computeBlockGrade(trackData)

    width = blocks.computeBlockMetric(trackData, blocks.WIDTH)

    accel = blocks.computeBlockAggregateMetric(trackData, blocks.ACCEL)
    accel_avg = [i[0] for i in accel]
    accel_std = [i[1] for i in accel]
    accel_q1 = [i[2] for i in accel]
    accel_q2 = [i[3] for i in accel]
    accel_q3 = [i[4] for i in accel]

    brake = blocks.computeBlockAggregateMetric(trackData, blocks.BRAKE)
    brake_avg = [i[0] for i in brake]
    brake_std = [i[1] for i in brake]
    brake_q1 = [i[2] for i in brake]
    brake_q2 = [i[3] for i in brake]
    brake_q3 = [i[4] for i in brake]

    steer = blocks.computeBlockAggregateMetric(trackData, blocks.STEER)
    steer_avg = [i[0] for i in steer]
    steer_std = [i[1] for i in steer]
    steer_q1 = [i[2] for i in steer]
    steer_q2 = [i[3] for i in steer]
    steer_q3 = [i[4] for i in steer]

    offset = blocks.computeBlockAggregateMetric(trackData, blocks.OFFSET)
    offset_avg = [i[0] for i in offset]
    offset_std = [i[1] for i in offset]
    offset_q1 = [i[2] for i in offset]
    offset_q2 = [i[3] for i in offset]
    offset_q3 = [i[4] for i in offset]

    gear = blocks.computeBlockAggregateMetric(trackData, blocks.GEAR)
    gear_avg = [i[0] for i in gear]
    gear_std = [i[1] for i in gear]
    gear_q1 = [i[2] for i in gear]
    gear_q2 = [i[3] for i in gear]
    gear_q3 = [i[4] for i in gear]

    speed = blocks.computeBlockAggregateMetric(trackData, blocks.SPEED)
    speed_avg = [i[0] for i in speed]
    speed_std = [i[1] for i in speed]
    speed_q1 = [i[2] for i in speed]
    speed_q2 = [i[3] for i in speed]
    speed_q3 = [i[4] for i in speed]

    if fillOvertakes:
        fileName = trackName + "-blocks-data.csv"
    else:
        fileName = trackName + "-blocks-data-without-overtakes-" + str(maxBlockLength) + ".csv"

    with open(outputFolder + "/" + fileName, "w") as csvfile:
        writer = csv.writer(csvfile, delimiter = ";")
        writer.writerow(["Track",
                         "Type",
                         "PrevType",
                         "NextType",
                         "Overtakes",
                         "Length",
                         "Block radius",
                         "Previous Block radius",
                         "Next Block radius",
                         "Previous radiuses mean",
                         "Next radiuses mean",
                         "PrevRad400",
                         "NextRad400",
                         "PrevRad200",
                         "NextRad200",
                         "PrevRad100",
                         "NextRad100",
                         "PrevRad50",
                         "NextRad50",
                         "Grade",
                         "PrevGrade",
                         "NextGrade",
                         "Width",
                         "PrevWidth",
                         "NextWidth",
                         "AccelAvg",
                         "PrevAccelAvg",
                         "NextAccelAvg",
                         "AccelStd",
                         "PrevAccelStd",
                         "NextAccelStd",
                         "AccelQ1",
                         "PrevAccelQ1",
                         "NextAccelQ1",
                         "AccelQ2",
                         "PrevAccelQ2",
                         "NextAccelQ2",
                         "AccelQ3",
                         "PrevAccelQ3",
                         "NextAccelQ3",
                         "BrakeAvg",
                         "PrevBrakeAvg",
                         "NextBrakeAvg",
                         "BrakeStd",
                         "PrevBrakeStd",
                         "NextBrakeStd",
                         "BrakeQ1",
                         "PrevBrakeQ1",
                         "NextBrakeQ1",
                         "BrakeQ2",
                         "PrevBrakeQ2",
                         "NextBrakeQ2",
                         "BrakeQ3",
                         "PrevBrakeQ3",
                         "NextBrakeQ3",
                         "SteerAvg",
                         "PrevSteerAvg",
                         "NextSteerAvg",
                         "SteerStd",
                         "PrevSteerStd",
                         "NextSteerStd",
                         "SteerQ1",
                         "PrevSteerQ1",
                         "NextSteerQ1",
                         "SteerQ2",
                         "PrevSteerQ2",
                         "NextSteerQ2",
                         "SteerQ3",
                         "PrevSteerQ3",
                         "NextSteerQ3",
                         "OffsetAvg",
                         "PrevOffsetAvg",
                         "NextOffsetAvg",
                         "OffsetStd",
                         "PrevOffsetStd",
                         "NextOffsetStd",
                         "OffsetQ1",
                         "PrevOffsetQ1",
                         "NextOffsetQ1",
                         "OffsetQ2",
                         "PrevOffsetQ2",
                         "NextOffsetQ2",
                         "OffsetQ3",
                         "PrevOffsetQ3",
                         "NextOffsetQ3",
                         "GearAvg",
                         "PrevGearAvg",
                         "NextGearAvg",
                         "GearStd",
                         "PrevGearStd",
                         "NextGearStd",
                         "GearQ1",
                         "PrevGearQ1",
                         "NextGearQ1",
                         "GearQ2",
                         "PrevGearQ2",
                         "NextGearQ2",
                         "GearQ3",
                         "PrevGearQ3",
                         "NextGearQ3",
                         "SpeedAvg",
                         "PrevSpeedAvg",
                         "NextSpeedAvg",
                         "SpeedStd",
                         "PrevSpeedStd",
                         "NextSpeedStd",
                         "SpeedQ1",
                         "PrevSpeedQ1",
                         "NextSpeedQ1",
                         "SpeedQ2",
                         "PrevSpeedQ2",
                         "NextSpeedQ2",
                         "SpeedQ3",
                         "PrevSpeedQ3",
                         "NextSpeedQ3",
                         "Lap position"])
        block = -1
        blockIndex = 0
        while blockIndex < len(blockTypes):
            if fillOvertakes:
                writer.writerow([trackName,
                                 blockTypes[blockIndex],
                                 blockTypes[blockIndex-1],
                                 blockTypes[(blockIndex+1)%len(blockTypes)],
                                 blockOvertakes[blockIndex],
                                 blockLengths[blockIndex],
                                 '{:.20f}'.format(blockRadiuses[blockIndex]),
                                 '{:.20f}'.format(blockRadiuses[blockIndex-1]),
                                 '{:.20f}'.format(blockRadiuses[(blockIndex+1)%len(blockRadiuses)]),
                                 '{:.20f}'.format(prevRadiusesMeans[blockIndex]),
                                 '{:.20f}'.format(nextRadiusesMeans[blockIndex]),
                                 '{:.20f}'.format(prevRad400[blockIndex]),
                                 '{:.20f}'.format(nextRad400[blockIndex]),
                                 '{:.20f}'.format(prevRad200[blockIndex]),
                                 '{:.20f}'.format(nextRad200[blockIndex]),
                                 '{:.20f}'.format(prevRad100[blockIndex]),
                                 '{:.20f}'.format(nextRad100[blockIndex]),
                                 '{:.20f}'.format(prevRad50[blockIndex]),
                                 '{:.20f}'.format(nextRad50[blockIndex]),
                                 '{:.20f}'.format(blockGrade[blockIndex]),
                                 '{:.20f}'.format(blockGrade[blockIndex-1]),
                                 '{:.20f}'.format(blockGrade[(blockIndex+1)%len(blockGrade)]),
                                 '{:.20f}'.format(width[blockIndex]),
                                 '{:.20f}'.format(width[blockIndex - 1]),
                                 '{:.20f}'.format(width[(blockIndex + 1) % len(width)]),
                                 '{:.20f}'.format(accel_avg[blockIndex]),
                                 '{:.20f}'.format(accel_avg[blockIndex-1]),
                                 '{:.20f}'.format(accel_avg[(blockIndex+1)%len(accel_avg)]),
                                 '{:.20f}'.format(accel_std[blockIndex]),
                                 '{:.20f}'.format(accel_std[blockIndex - 1]),
                                 '{:.20f}'.format(accel_std[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q1[blockIndex]),
                                 '{:.20f}'.format(accel_q1[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q1[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q2[blockIndex]),
                                 '{:.20f}'.format(accel_q2[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q2[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q3[blockIndex]),
                                 '{:.20f}'.format(accel_q3[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q3[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(brake_avg[blockIndex]),
                                 '{:.20f}'.format(brake_avg[blockIndex - 1]),
                                 '{:.20f}'.format(brake_avg[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_std[blockIndex]),
                                 '{:.20f}'.format(brake_std[blockIndex - 1]),
                                 '{:.20f}'.format(brake_std[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q1[blockIndex]),
                                 '{:.20f}'.format(brake_q1[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q1[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q2[blockIndex]),
                                 '{:.20f}'.format(brake_q2[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q2[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q3[blockIndex]),
                                 '{:.20f}'.format(brake_q3[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q3[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(steer_avg[blockIndex]),
                                 '{:.20f}'.format(steer_avg[blockIndex - 1]),
                                 '{:.20f}'.format(steer_avg[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_std[blockIndex]),
                                 '{:.20f}'.format(steer_std[blockIndex - 1]),
                                 '{:.20f}'.format(steer_std[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q1[blockIndex]),
                                 '{:.20f}'.format(steer_q1[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q1[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q2[blockIndex]),
                                 '{:.20f}'.format(steer_q2[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q2[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q3[blockIndex]),
                                 '{:.20f}'.format(steer_q3[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q3[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(offset_avg[blockIndex]),
                                 '{:.20f}'.format(offset_avg[blockIndex - 1]),
                                 '{:.20f}'.format(offset_avg[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_std[blockIndex]),
                                 '{:.20f}'.format(offset_std[blockIndex - 1]),
                                 '{:.20f}'.format(offset_std[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q1[blockIndex]),
                                 '{:.20f}'.format(offset_q1[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q1[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q2[blockIndex]),
                                 '{:.20f}'.format(offset_q2[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q2[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q3[blockIndex]),
                                 '{:.20f}'.format(offset_q3[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q3[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(gear_avg[blockIndex]),
                                 '{:.20f}'.format(gear_avg[blockIndex - 1]),
                                 '{:.20f}'.format(gear_avg[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_std[blockIndex]),
                                 '{:.20f}'.format(gear_std[blockIndex - 1]),
                                 '{:.20f}'.format(gear_std[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q1[blockIndex]),
                                 '{:.20f}'.format(gear_q1[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q1[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q2[blockIndex]),
                                 '{:.20f}'.format(gear_q2[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q2[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q3[blockIndex]),
                                 '{:.20f}'.format(gear_q3[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q3[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(speed_avg[blockIndex]),
                                 '{:.20f}'.format(speed_avg[blockIndex - 1]),
                                 '{:.20f}'.format(speed_avg[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_std[blockIndex]),
                                 '{:.20f}'.format(speed_std[blockIndex - 1]),
                                 '{:.20f}'.format(speed_std[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q1[blockIndex]),
                                 '{:.20f}'.format(speed_q1[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q1[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q2[blockIndex]),
                                 '{:.20f}'.format(speed_q2[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q2[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q3[blockIndex]),
                                 '{:.20f}'.format(speed_q3[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q3[(blockIndex + 1) % len(speed_avg)]),
                                 blockEndLengths[blockIndex] / trackLength])
            else:
                writer.writerow([trackName,
                                 blockTypes[blockIndex],
                                 blockTypes[blockIndex - 1],
                                 blockTypes[(blockIndex + 1) % len(blockTypes)],
                                 "",
                                 blockLengths[blockIndex],
                                 '{:.20f}'.format(blockRadiuses[blockIndex]),
                                 '{:.20f}'.format(blockRadiuses[blockIndex - 1]),
                                 '{:.20f}'.format(blockRadiuses[(blockIndex+1)%len(blockRadiuses)]),
                                 '{:.20f}'.format(prevRadiusesMeans[blockIndex]),
                                 '{:.20f}'.format(nextRadiusesMeans[blockIndex]),
                                 '{:.20f}'.format(prevRad400[blockIndex]),
                                 '{:.20f}'.format(nextRad400[blockIndex]),
                                 '{:.20f}'.format(prevRad200[blockIndex]),
                                 '{:.20f}'.format(nextRad200[blockIndex]),
                                 '{:.20f}'.format(prevRad100[blockIndex]),
                                 '{:.20f}'.format(nextRad100[blockIndex]),
                                 '{:.20f}'.format(prevRad50[blockIndex]),
                                 '{:.20f}'.format(nextRad50[blockIndex]),
                                 '{:.20f}'.format(blockGrade[blockIndex]),
                                 '{:.20f}'.format(blockGrade[blockIndex - 1]),
                                 '{:.20f}'.format(blockGrade[(blockIndex+1)%len(blockGrade)]),
                                 '{:.20f}'.format(width[blockIndex]),
                                 '{:.20f}'.format(width[blockIndex - 1]),
                                 '{:.20f}'.format(width[(blockIndex + 1) % len(width)]),
                                 '{:.20f}'.format(accel_avg[blockIndex]),
                                 '{:.20f}'.format(accel_avg[blockIndex - 1]),
                                 '{:.20f}'.format(accel_avg[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_std[blockIndex]),
                                 '{:.20f}'.format(accel_std[blockIndex - 1]),
                                 '{:.20f}'.format(accel_std[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q1[blockIndex]),
                                 '{:.20f}'.format(accel_q1[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q1[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q2[blockIndex]),
                                 '{:.20f}'.format(accel_q2[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q2[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(accel_q3[blockIndex]),
                                 '{:.20f}'.format(accel_q3[blockIndex - 1]),
                                 '{:.20f}'.format(accel_q3[(blockIndex + 1) % len(accel_avg)]),
                                 '{:.20f}'.format(brake_avg[blockIndex]),
                                 '{:.20f}'.format(brake_avg[blockIndex - 1]),
                                 '{:.20f}'.format(brake_avg[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_std[blockIndex]),
                                 '{:.20f}'.format(brake_std[blockIndex - 1]),
                                 '{:.20f}'.format(brake_std[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q1[blockIndex]),
                                 '{:.20f}'.format(brake_q1[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q1[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q2[blockIndex]),
                                 '{:.20f}'.format(brake_q2[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q2[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(brake_q3[blockIndex]),
                                 '{:.20f}'.format(brake_q3[blockIndex - 1]),
                                 '{:.20f}'.format(brake_q3[(blockIndex + 1) % len(brake_avg)]),
                                 '{:.20f}'.format(steer_avg[blockIndex]),
                                 '{:.20f}'.format(steer_avg[blockIndex - 1]),
                                 '{:.20f}'.format(steer_avg[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_std[blockIndex]),
                                 '{:.20f}'.format(steer_std[blockIndex - 1]),
                                 '{:.20f}'.format(steer_std[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q1[blockIndex]),
                                 '{:.20f}'.format(steer_q1[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q1[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q2[blockIndex]),
                                 '{:.20f}'.format(steer_q2[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q2[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(steer_q3[blockIndex]),
                                 '{:.20f}'.format(steer_q3[blockIndex - 1]),
                                 '{:.20f}'.format(steer_q3[(blockIndex + 1) % len(steer_avg)]),
                                 '{:.20f}'.format(offset_avg[blockIndex]),
                                 '{:.20f}'.format(offset_avg[blockIndex - 1]),
                                 '{:.20f}'.format(offset_avg[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_std[blockIndex]),
                                 '{:.20f}'.format(offset_std[blockIndex - 1]),
                                 '{:.20f}'.format(offset_std[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q1[blockIndex]),
                                 '{:.20f}'.format(offset_q1[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q1[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q2[blockIndex]),
                                 '{:.20f}'.format(offset_q2[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q2[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(offset_q3[blockIndex]),
                                 '{:.20f}'.format(offset_q3[blockIndex - 1]),
                                 '{:.20f}'.format(offset_q3[(blockIndex + 1) % len(offset_avg)]),
                                 '{:.20f}'.format(gear_avg[blockIndex]),
                                 '{:.20f}'.format(gear_avg[blockIndex - 1]),
                                 '{:.20f}'.format(gear_avg[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_std[blockIndex]),
                                 '{:.20f}'.format(gear_std[blockIndex - 1]),
                                 '{:.20f}'.format(gear_std[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q1[blockIndex]),
                                 '{:.20f}'.format(gear_q1[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q1[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q2[blockIndex]),
                                 '{:.20f}'.format(gear_q2[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q2[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(gear_q3[blockIndex]),
                                 '{:.20f}'.format(gear_q3[blockIndex - 1]),
                                 '{:.20f}'.format(gear_q3[(blockIndex + 1) % len(gear_avg)]),
                                 '{:.20f}'.format(speed_avg[blockIndex]),
                                 '{:.20f}'.format(speed_avg[blockIndex - 1]),
                                 '{:.20f}'.format(speed_avg[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_std[blockIndex]),
                                 '{:.20f}'.format(speed_std[blockIndex - 1]),
                                 '{:.20f}'.format(speed_std[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q1[blockIndex]),
                                 '{:.20f}'.format(speed_q1[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q1[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q2[blockIndex]),
                                 '{:.20f}'.format(speed_q2[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q2[(blockIndex + 1) % len(speed_avg)]),
                                 '{:.20f}'.format(speed_q3[blockIndex]),
                                 '{:.20f}'.format(speed_q3[blockIndex - 1]),
                                 '{:.20f}'.format(speed_q3[(blockIndex + 1) % len(speed_avg)]),
                                 blockEndLengths[blockIndex] / trackLength])
            blockIndex += 1

        print("  -> Created " + fileName)



def dist2seg(dist, trackData):
    segIndex = 0
    curDist = trackData[segIndex][blocks.LENGTH]

    while dist > curDist:
        segIndex += 1
        curDist += trackData[segIndex][blocks.LENGTH]

    return trackData[segIndex]



def drawCarPositions(trackData, logFile, trackName):
    records = []
    width = getTrackWidth(trackName)
    playerFound = False
    
    for line in subprocess.check_output('grep overtake "' + logFile + '"', shell = True).splitlines():
        if int(line.decode("ascii").split(",")[5]) == 0 and int(line.decode("ascii").split(",")[11]) == 0 and int(line.decode("ascii").split(",")[8]) > 1:
            # Both cars are racing regularly (no one is retired or in pit) and the overtaker is at least at the second lap
            record = []
            record.append(line.decode("ascii").split(",")[0])
            record.append(line.decode("ascii").split(",")[3])   # overtakee
            record.append(line.decode("ascii").split(",")[9])   # overtaken
            records.append(record)
    
    # overtakee
    for rec in records:
        output = subprocess.check_output('grep "' + rec[0] + ',' + rec[1] + '" "' + logFile + '"', shell = True).splitlines()
        xy = RtTrackLocal2Global(dist2seg(float(output[0].decode("ascii").split(",")[11]), trackData), float(output[0].decode("ascii").split(",")[6]), float(output[0].decode("ascii").split(",")[8]), width)
        if output[0].decode("ascii").split(",")[1] == "Player":
            plt.plot(xy[0], xy[1], color = "#007700", marker = "+", markersize = 5)
            playerFound = True
        else:
            plt.plot(xy[0], xy[1], color = "#5555ff", marker = "+", markersize = 5)
    
    # overtaken
    for rec in records:
        output = subprocess.check_output('grep "' + rec[0] + ',' + rec[2] + '" "' + logFile + '"', shell = True).splitlines()
        xy = RtTrackLocal2Global(dist2seg(float(output[0].decode("ascii").split(",")[11]), trackData), float(output[0].decode("ascii").split(",")[6]), float(output[0].decode("ascii").split(",")[8]), width)
        if output[0].decode("ascii").split(",")[1] == "Player":
            plt.plot(xy[0], xy[1], color = "#666666", marker = "+", markersize = 5)
            playerFound = True
        else:
            plt.plot(xy[0], xy[1], "k+", markersize = 5)

    return playerFound



def plotOvertakes(blockData, plotCarPositions, folder, logList, path, trackName):
    blocks.plotMetric(blockData, computeBlockOvertakes(blockData), "Number of overtakes", path + ".svg", True, False)

    if plotCarPositions:
        playerFound = False

        for log in logList:
            playerFoundInCurrentLog = drawCarPositions(blockData, folder + "/" + log, trackName)
            playerFound = playerFound or playerFoundInCurrentLog

        patches = [matplotlib.patches.Patch(color = "#5555ff"), matplotlib.patches.Patch(color = "k"), matplotlib.patches.Patch(color = "#007700"), matplotlib.patches.Patch(color = "#666666")]

        if playerFound:
            legend = plt.legend(patches, ["Overtakee", "Overtaken", "Human overtakee", "Human overtaken"], loc = 8, bbox_to_anchor=(0.5, -0.5))
        else:
            legend = plt.legend(patches, ["Overtakee", "Overtaken"], loc = 8, bbox_to_anchor=(0.5, -0.5))

        plt.savefig(path + "-with-cars.svg", bbox_extra_artists = (legend,), bbox_inches = "tight")
        print("  -> Created " + path.split("/")[-2] + "/" + path.split("/")[-1] + "-with-cars.svg")
    
    plt.clf()



def makeOvertakePlotsAndSegmentDataFile(folder, logList, trackDirectory, trackName, maxBlockLength, plotCarPositions):
    overtakesFolder = folder + "/overtakes"
    if not os.path.exists(overtakesFolder):
        os.makedirs(overtakesFolder)

    allBlockData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    allBlockData = blocks.readDynamics(trackName, allBlockData)

    for index, log in enumerate(logList):
        print("==> Analyzing " + log + " (" + str(index + 1) + "/" + str(len(logList)) + ")")
        
        trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
        trackData = fillOvertakes(trackData, folder + "/" + log)
        allBlockData = fillOvertakes(allBlockData, folder + "/" + log)

        plotOvertakes(trackData, plotCarPositions, folder, [log], overtakesFolder + "/" + log, trackName)

    if len(logList) > 1:
        print("==> Analyzing all the races together")
        plotOvertakes(allBlockData, plotCarPositions, folder, logList, overtakesFolder + "/overtakes", trackName)
    
    makeBlocksDataFile(allBlockData, trackName, folder, True, maxBlockLength)



def makeBlocksDataWithoutOvertakes(folder, trackDirectory, trackName, maxBlockLength):
    trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    trackData = blocks.readDynamics(trackName, trackData)
    makeBlocksDataFile(trackData, trackName, folder, False, maxBlockLength)
