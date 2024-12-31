import { exec } from 'child_process';
import { promises as fs } from 'fs';
import { generateTrack } from '../trackGen/trackGenerator.js';
import * as xml from '../utils/xmlTorcsGenerator.js';
import { saveFitnessToJson } from '../utils/jsonUtils.js';
import path from 'path';
import os from 'os';
import {
  BBOX,
  MODE,
  DOCKER_IMAGE_NAME,
  MAPELITE_PATH,
  MEMORY_LIMIT,
  JSON_DEBUG
} from '../utils/constants.js';

const executeCommand = (command) => {
  return new Promise((resolve, reject) => {
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Command failed: ${error.message}`));
        return;
      }
      if (stderr) {
        console.warn(`stderr: ${stderr}`);
      }
      resolve(stdout.trim());
    });
  });
};

const SIMULATION_TIMEOUT = 30000; // 30 seconds timeout

export async function simulate(
  mode = MODE,
  trackSize = 0,
  dataSet = [],
  selected = [],
  seed = null,
  saveJson = false,
  plot = false
) {
  // 1) Decide trackSize:
  if (isNaN(trackSize)) {
    if (mode === 'voronoi') {
      if (selected.length > 0) {
        trackSize = selected.length;
      } else {
        trackSize = Math.ceil(Math.random() * 4) + 1;
      }
    } else {
      trackSize = 50;
    }
  }
  if (seed === null) seed = Math.random();

  // 2) Generate track geometry in Node (on the host):
  const trackResults = await generateTrack(
    mode, BBOX, seed, trackSize,
    saveJson, dataSet, selected
  );
  const trackXml = xml.exportTrackToXML(trackResults.track);

  console.log(`SEED: ${seed}`);
  console.log(`MODE: ${mode}`);
  console.log(`trackSize: ${trackSize}`);

  let containerId;
  try {
    // 3) Start Docker container
    containerId = await startDockerContainer();

    // 4) Copy the track XML into the container, run trackgen inside
    const trackGenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
    console.log(trackGenOutput);

    // 5) Now run the entire simulation pipeline in one command:
    //    - track-export
    //    - full race (10 laps)
    //    - JSON analysis
    const simCommand = `docker exec ${containerId} python3 /usr/local/lib/sirianni_tools/run-simulations.py --track-export -r 10 --json ${plot ? '' : '--plots'}`;
    const simulationOutput = await Promise.race([
      executeCommand(simCommand),
      new Promise((_, reject) =>
        setTimeout(() => reject(new Error('Simulation timeout')), SIMULATION_TIMEOUT)
      )
    ]);

    // 6) The simulationOutput should contain lines from the script + JSON. So parse them:
    let rawMetrics = {};
    const jsonStart = simulationOutput.indexOf('===JSON_START===');
    const jsonEnd = simulationOutput.indexOf('===JSON_END===', jsonStart);
    if (jsonStart !== -1 && jsonEnd !== -1) {
      const jsonString = simulationOutput
        .substring(jsonStart + '===JSON_START==='.length, jsonEnd)
        .trim();
      rawMetrics = JSON.parse(jsonString);
    } else {
      throw new Error('JSON markers not found in run-simulations.py output.');
    }


    //    parseTrackgenOutput is optional if you want track length from trackGenOutput
    const { length, deltaX, deltaY, deltaAngleDegrees } = parseTrackgenOutput(trackGenOutput);
    const fitness = {
      length,
      deltaX,
      deltaY,
      deltaAngleDegrees,
      ...rawMetrics
    };

    // 8) Optionally save the final JSON
    if (saveJson) {
      await saveFitnessToJson(
        seed,
        mode,
        trackResults.generator.trackSize,
        fitness.track_length || fitness.length,
        fitness.deltaX,
        fitness.deltaY,
        fitness.deltaAngleDegrees
      );
    }

    // 9) Return your final results
    return { fitness };

  } catch (err) {
    console.error(`Error: ${err.message}`);
    throw err;
  } finally {
    // Optionally comment this to not stop the container for debugging
    //if (containerId) { await stopDockerContainer(containerId); }
  }
}

async function startDockerContainer() {
  const containerId = await executeCommand(
    `docker run -d -it --memory ${MEMORY_LIMIT} ${DOCKER_IMAGE_NAME}`
  );
  console.log(`Docker container started with ID: ${containerId}`);
  await executeCommand(
    `docker exec ${containerId} mkdir -p /usr/local/share/games/torcs/tracks/road/output`
  );
  return containerId;
}

async function stopDockerContainer(containerId) {
  try {
    await executeCommand(`docker rm --force ${containerId}`);
    console.log(`Docker container ${containerId} stopped and removed.`);
  } catch (err) {
    console.error(`Failed to stop Docker container ${containerId}: ${err.message}`);
  }
}

async function generateAndMoveTrackFiles(containerId, trackXml, seed) {
  const tmpDir = os.tmpdir();
  const tmpFilePath = path.join(tmpDir, `${seed}.xml`);
  await fs.writeFile(tmpFilePath, trackXml);

  try {
    await executeCommand(
      `docker cp ${tmpFilePath} ` +
      `${containerId}:/usr/local/share/games/torcs/tracks/road/output/${seed}.xml`
    );
    await executeCommand(
      `docker exec ${containerId} ` +
      `mv /usr/local/share/games/torcs/tracks/road/output/${seed}.xml ` +
      `/usr/local/share/games/torcs/tracks/road/output/output.xml`
    );
    const trackgenOutput = await executeCommand(
      `docker exec ${containerId} xvfb-run trackgen -c road -n output`
    );
    await fs.unlink(tmpFilePath);
    return trackgenOutput;
  } catch (err) {
    await fs.unlink(tmpFilePath);
    throw new Error(`Failed to generate and move track files: ${err.message}`);
  }
}

function parseTrackgenOutput(trackgenOutput) {
  // Optional: parse length, deltaX, deltaY from trackgenOutput
  const lengthMatch = trackgenOutput.match(/length\s*=\s*([\d.]+)/);
  const deltaXMatch = trackgenOutput.match(/Delta X\s*=\s*(-?[\d.]+)/);
  const deltaYMatch = trackgenOutput.match(/Delta Y\s*=\s*(-?[\d.]+)/);
  const deltaAngleMatch = trackgenOutput.match(/Delta Ang\s*=\s*(-?[\d.]+)\s*\((-?[\d.]+)\)/);
  return {
    length: lengthMatch ? parseFloat(lengthMatch[1]) : null,
    deltaX: deltaXMatch ? parseFloat(deltaXMatch[1]) : null,
    deltaY: deltaYMatch ? parseFloat(deltaYMatch[1]) : null,
    deltaAngleDegrees: deltaAngleMatch ? parseFloat(deltaAngleMatch[2]) : null
  };
}

if (process.argv[1].includes('simulateTrack.js')) {
  // If needed to run directly from CLI
  simulate().catch(err => console.error(`Unhandled error: ${err.message}`));
}
