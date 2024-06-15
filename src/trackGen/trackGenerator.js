import { VoronoiTrackGenerator } from '../trackGen/voronoiTrackGenerator.js';
import { ConvexHullTrackGenerator } from '../trackGen/convexHullTrackGenerator.js';
import * as utils from '../utils/utils.js';
import { savePointsToJson } from '../utils/jsonUtils.js';

export function generateTrack(mode, bbox, seed, trackSize, saveJSON = false) {
    if (isNaN(seed)) seed = Math.random();

    let trackGenerator;
    switch (mode) {
        case 'voronoi':
            trackGenerator = new VoronoiTrackGenerator(bbox, seed, trackSize);
            break;
        case 'convexHull':
            trackGenerator = new ConvexHullTrackGenerator(bbox, seed, trackSize);
            break;
        default:
            throw new Error('Invalid track generator mode');
    }

    let splineTrack = utils.splineSmoothing(trackGenerator.trackEdges);

    //process to reduce the approximation error using "findMaxCurveBeforeStraight" heuristic
    const segmentLength = 10;
    const minIndex = utils.findMaxCurveBeforeStraight(splineTrack, segmentLength);
    splineTrack.slice(minIndex).concat(splineTrack.slice(0, minIndex));
    
    if (saveJSON && mode === 'voronoi') {
        savePointsToJson(seed, trackGenerator.dataSet);
        return splineTrack;
    } else {
        return splineTrack;
    }
}