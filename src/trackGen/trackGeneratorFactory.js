import { VoronoiTrackGenerator } from './voronoiTrackGenerator.js';
import ConvexHullTrackGenerator from './convexHullTrackGenerator.js';

export class TrackGeneratorFactory {
    static createTrackGenerator(type, bbox, seed, size) {
        switch (type) {
            case 'voronoi':
                return new VoronoiTrackGenerator(bbox, seed, size);
            case 'convexHull':
                return new ConvexHullTrackGenerator(bbox, seed, size);
            default:
                throw new Error('Invalid track generator type');
        }
    }
}

