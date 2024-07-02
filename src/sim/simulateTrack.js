import { exec } from 'child_process';
import { generateTrack } from '../trackGen/trackGenerator.js';
import * as xml from '../utils/xmlTorcsGenerator.js';
import { saveFitnessToJson } from '../utils/jsonUtils.js';
import { promises as fs } from 'fs';
import path from 'path';
import os from 'os';

import { BBOX, MODE, DOCKER_IMAGE_NAME, MAPELITE_PATH, MEMORY_LIMIT } from '../utils/constants.js';

const SIMULATION_TIMEOUT = 30000; // 30 seconds timeout

export async function simulate(mode = MODE, trackSize = 0, 
    dataSet = [], selected = [], seed = null, saveJson = true) {
    
    if(trackSize == 0){
        if(mode=='voronoi'){
            trackSize = Math.ceil(Math.random() * 4 ) + 1; 
        } else {
            trackSize = 50;
        }
    }

    if(seed === null) seed = Math.random();
    
    const trackResults = await generateTrack(mode, BBOX, seed, trackSize, saveJson, dataSet, selected);
    const trackXml = xml.exportTrackToXML(trackResults.track);

    console.log(`SEED: ${seed}`);
    console.log(`MODE: ${mode}`);
    console.log(`trackSize: ${trackSize}`);

    let containerId;
    try {
        containerId = await startDockerContainer();
        const trackGenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
        
        const fitness = await Promise.race([
            runRaceSimulation(containerId, trackGenOutput),
            new Promise((_, reject) => 
                setTimeout(() => reject(new Error('Simulation timeout')), SIMULATION_TIMEOUT)
            )
        ]);

        if (saveJson) {
            await saveFitnessToJson(seed, mode, trackResults.generator.trackSize, fitness.length, fitness.deltaX, fitness.deltaY, fitness.deltaAngleDegrees);
        }

        return {fitness: fitness        };

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

async function runRaceSimulation(containerId,trackGenOutput) {
    await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
    await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
    console.log(`Race simulation completed inside Docker container ${containerId}`);

    const { length, deltaX, deltaY, deltaAngle, deltaAngleDegrees } = parseTrackgenOutput(trackGenOutput);

    return { length, deltaX, deltaY, deltaAngleDegrees };
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

if (process.argv[1].indexOf('simulateTrack.js') !== -1) {
    //it means it is run directly as a script and not by another module
    simulate().catch(err => console.error(`Unhandled error: ${err.message}`));
}