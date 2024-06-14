import { VoronoiTrackGenerator } from '../trackGen/voronoiTrackGenerator.js';
import { ConvexHullTrackGenerator } from '../trackGen/convexHullTrackGenerator.js';
import * as utils from '../utils/utils.js';

export function generateTrack(mode, bbox, seed, trackSize) {
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

    const segmentLength = 10;
    const minIndex = utils.findMaxCurveBeforeStraight(splineTrack, segmentLength);
    const track = splineTrack.slice(minIndex).concat(splineTrack.slice(0, minIndex));

    return track;
}