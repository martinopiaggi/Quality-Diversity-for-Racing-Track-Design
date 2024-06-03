import { exec } from 'child_process';
import { TrackGeneratorFactory } from './src/trackGen/trackGeneratorFactory.js';
import * as utils from './src/utils/utils.js';
import * as xml from './src/utils/xmlTorcsGenerator.js';
import { promises as fs } from 'fs';
import path from 'path';
import os from 'os';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const MODE = 'voronoi'; // or convexHull
const TRACK_SIZE = 7;
const DOCKER_IMAGE_NAME = 'torcs';
const MAPELITE_PATH = './src/utils/mapelite.xml';
const MEMORY_LIMIT = '24m';
const OUTPUT_DIR = './testing/tests';

// Main function to execute the process
async function main() {
    const seed = Math.random();
    const trackGenerator = TrackGeneratorFactory.createTrackGenerator(MODE, BBOX, seed, TRACK_SIZE);
    let splineTrack = utils.splineSmoothing(trackGenerator.trackEdges);
    splineTrack = processTrackEdges(splineTrack);
    const trackXml = xml.exportTrackToXML(splineTrack, 0);

    console.log(`SEED: ${seed}`);
    console.log(`MODE: ${MODE}`);
    console.log(`trackSize: ${TRACK_SIZE}`);

    try {
        const containerId = await startDockerContainer();
        const trackgenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
        
        await runRaceSimulation(containerId, seed, TRACK_SIZE, trackgenOutput);
        await stopDockerContainer(containerId);
    } catch (err) {
        console.error(`Error: ${err.message}`);
    }
}

// Docker container management
async function startDockerContainer() {
    const containerId = await executeCommand(`docker run -d -it --memory ${MEMORY_LIMIT} ${DOCKER_IMAGE_NAME}`);
    console.log(`Docker container started with ID: ${containerId}`);
    await executeCommand(`docker exec ${containerId} mkdir -p /usr/share/games/torcs/tracks/dirt/output`);
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
        await executeCommand(`docker cp ${tmpFilePath} ${containerId}:/usr/share/games/torcs/tracks/dirt/output/${seed}.xml`);
        await executeCommand(`docker exec ${containerId} mv /usr/share/games/torcs/tracks/dirt/output/${seed}.xml /usr/share/games/torcs/tracks/dirt/output/output.xml`);
        const trackgenOutput = await executeCommand(`docker exec ${containerId} xvfb-run /usr/games/trackgen -c dirt -n output`);
        console.log(trackgenOutput);

        await fs.unlink(tmpFilePath);
        return trackgenOutput;
    } catch (err) {
        await fs.unlink(tmpFilePath);
        throw new Error(`Failed to generate and move track files: ${err.message}`);
    }
}

// Track processing
function processTrackEdges(track) {

    const segmentLength = 10;
    const minIndex = utils.findMaxCurveBeforeStraight(track, segmentLength);
    return track.slice(minIndex).concat(track.slice(0, minIndex));
}


// Simulation and output
async function runRaceSimulation(containerId, seed, trackSize, trackgenOutput) {
    try {
        await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
        await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
        console.log(`Race simulation completed inside Docker container ${containerId}`);

        const { length, deltaX, deltaY, deltaAngle, deltaAngleDegrees } = parseTrackgenOutput(trackgenOutput);

        const jsonContent = {
            MODE,
            seed,
            trackSize,
            length,
            deltaX,
            deltaY,
            deltaAngleDegrees
        };

        const jsonFileName = `${seed}.json`;
        const jsonFilePath = path.join(OUTPUT_DIR, jsonFileName);
        await fs.mkdir(OUTPUT_DIR, { recursive: true });
        await fs.writeFile(jsonFilePath, JSON.stringify(jsonContent, null, 2));
        console.log(`JSON file saved at: ${jsonFilePath}`);
    } catch (err) {
        throw new Error(`Failed to run race simulation: ${err.message}`);
    }
}

function parseTrackgenOutput(trackgenOutput) {
    const lengthMatch = trackgenOutput.match(/length\s*=\s*([\d.]+)/);
    const deltaXMatch = trackgenOutput.match(/Delta X\s*=\s*(-?[\d.]+)/);
    const deltaYMatch = trackgenOutput.match(/Delta Y\s*=\s*(-?[\d.]+)/);
    const deltaAngleMatch = trackgenOutput.match(/Delta Ang\s*=\s*(-?[\d.]+)\s*\((-?[\d.]+)\)/);

    return {
        length: lengthMatch ? parseFloat(lengthMatch[1]) : null,
        deltaX        : deltaXMatch ? parseFloat(deltaXMatch[1]) : null,
        deltaY: deltaYMatch ? parseFloat(deltaYMatch[1]) : null,
        deltaAngle: deltaAngleMatch ? parseFloat(deltaAngleMatch[1]) : null,
        deltaAngleDegrees: deltaAngleMatch ? parseFloat(deltaAngleMatch[2]) : null
    };
}

// Utility function to execute shell commands
function executeCommand(command) {
    return new Promise((resolve, reject) => {
        exec(command, (error, stdout, stderr) => {
            if (error) {
                reject(new Error(`Command failed: ${error.message}`));
                return;
            }
            if (stderr) {
                reject(new Error(`stderr: ${stderr}`));
                return;
            }
            resolve(stdout.trim());
        });
    });
}

// Execute the main function
main().catch(err => console.error(`Unhandled error: ${err.message}`));

