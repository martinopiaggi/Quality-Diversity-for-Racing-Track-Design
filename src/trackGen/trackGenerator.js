import { VoronoiTrackGenerator } from '../trackGen/voronoiTrackGenerator.js';
import { ConvexHullTrackGenerator } from '../trackGen/convexHullTrackGenerator.js';
import * as utils from '../utils/utils.js';

let trackGenerator;
let savePointsToJson;

async function importJsonUtils() {
    if (typeof window === 'undefined') {
      const module = await import('../utils/jsonUtils.js');
      savePointsToJson = module.savePointsToJson;
    }
}
  
export async function generateTrack(mode, bbox, seed, trackSize, saveJSON = false, dataSet = [], selectedVoronoiCells = []) {
    if (isNaN(seed)) seed = Math.random();
    
    switch (mode) {
        case 'voronoi':
            trackGenerator = new VoronoiTrackGenerator(bbox, seed, trackSize, dataSet,selectedVoronoiCells);
            break;
        case 'convexHull':
            trackGenerator = new ConvexHullTrackGenerator(bbox, seed, trackSize, dataSet);
            break;
        default:
            throw new Error('Invalid track generator mode');
    }

    let splineTrack = utils.splineSmoothing(trackGenerator.trackEdges);

    //process to reduce the approximation error using "findMaxCurveBeforeStraight" heuristic
    const segmentLength = 10;
    const minIndex = utils.findMaxCurveBeforeStraight(splineTrack, segmentLength);
    splineTrack.slice(minIndex).concat(splineTrack.slice(0, minIndex));
    
    if (saveJSON) {
        await importJsonUtils();
        if(mode === 'voronoi') 
            savePointsToJson(seed, trackGenerator.dataSet, trackGenerator.selectedCells.map(cell => cell.site));
        else
            savePointsToJson(seed, trackGenerator.dataSet);
    }

    return splineTrack;
}

export function getGenerator(){
    return trackGenerator;
}