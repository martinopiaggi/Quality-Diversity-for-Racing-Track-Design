import { exec } from 'child_process';
import { generateTrack } from '../trackGen/trackGenerator.js';
import * as xml from '../utils/xmlTorcsGenerator.js';
import {saveFitnessToJson} from '../utils/jsonUtils.js';
import { promises as fs } from 'fs';
import path from 'path';
import os from 'os';

import { BBOX, MODE, DOCKER_IMAGE_NAME, MAPELITE_PATH, MEMORY_LIMIT } from '../utils/constants.js';

// Main function to simulate a track using Docker torcs
async function simulate(mode = MODE, trackSize = 0, dataSet = [], voronoiCells = []) {
    if(trackSize == 0) trackSize = Math.floor(Math.random() * (5 - 2 + 1)) + 2;
    
    const seed = Math.random();
    const splineTrack = await generateTrack(mode, BBOX, seed, trackSize, true, dataSet, voronoiCells);

    //uncomment this if you want to save the "output.xml" inside current folder for local Torcs test
    //const trackXml = xml.exportTrackToXML(splineTrack,0, true);

    const trackXml = xml.exportTrackToXML(splineTrack);

    console.log(`SEED: ${seed}`);
    console.log(`MODE: ${mode}`);
    console.log(`trackSize: ${trackSize}`);

    try {
        const containerId = await startDockerContainer();
        const trackgenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
        
        await runRaceSimulation(containerId, seed, trackSize, trackgenOutput);
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


// Simulation and output
async function runRaceSimulation(containerId, seed, trackSize, trackgenOutput) {
    try {
        await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
        await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
        console.log(`Race simulation completed inside Docker container ${containerId}`);

        const { length, deltaX, deltaY, deltaAngle, deltaAngleDegrees } = parseTrackgenOutput(trackgenOutput);

        await saveFitnessToJson(seed, MODE, trackSize, length, deltaX, deltaY, deltaAngleDegrees);
    
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
simulate().catch(err => console.error(`Unhandled error: ${err.message}`));

