import { exec } from 'child_process';
import { VoronoiTrackGenerator } from './src/voronoiTrackGenerator.js';
import * as utils from './src/utils/utils.js';
import * as xml from './src/utils/xmlTorcsGenerator.js';
import { promises as fs } from 'fs';
import path from 'path';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 5;
const DOCKER_IMAGE_NAME = 'torcs';
const MAPELITE_PATH = './src/utils/mapelite.xml';
const MEMORY_LIMIT = '24m';
const TRACKS_INFO_DIR = './tests';

// Track generation
const seed = 0.27443702376629964 //Math.random();
const trackGenerator = new VoronoiTrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const splineTrack = utils.splineSmoothing(trackEdges);

// Process track edges for Torcs
processTrackEdges(splineTrack);

console.log(`SEED: ${seed}`);
console.log(`trackSize (# cells): ${TRACK_SIZE}`);

// Generate and move track files
try {
    const { containerId, trackgenOutput } = await generateAndMoveTrackFiles();
    await runRaceSimulation(containerId, seed, TRACK_SIZE, trackgenOutput);
} catch (err) {
    console.error(`Error: ${err.message}`);
}

function processTrackEdges(track) {
    const segmentLength = 20;
    let minIndex = utils.findMinCurvatureSegment(track, segmentLength);
    track = track.slice(minIndex).concat(track.slice(0, minIndex));
    track.splice(0, segmentLength / 4);
    xml.exportTrackToXML(track, 0);
}

async function generateAndMoveTrackFiles() {
    try {
        const containerId = await executeCommand(`docker run -d -it --memory ${MEMORY_LIMIT} ${DOCKER_IMAGE_NAME}`);
        console.log(`Docker container started with ID: ${containerId}`);
        await executeCommand(`docker exec ${containerId} mkdir -p /usr/share/games/torcs/tracks/dirt/output`);
        await executeCommand(`docker cp "output.xml" ${containerId}:/usr/share/games/torcs/tracks/dirt/output`);
        const trackgenOutput = await executeCommand(`docker exec ${containerId} xvfb-run /usr/games/trackgen -c dirt -n output`);
        console.log(trackgenOutput);

        return { containerId, trackgenOutput };
    } catch (err) {
        throw new Error(`Failed to generate and move track files: ${err.message}`);
    }
}

async function runRaceSimulation(containerId, seed, trackSize, trackgenOutput) {
    try {
        await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
        await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
        console.log(`Race simulation completed inside Docker container ${containerId}`);

        await executeCommand(`docker rm --force ${containerId}`);
        console.log(`Docker container ${containerId} stopped and removed.`);

        // Parse the trackgenOutput
        const { length, deltaX, deltaY } = parseTrackgenOutput(trackgenOutput);

        // Create JSON structure
        const jsonContent = {
            seed,
            trackSize,
            length,
            deltaX,
            deltaY
        };

        // Write JSON to file
        const jsonFileName = `${seed}.json`;
        const jsonFilePath = path.join(TRACKS_INFO_DIR, jsonFileName);
        await fs.mkdir(TRACKS_INFO_DIR, { recursive: true });
        await fs.writeFile(jsonFilePath, JSON.stringify(jsonContent, null, 2));
        console.log(`JSON file saved at: ${jsonFilePath}`);

    } catch (err) {
        throw new Error(`Failed to run race simulation: ${err.message}`);
    }
}

function parseTrackgenOutput(trackgenOutput) {
    const lengthMatch = trackgenOutput.match(/length\s*=\s*([\d.]+)/);
    const deltaXMatch = trackgenOutput.match(/Delta X\s*=\s*([\d.]+)/);
    const deltaYMatch = trackgenOutput.match(/Delta Y\s*=\s*([\d.]+)/);

    return {
        length: lengthMatch ? parseFloat(lengthMatch[1]) : null,
        deltaX: deltaXMatch ? parseFloat(deltaXMatch[1]) : null,
        deltaY: deltaYMatch ? parseFloat(deltaYMatch[1]) : null
    };
}

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
