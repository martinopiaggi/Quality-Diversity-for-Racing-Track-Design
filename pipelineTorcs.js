import { exec } from 'child_process';
import { VoronoiTrackGenerator } from './src/voronoiTrackGenerator.js';
import * as utils from './src/utils/utils.js';
import * as xml from './src/utils/xmlTorcsGenerator.js';
import { promises as fs } from 'fs';
import path from 'path';
import os from 'os';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 4;
const DOCKER_IMAGE_NAME = 'torcs';
const MAPELITE_PATH = './src/utils/mapelite.xml';
const MEMORY_LIMIT = '24m';
const OUTPUT_DIR = './testing/tests';

// Track generation
const seed = Math.random();
const trackGenerator = new VoronoiTrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const splineTrack = utils.splineSmoothing(trackEdges);

// Initial XML parsing
let trackXml = processTrackEdges(splineTrack);

console.log(`SEED: ${seed}`);
console.log(`trackSize (# cells): ${TRACK_SIZE}`);

// Generate and move track files
try {
    // Start the Docker container
    const containerId = await startDockerContainer();

    // Process initial track
    let trackgenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
    let { deltaX, deltaY } = parseTrackgenOutput(trackgenOutput);

    // Modify the track by adding an artificial last point
    if((Math.abs(deltaX) > 1)&&(Math.abs(deltaY) > 1)){
        let modifiedTrackXml = await addArtificialLastPoint(splineTrack, deltaX, deltaY, seed);
        // Process the modified track
        trackgenOutput = await generateAndMoveTrackFiles(containerId, modifiedTrackXml, seed);
    }
    
    // Run the race simulation with the final track
    await runRaceSimulation(containerId, seed, TRACK_SIZE, trackgenOutput);

    // Clean up the Docker container
    await stopDockerContainer(containerId);

} catch (err) {
    console.error(`Error: ${err.message}`);
}

function processTrackEdges(track) {
    const segmentLength = 10;
    let minIndex = utils.findMaxCurveBeforeStraight(track, segmentLength);
    track = track.slice(minIndex).concat(track.slice(0, minIndex));
    return xml.exportTrackToXML(track, 0); // Return the XML string
}

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
    // Create a temporary file on the host machine with the seed name
    const tmpDir = os.tmpdir();
    const tmpFilePath = path.join(tmpDir, `${seed}.xml`);
    await fs.writeFile(tmpFilePath, trackXml);

    try {
        // Copy the temporary file into the Docker container with the seed name
        await executeCommand(`docker cp ${tmpFilePath} ${containerId}:/usr/share/games/torcs/tracks/dirt/output/${seed}.xml`);
        
        // Rename the file inside the Docker container to output.xml
        await executeCommand(`docker exec ${containerId} mv /usr/share/games/torcs/tracks/dirt/output/${seed}.xml /usr/share/games/torcs/tracks/dirt/output/output.xml`);
        
        const trackgenOutput = await executeCommand(`docker exec ${containerId} xvfb-run /usr/games/trackgen -c dirt -n output`);
        console.log(trackgenOutput);

        // Clean up the temporary file
        await fs.unlink(tmpFilePath);

        return trackgenOutput;
    } catch (err) {
        // Clean up the temporary file in case of error
        await fs.unlink(tmpFilePath);
        throw new Error(`Failed to generate and move track files: ${err.message}`);
    }
}

async function addArtificialLastPoint(track, deltaX, deltaY) {
    // Step 1: Calculate the artificial last point
    const lastPoint = track[track.length - 1];
    const artificialLastPoint = {
        x: lastPoint.x - deltaX,
        y: lastPoint.y - deltaY
    };

    // Step 2: Remove the first 1 point and the last 5 points
    track.splice(track.length - 1, 1); //remove current last point
    while((track.length+1)%3!=0){
        track.pop(track.length - 1);
    }

    // Step 3: Add the artificial last point as the new last point
    track.push(artificialLastPoint);

    // Export the modified track to XML
    const modifiedTrackXml = xml.exportTrackToXML(track, 0);
    console.log('Artificial last point added and new track XML generated.');
    return modifiedTrackXml;
}

async function runRaceSimulation(containerId, seed, trackSize, trackgenOutput) {
    try {
        await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
        await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
        console.log(`Race simulation completed inside Docker container ${containerId}`);

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
