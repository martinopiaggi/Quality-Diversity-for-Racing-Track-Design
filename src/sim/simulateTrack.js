import { exec } from 'child_process';
import { promises as fs } from 'fs';
import { generateTrack } from '../trackGen/trackGenerator.js';
import * as xml from '../utils/xmlTorcsGenerator.js';
import { saveFitnessToJson } from '../utils/jsonUtils.js';
import path from 'path';
import os from 'os';
import { BBOX, MODE, DOCKER_IMAGE_NAME, MAPELITE_PATH, MEMORY_LIMIT, JSON_DEBUG } from '../utils/constants.js';

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

    const trackResults = await generateTrack(mode, BBOX, seed, trackSize, saveJson, dataSet, selected);
    const trackXml = xml.exportTrackToXML(trackResults.track);

    console.log(`SEED: ${seed}`);
    console.log(`MODE: ${mode}`);
    console.log(`trackSize: ${trackSize}`);

    let containerId;
    try {
        containerId = await startDockerContainer();
        const trackGenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);

        await executeCommand(`docker exec ${containerId} python3 /usr/local/lib/sirianni_tools/run-simulations.py --track-export`);
        console.log("Track export completed.");

        const raceFitness = await Promise.race([
            runRaceSimulation(containerId, trackGenOutput),
            new Promise((_, reject) => setTimeout(() => reject(new Error('Simulation timeout')), SIMULATION_TIMEOUT))
        ]);

        console.log(`Race simulation completed inside Docker container ${containerId}`);

        // Now run the analysis with fallback track-length and json output
        const analyzeCmd = `docker exec ${containerId} python3 /usr/local/lib/sirianni_tools/analyze-simulations.py -B 200 --json-output -c /root/.torcs/logs`;
        const analyzeOutput = await executeCommand(analyzeCmd);

        let rawMetrics = {};
        const jsonStart = analyzeOutput.indexOf('===JSON_START===');
        const jsonEnd = analyzeOutput.indexOf('===JSON_END===', jsonStart);
        
        if (jsonStart !== -1 && jsonEnd !== -1) {
        const jsonString = analyzeOutput.substring(jsonStart + '===JSON_START==='.length, jsonEnd).trim();
        rawMetrics = JSON.parse(jsonString);
        } else {
        throw new Error('JSON markers not found in analysis output.');
        }


        // Combine the metrics:
        const fitness = {
            length: raceFitness.length,
            deltaX: raceFitness.deltaX,
            deltaY: raceFitness.deltaY,
            deltaAngleDegrees: raceFitness.deltaAngleDegrees,
            ...rawMetrics
        };

        if (saveJson) {
            await saveFitnessToJson(seed, mode, trackResults.generator.trackSize, fitness.track_length || fitness.length, fitness.deltaX, fitness.deltaY, fitness.deltaAngleDegrees);
        }

        return { fitness };

    } catch (err) {
        console.error(`Error: ${err.message}`);
        throw err;
    } finally {
        if (containerId) {
            // Optionally stop/remove container if needed
            // await stopDockerContainer(containerId);
        }
    }
}

async function startDockerContainer() {
    const containerId = await executeCommand(`docker run -d -it --memory ${MEMORY_LIMIT} ${DOCKER_IMAGE_NAME}`);
    console.log(`Docker container started with ID: ${containerId}`);
    await executeCommand(`docker exec ${containerId} mkdir -p /usr/local/share/games/torcs/tracks/road/output`);
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
        await executeCommand(`docker cp ${tmpFilePath} ${containerId}:/usr/local/share/games/torcs/tracks/road/output/${seed}.xml`);
        await executeCommand(`docker exec ${containerId} mv /usr/local/share/games/torcs/tracks/road/output/${seed}.xml /usr/local/share/games/torcs/tracks/road/output/output.xml`);
        const trackgenOutput = await executeCommand(`docker exec ${containerId} xvfb-run trackgen -c road -n output`);
        console.log(trackgenOutput);

        await fs.unlink(tmpFilePath);
        return trackgenOutput;
    } catch (err) {
        await fs.unlink(tmpFilePath);
        throw new Error(`Failed to generate and move track files: ${err.message}`);
    }
}

async function runRaceSimulation(containerId, trackGenOutput) {
    await executeCommand(`docker exec ${containerId} python3 /usr/local/lib/sirianni_tools/run-simulations.py -r 10`);

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

if (process.argv[1].includes('simulateTrack.js')) {
    // If needed to run directly
    simulate().catch(err => console.error(`Unhandled error: ${err.message}`));
}
