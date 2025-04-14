// convertGenotype.js

import fs from 'fs/promises';
import path from 'path';
import { generateTrack } from '../src/trackGen/trackGenerator.js'; // Adjust the path as needed

// Define the bounding box as used in your website.
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };

/**
 * Resamples an array of points along the polyline to a fixed number of samples using linear interpolation.
 * @param {Array} points - Array of points with properties {x, y}.
 * @param {number} numSamples - The desired number of samples.
 * @returns {Array} - Resampled array of points.
 */
function resamplePoints(points, numSamples) {
  if (points.length < 2) return points;
  
  // Compute cumulative distances along the polyline.
  const distances = [0];
  for (let i = 1; i < points.length; i++) {
    const dx = points[i].x - points[i - 1].x;
    const dy = points[i].y - points[i - 1].y;
    distances.push(distances[i - 1] + Math.hypot(dx, dy));
  }
  
  const totalLength = distances[distances.length - 1];
  const resampled = [];
  
  // For each sample point, compute its target distance and interpolate.
  for (let i = 0; i < numSamples; i++) {
    const target = (i / (numSamples - 1)) * totalLength;
    let j = 1;
    while (j < distances.length && distances[j] < target) {
      j++;
    }
    const t = (target - distances[j - 1]) / (distances[j] - distances[j - 1]);
    const x = points[j - 1].x + t * (points[j].x - points[j - 1].x);
    const y = points[j - 1].y + t * (points[j].y - points[j - 1].y);
    resampled.push({ x, y });
  }
  
  return resampled;
}

/**
 * Main function: For each JSON file provided (or all JSON files in a directory),
 * load the genotype, generate and normalize the track, resample to a fixed-length spline vector,
 * insert it into the JSON, print, and rewrite the file.
 */
async function main() {
  // Read the first command-line argument (or default to ".").
  const inputArg = process.argv[2] || ".";
  let inputFiles = [];
  
  try {
    const stat = await fs.stat(inputArg);
    if (stat.isDirectory()) {
      // If a directory, list all .json files.
      const files = await fs.readdir(inputArg);
      inputFiles = files
        .filter(file => file.endsWith('.json'))
        .map(file => path.join(inputArg, file));
    } else {
      inputFiles = [inputArg];
    }
  } catch (err) {
    console.error("Error accessing input argument:", err);
    process.exit(1);
  }
  
  if (inputFiles.length === 0) {
    console.error("No JSON files found.");
    process.exit(1);
  }
  
  // Process each JSON file.
  for (const inputFilePath of inputFiles) {
    try {
      const data = await fs.readFile(inputFilePath, 'utf8');
      let genotype = JSON.parse(data);
      
      // Extract required properties from the genotype JSON.
      // Assume the genotype JSON includes: id, mode, trackSize, dataSet, and optionally selectedCells.
      const seed = genotype.id; // Using id as the seed.
      const mode = genotype.mode;
      const trackSize = genotype.trackSize;
      const dataSet = genotype.dataSet || [];
      const selectedCells = genotype.selectedCells || [];
      
      const trackResult = await generateTrack(mode, BBOX, seed, trackSize, false, dataSet, selectedCells);
      
      // Assume the generated track is available as trackResult.track (an array of points).
      let generatedTrack = trackResult.track;
      if (generatedTrack.length > 0) {
        const first = generatedTrack[0];
        const last = generatedTrack[generatedTrack.length - 1];
        if (first.x !== last.x || first.y !== last.y) {
          generatedTrack.push({ x: first.x, y: first.y });
        }
      }
      
      // Normalize the track points into the unit square.
      const normalizedTrack = generatedTrack.map(pt => ({
        x: pt.x / BBOX.xr,
        y: pt.y / BBOX.yb
      }));
      
      // Resample the normalized track into a fixed-length vector (e.g., 100 points).
      const fixedLength = 100;
      const splineVector = resamplePoints(normalizedTrack, fixedLength);
      
      // Insert the fixed-length spline vector into the genotype JSON.
      genotype.splineVector = splineVector;
      
      // Print the updated JSON to the console.
      console.log(`Updated Genotype JSON for ${inputFilePath}:\n`, JSON.stringify(genotype, null, 2));
      
      // Rewrite the file with the updated JSON.
      await fs.writeFile(inputFilePath, JSON.stringify(genotype, null, 2), 'utf8');
      console.log(`File "${inputFilePath}" updated successfully.`);
      
    } catch (error) {
      console.error(`Error processing file ${inputFilePath}:`, error);
    }
  }
}

main();
