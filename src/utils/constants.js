// constants.js
export const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
export const MODE = 'convexHull'; // voronoi or convexHull
export const NUMBER_OF_VORONOI_SITES = 100;
export const DOCKER_IMAGE_NAME = 'torcs';
export const MAPELITE_PATH = '../utils/mapelite.xml';
export const MEMORY_LIMIT = '2000m';
export const OUTPUT_DIR = '../data/convexHull'
export const JSON_DEBUG = true;
export const SIMULATION_TIMEOUT = 60000;

export const COLORS = {
      SEPARATION_LINE: [255,0,0],
      VORONOI: [180, 192, 165],    // Sage
      POINTS: [113, 131, 85],     // Reseda Green
      EDGES: [164, 176, 146],     // Sage
      BACKGROUND: [248, 248, 248],  // Light Gray (original background color)
      SPLINE: [
        [247, 37, 133],   // Rose
        [181, 23, 158],   // Fandango
        [114, 9, 183],    // Grape
        [76, 201, 240]    // Vivid Sky Blue
      ]
    };
