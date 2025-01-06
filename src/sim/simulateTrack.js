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
  MEMORY_LIMIT,
  SIMULATION_TIMEOUT,
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

export async function simulate(
  mode = MODE,
  trackSize = 0,
  dataSet = [],
  selected = [],
  seed = null,
  saveJson = false,
  plot = false
) {
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
    containerId = await startDockerContainer();
    const trackGenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
    console.log(trackGenOutput);

    const simCommand = `docker exec ${containerId} python3 /usr/local/lib/sirianni_tools/run-simulations.py --track-export -r 5 --json ${plot ? '' : '--plots'}`;
    const simulationOutput = await Promise.race([
      executeCommand(simCommand),
      new Promise((_, reject) =>
        setTimeout(() => reject(new Error('Simulation timeout')), SIMULATION_TIMEOUT)
      )
    ]);
    let rawMetrics = {};
    const jsonStart = simulationOutput.indexOf('===FINAL_JSON_START===');
    const jsonEnd = simulationOutput.indexOf('===FINAL_JSON_END===', jsonStart);
    if (jsonStart !== -1 && jsonEnd !== -1) {
      const jsonString = simulationOutput
        .substring(jsonStart + '===FINAL_JSON_START==='.length, jsonEnd)
        .trim();
      rawMetrics = JSON.parse(jsonString);
    } else {
      throw new Error('JSON markers not found in run-simulations.py output.');
    }
    const { length, deltaX, deltaY, deltaAngleDegrees } = parseTrackgenOutput(trackGenOutput);
    const {
      speed_entropy = 0,
      acceleration_entropy = 0,
      braking_entropy = 0,
      positions_mean = 0,
      avg_radius_mean = 0,
      gaps_mean = 0,
      right_bends = 0,
      avg_radius_var = 0,
      total_overtakes = 0,
      straight_sections = 0,
      gaps_var = 0,
      left_bends = 0,
      positions_var = 0,
      curvature_entropy = 0,
      ...rest
    } = rawMetrics;
    const fitness = {
      length,
      deltaX,
      deltaY,
      deltaAngleDegrees,
      speed_entropy,
      acceleration_entropy,
      braking_entropy,
      positions_mean,
      avg_radius_mean,
      gaps_mean,
      right_bends,
      avg_radius_var,
      total_overtakes,
      straight_sections,
      gaps_var,
      left_bends,
      positions_var,
      curvature_entropy,
      ...rest
    };
    if (saveJson) {
      await saveFitnessToJson(
        seed,
        mode,
        trackResults.generator.trackSize,
        {
          length: fitness.track_length || fitness.length,
          deltaX: fitness.deltaX,
          deltaY: fitness.deltaY,
          deltaAngleDegrees: fitness.deltaAngleDegrees,
          speed_entropy: fitness.speed_entropy,
          acceleration_entropy: fitness.acceleration_entropy,
          braking_entropy: fitness.braking_entropy,
          positions_mean: fitness.positions_mean,
          avg_radius_mean: fitness.avg_radius_mean,
          gaps_mean: fitness.gaps_mean,
          right_bends: fitness.right_bends,
          avg_radius_var: fitness.avg_radius_var,
          total_overtakes: fitness.total_overtakes,
          straight_sections: fitness.straight_sections,
          gaps_var: fitness.gaps_var,
          left_bends: fitness.left_bends,
          positions_var: fitness.positions_var,
          curvature_entropy: fitness.curvature_entropy
        }
      );
    }
    return { fitness };
  } catch (err) {
    console.error(`Error: ${err.message}`);
    throw err;
  } finally {
    if (containerId) {
      await stopDockerContainer(containerId);
    }
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
  simulate().catch(err => console.error(`Unhandled error: ${err.message}`));
}
