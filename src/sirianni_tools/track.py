"""Analyze track topology and plot track's data."""

import csv
import os
import matplotlib.pyplot as plt
import scipy.stats
import utils  # Ensure utils module is available

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"


def verify_track_file(trackfile):
    """
    Verify track file exists and is readable.
    Returns True if file is valid, False otherwise.
    """
    if not os.path.exists(trackfile):
        print(f"Warning: Track file not found: {trackfile}")
        return False
    if not os.path.isfile(trackfile):
        print(f"Warning: Path is not a file: {trackfile}")
        return False
    if os.path.getsize(trackfile) == 0:
        print(f"Warning: Track file is empty: {trackfile}")
        return False
    return True


def get_segment_coordinates(row, utils):
    """
    Safely extract and convert segment coordinates from a CSV row.
    Returns tuple of (x_coords, y_coords) as float lists.
    """
    try:
        # Extract coordinate indices from utils.SegmentData
        x_indices = [
            utils.SegmentData.slx.value,
            utils.SegmentData.srx.value,
            utils.SegmentData.erx.value,
            utils.SegmentData.elx.value
        ]
        
        y_indices = [
            utils.SegmentData.sly.value,
            utils.SegmentData.sry.value,
            utils.SegmentData.ery.value,
            utils.SegmentData.ely.value
        ]
        
        # Verify row has enough elements
        if len(row) <= max(max(x_indices), max(y_indices)):
            print(f"Warning: Row too short, expected at least {max(max(x_indices), max(y_indices))} elements, got {len(row)}")
            return None, None
            
        # Convert to float and create coordinate lists
        x_coords = [float(row[i]) for i in x_indices]
        y_coords = [float(row[i]) for i in y_indices]
        
        return x_coords, y_coords
    except (ValueError, IndexError) as e:
        print(f"Warning: Error processing segment coordinates: {e}")
        return None, None


def readtrack(trackfile):
    """
    Read track data from CSV file.
    Returns list of track metrics or None if file cannot be read.
    """
    if not verify_track_file(trackfile):
        return None
        
    try:
        with open(trackfile, "r") as csvfile:
            isFirstRow = True
            leftBendsCount = 0
            rightBendsCount = 0
            straightsCount = 0
            straightsLength = 0
            previousSegmentType = utils.SegmentType.none
            radiuses = []
            heights = []
            analyzedLength = 0
            
            # Initialize partial lap counters
            leftBendsCount30pc = leftBendsCount50pc = 0
            rightBendsCount30pc = rightBendsCount50pc = 0
            straightsCount30pc = straightsCount50pc = 0
            straightsLength30pc = straightsLength50pc = 0
            radiuses30pc = []
            radiuses50pc = []
            heights30pc = []
            heights50pc = []
            
            for row in csv.reader(csvfile):
                if isFirstRow:
                    try:
                        length = float(row[0])
                        width = float(row[1])
                    except (ValueError, IndexError) as e:
                        print(f"Error reading track dimensions: {e}")
                        return None
                    isFirstRow = False
                    continue
                
                try:
                    # Process segment data
                    segment_type = int(row[utils.SegmentData.type.value])
                    radius = float(row[utils.SegmentData.radius.value])
                    seg_length = float(row[utils.SegmentData.length.value])
                    
                    if segment_type != utils.SegmentType.straight.value and radius < utils.maxBendRadius:
                        segmentType = segment_type
                    else:
                        segmentType = utils.SegmentType.straight.value
                    
                    analyzedLength += seg_length
                    
                    # Update metrics based on segment type
                    if segmentType == utils.SegmentType.straight.value:
                        straightsLength += seg_length
                        if analyzedLength <= length / 3:
                            straightsLength30pc += seg_length
                        if analyzedLength <= length / 2:
                            straightsLength50pc += seg_length
                    else:
                        radiuses.append(radius)
                        if analyzedLength <= length / 3:
                            radiuses30pc.append(radius)
                        if analyzedLength <= length / 2:
                            radiuses50pc.append(radius)
                    
                    # Update bend counts
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
                    
                    # Process heights
                    height = (float(row[utils.SegmentData.slz.value]) + 
                             float(row[utils.SegmentData.srz.value])) / 2
                    heights.append(height)
                    if analyzedLength <= length / 3:
                        heights30pc.append(height)
                    if analyzedLength <= length / 2:
                        heights50pc.append(height)
                        
                except (ValueError, IndexError) as e:
                    print(f"Warning: Error processing segment: {e}")
                    continue
            
            # Prepare return data
            trackData = [
                length, width,
                leftBendsCount, rightBendsCount, straightsCount,
                straightsLength, radiuses, heights,
                analyzedLength,  # length30pc
                leftBendsCount30pc, rightBendsCount30pc,
                straightsCount30pc, straightsLength30pc,
                radiuses30pc, heights30pc,
                analyzedLength,  # length50pc
                leftBendsCount50pc, rightBendsCount50pc,
                straightsCount50pc, straightsLength50pc,
                radiuses50pc, heights50pc
            ]
            
            return trackData
            
    except Exception as e:
        print(f"Error reading track file: {e}")
        return None


def plotsegmenttypes(trackfile, filename, utils):
    """Plot track segments colored by their type."""
    if not verify_track_file(trackfile):
        return
        
    plt.figure(figsize=(12, 8))
    plt.axis("off")
    plt.gca().set_aspect("equal", adjustable="box")
    
    with open(trackfile, "r") as csvfile:
        firstrow = True
        for row in csv.reader(csvfile):
            if firstrow:
                firstrow = False
                continue
            
            try:
                x_coords, y_coords = get_segment_coordinates(row, utils)
                if x_coords is None or y_coords is None:
                    continue
                
                segment_type = int(row[utils.SegmentData.type.value])
                radius = float(row[utils.SegmentData.radius.value])
                
                if segment_type == utils.SegmentType.left.value and radius < utils.maxBendRadius:
                    color = "r"
                elif segment_type == utils.SegmentType.right.value and radius < utils.maxBendRadius:
                    color = "g"
                else:
                    color = "k"
                
                plt.fill(x_coords, y_coords, color, edgecolor="none")
            except Exception as e:
                print(f"Warning: Error plotting segment: {e}")
                continue
    
    # Create legend
    redsquare = plt.Line2D(range(10), range(10), color="r", linewidth=2)
    greensquare = plt.Line2D(range(10), range(10), color="g", linewidth=2)
    blacksquare = plt.Line2D(range(10), range(10), color="k", linewidth=2)
    legend = plt.legend([redsquare, greensquare, blacksquare], 
                       ["Left", "Right", "Straight"], 
                       loc=8, 
                       bbox_to_anchor=(0.5, -0.5))
    
    try:
        plt.savefig(filename, bbox_extra_artists=(legend,), bbox_inches="tight", dpi=300)
        print(" -> Created " + filename.split("/")[-2] + "/" + filename.split("/")[-1])
    except Exception as e:
        print(f"Error saving figure: {e}")
    finally:
        plt.close()


def plotsegments(trackfile, outputpath, color1, color2):
    """Plot track segments alternating between two colors."""
    if not verify_track_file(trackfile):
        return
        
    plt.figure(figsize=(12, 8))
    plt.axis("off")
    plt.gca().set_aspect("equal", adjustable="box")
    
    with open(trackfile, "r") as csvfile:
        index = 0
        firstrow = True
        for row in csv.reader(csvfile):
            if firstrow:
                firstrow = False
                continue
            
            try:
                x_coords, y_coords = get_segment_coordinates(row, utils)
                if x_coords is None or y_coords is None:
                    continue
                
                color = color1 if index % 2 == 0 else color2
                plt.fill(x_coords, y_coords, color, edgecolor="none")
                index += 1
            except Exception as e:
                print(f"Warning: Error plotting segment: {e}")
                continue
    
    try:
        plt.savefig(outputpath, bbox_inches="tight", dpi=300)
        print(" -> Created " + outputpath.split("/")[-2] + "/" + outputpath.split("/")[-1])
    except Exception as e:
        print(f"Error saving figure: {e}")
    finally:
        plt.close()


def analyzeTrack(trackfile, outputfolder, generate_plots=True):
    """Main entry point for track analysis."""
    if not verify_track_file(trackfile):
        return None
        
    # Create output folder if needed
    if generate_plots and not os.path.exists(outputfolder):
        try:
            os.makedirs(outputfolder)
        except Exception as e:
            print(f"Error creating output folder: {e}")
            return None
    
    # Read and analyze track data
    trackdata = readtrack(trackfile)
    if trackdata is None:
        return None
    
    # Generate plots if requested
    if generate_plots:
        plotsegmenttypes(trackfile, os.path.join(outputfolder, "segment-types.svg"), utils)
        plotsegments(trackfile, os.path.join(outputfolder, "segments.svg"), "k", "#666666")
        plottrack(trackfile, os.path.join(outputfolder, "track.svg"))
    
    return trackdata


def plottrack(trackfile, outputpath):
    """Plot entire track in solid black."""
    if not verify_track_file(trackfile):
        return
        
    plt.figure(figsize=(12, 8))
    plt.axis("off")
    plt.gca().set_aspect("equal", adjustable="box")
    
    with open(trackfile, "r") as csvfile:
        firstrow = True
        for row in csv.reader(csvfile):
            if firstrow:
                firstrow = False
                continue
            
            try:
                x_coords, y_coords = get_segment_coordinates(row, utils)
                if x_coords is None or y_coords is None:
                    continue
                
                plt.fill(x_coords, y_coords, "k", edgecolor="none")
            except Exception as e:
                print(f"Warning: Error plotting segment: {e}")
                continue
    
    try:
        plt.savefig(outputpath, bbox_inches="tight", dpi=300)
        print(" -> Created " + outputpath.split("/")[-2] + "/" + outputpath.split("/")[-1])
    except Exception as e:
        print(f"Error saving figure: {e}")
    finally:
        plt.close()