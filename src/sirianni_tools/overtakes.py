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
from utils import SegmentType

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


def fillOvertakes(trackData, logFile):
    try:
        with open(logFile, 'r') as f:
            lines = f.readlines()
            overtake_count = 0
            for line in lines:
                if 'overtake' in line:
                    parts = line.strip().split(',')
                    if len(parts) >= 3:
                        try:
                            length = float(parts[2])  # Distance where overtake occurred
                            for seg in trackData:
                                if seg[blocks.ENDLENGTH] >= length:
                                    seg[blocks.OVERTAKES] += 1
                                    overtake_count += 1
                                    break
                        except ValueError:
                            print(f"Warning: Invalid length value in line: {line.strip()}")
                    else:
                        print(f"Warning: Not enough data fields in line: {line.strip()}")
            print(f"Processed {overtake_count} overtakes in {logFile}")
        return trackData, overtake_count
    except Exception as e:
        print(f"Error processing overtakes: {e}")
        return trackData, 0



def computeBlockOvertakes(trackData):
    """Compute overtakes per block"""
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


def drawCarPositions(trackData, logFile, trackName):
    records = []
    width = getTrackWidth(trackName)
    playerFound = False
    
    try:
        with open(logFile, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            if 'overtake' in line:
                parts = line.strip().split(',')
                if len(parts) >= 15:  # Ensure we have enough fields
                    record = []
                    record.append(parts[0])  # timestamp
                    record.append(parts[3])  # overtaker
                    record.append(parts[8])  # overtaken
                    records.append(record)

        # Process records for position plotting
        for rec in records:
            timestamp = rec[0]
            overtaker = rec[1]
            overtaken = rec[2]
            
            overtaker_line = None
            overtaken_line = None
            
            for line in lines:
                parts = line.strip().split(',')
                if len(parts) > 11:
                    if parts[1] == overtaker and abs(float(parts[0]) - float(timestamp)) < 0.1:
                        overtaker_line = parts
                    elif parts[1] == overtaken and abs(float(parts[0]) - float(timestamp)) < 0.1:
                        overtaken_line = parts
                
                if overtaker_line and overtaken_line:
                    break
            
            if overtaker_line:
                try:
                    xy = rtTrackLocal2Global(
                        dist2Seg(float(overtaker_line[11]), trackData),
                        float(overtaker_line[6]),
                        float(overtaker_line[8]),
                        width
                    )
                    
                    if overtaker_line[1] == "player":
                        plt.plot(xy[0], xy[1], color="#007700", marker="+", markersize=5)
                        playerFound = True
                    else:
                        plt.plot(xy[0], xy[1], color="#5555ff", marker="+", markersize=5)
                except Exception as e:
                    print(f"Warning: Could not plot overtaker position: {e}")

            if overtaken_line:
                try:
                    xy = rtTrackLocal2Global(
                        dist2Seg(float(overtaken_line[11]), trackData),
                        float(overtaken_line[6]),
                        float(overtaken_line[8]),
                        width
                    )
                    
                    if overtaken_line[1] == "player":
                        plt.plot(xy[0], xy[1], color="#666666", marker="+", markersize=5)
                        playerFound = True
                    else:
                        plt.plot(xy[0], xy[1], "k+", markersize=5)
                except Exception as e:
                    print(f"Warning: Could not plot overtaken position: {e}")

        return playerFound
        
    except Exception as e:
        print(f"Warning: Error processing log file {logFile}: {e}")
        return False

def makeOvertakePlotsAndSegmentDataFile(folder, logList, trackDirectory, trackName, maxBlockLength):
    """Main function to analyze and plot overtakes"""
    overtakesFolder = os.path.join(folder, "overtakes")
    if not os.path.exists(overtakesFolder):
        os.makedirs(overtakesFolder)

    # Get base track data
    allBlockData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    allBlockData = blocks.readDynamics(trackName, allBlockData)

    # Process each log file
    totalOvertakes = 0
    for index, log in enumerate(logList):
        print(f"==> Analyzing {log} ({index + 1}/{len(logList)})")
        
        # Get fresh track data for this log
        trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
        if trackData is None:
            continue
            
        # Fill in overtakes data
        trackData, overtake_count = fillOvertakes(trackData, os.path.join(folder, log))
        totalOvertakes += overtake_count
        allBlockData, _ = fillOvertakes(allBlockData, os.path.join(folder, log))

        # Create plots for this log
        plotOvertakes(trackData, folder, [log], 
                     os.path.join(overtakesFolder, log), trackName)

    # Process combined data if we have multiple logs
    if len(logList) > 1:
        print("==> Analyzing races together")
        plotOvertakes(allBlockData, folder, logList,
                     os.path.join(overtakesFolder, "overtakes"), trackName)

    # Create final data file
    makeBlocksDataFile(allBlockData, trackName, folder, True, maxBlockLength)
    return totalOvertakes

def plotOvertakes(blockData, folder, logList, path, trackName):
    blocks.plotMetric(blockData, computeBlockOvertakes(blockData), "Number of overtakes", path + ".svg", True, False)


def makeBlocksDataWithoutOvertakes(folder, trackDirectory, trackName, maxBlockLength):
    trackData = blocks.readTrack(trackDirectory, trackName, maxBlockLength)
    trackData = blocks.readDynamics(trackName, trackData)
    makeBlocksDataFile(trackData, trackName, folder, False, maxBlockLength)


def rtTrackLocal2Global(seg, toStart, toMiddle, width):
    """Convert local track coordinates to global coordinates.
    The segment types in track data are:
    1 = LEFT turn
    2 = RIGHT turn
    3 = STRAIGHT
    """
    import math
    
    try:
        seg_type = int(seg[blocks.TYPE])
        
        # Straight segment (type 3)
        if seg_type == SegmentType.straight.value:
            CosA = math.cos(float(seg[blocks.ANGLE_ZS]))
            SinA = math.sin(float(seg[blocks.ANGLE_ZS]))
            tr = toMiddle + width / 2
            x = float(seg[blocks.SRX]) + toStart * CosA - tr * SinA
            y = float(seg[blocks.SRY]) + toStart * SinA + tr * CosA
            
        # Left turn (type 1)
        elif seg_type == 1:
            a = float(seg[blocks.ANGLE_ZS]) + toStart / float(seg[blocks.RADIUS])
            r = float(seg[blocks.RADIUS]) - toMiddle
            x = float(seg[blocks.CENTER_X]) + r * math.sin(a)
            y = float(seg[blocks.CENTER_Y]) - r * math.cos(a)
            
        # Right turn (type 2)
        elif seg_type == 2:
            a = float(seg[blocks.ANGLE_ZS]) - toStart / float(seg[blocks.RADIUS])
            r = float(seg[blocks.RADIUS]) + toMiddle
            x = float(seg[blocks.CENTER_X]) - r * math.sin(a)
            y = float(seg[blocks.CENTER_Y]) + r * math.cos(a)
        
        return (x, y)
        
    except Exception as e:
        print(f"Error in RtTrackLocal2Global: {str(e)}")
        return (0, 0)


def dist2Seg(dist, trackData):
    try:
        segIndex = 0
        curDist = trackData[segIndex][blocks.LENGTH]
        
        # Keep accumulating distance until we find the right segment
        # or reach the end of the track
        while dist > curDist and segIndex < len(trackData) - 1:
            segIndex += 1
            curDist += trackData[segIndex][blocks.LENGTH]
            
        return trackData[segIndex]
    except Exception as e:
        print(f"Warning: Error in dist2seg: {e}")
        return None