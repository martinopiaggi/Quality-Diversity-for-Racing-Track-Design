// convertGenotype.js

import fs from 'fs/promises';
import path from 'path';
import { resamplePoints } from '../src/utils/utils.js'
import { generateTrack } from '../src/trackGen/trackGenerator.js'; // Adjust the path as needed
import { BBOX } from "../src/utils/constants.js";
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
      const splineVector = resamplePoints(trackResult.track);
      
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
