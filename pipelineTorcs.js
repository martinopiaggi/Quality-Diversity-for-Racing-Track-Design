import { exec } from 'child_process';
import { TrackGeneratorFactory } from './src/trackGen/trackGeneratorFactory.js';
import * as utils from './src/utils/utils.js';
import * as xml from './src/utils/xmlTorcsGenerator.js';
import { promises as fs } from 'fs';
import path from 'path';
import os from 'os';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const MODE = 'voronoi'; // or 'voronoi'
const TRACK_SIZE = 2;
const DOCKER_IMAGE_NAME = 'torcs';
const MAPELITE_PATH = './src/utils/mapelite.xml';
const MEMORY_LIMIT = '24m';
const OUTPUT_DIR = './testing/tests';

// Track generation
const seed = Math.random();

const trackGenerator = TrackGeneratorFactory.createTrackGenerator(MODE, BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
let splineTrack = utils.splineSmoothing(trackEdges);

splineTrack= processTrackEdges(splineTrack);

let trackXml = xml.exportTrackToXML(splineTrack, 0);


console.log(`SEED: ${seed}`);
console.log(`MODE: ${MODE}`);
console.log(`trackSize: ${TRACK_SIZE}`);

// Generate and move track files
try {
    // Start the Docker container       
    const containerId = await startDockerContainer();

    // Process initial track
    let trackgenOutput = await generateAndMoveTrackFiles(containerId, trackXml, seed);
    let { deltaX, deltaY } = parseTrackgenOutput(trackgenOutput);

    // Modify the track by adding an artificial last point
    if(false){
        let modifiedTrackXml = await addArtificialLastPoints(splineTrack, deltaX, deltaY, seed);
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
    return track;
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

async function addArtificialLastPoints(track, deltaX, deltaY) {
    // Step 1: Determine the number of points to adjust (e.g., last 3 points)
    const pointsToAdjust = 3;
    const trackLength = track.length;
    const adjustedTrack = track.slice(0, trackLength - pointsToAdjust);

    // Step 2: Calculate control points for Bezier curve fitting
    const controlPoints = calculateControlPoints(track.slice(trackLength - pointsToAdjust), deltaX, deltaY);

    // Step 3: Generate the adjusted points using Bezier curves
    const adjustedPoints = generateBezierPoints(controlPoints);

    // Step 4: Add the adjusted points back to the track
    adjustedTrack.push(...adjustedPoints);

    // Export the modified track to XML
    const modifiedTrackXml = xml.exportTrackToXML(adjustedTrack, 0);
    console.log('Artificial last points added and new track XML generated.');
    return modifiedTrackXml;
}

function calculateControlPoints(points, deltaX, deltaY) {
    const [thirdLast, secondLast, last] = points;

    // Calculate the first control point based on the curve before the straight line
    const controlPoint1 = {
        x: secondLast.x + (thirdLast.x - secondLast.x) * 0.5,
        y: secondLast.y + (thirdLast.y - secondLast.y) * 0.5
    };

    // Calculate the second control point influenced by delta values
    const controlPoint2 = {
        x: last.x - deltaX * 0.5,
        y: last.y - deltaY * 0.5
    };

    // Return control points along with the end point adjusted by delta
    return [thirdLast, controlPoint1, controlPoint2, { x: last.x - deltaX, y: last.y - deltaY }];
}

function generateBezierPoints(controlPoints) {
    const [p0, p1, p2, p3] = controlPoints;
    const bezierPoints = [];
    const numPoints = 10; // Number of points to generate along the curve

    for (let i = 0; i <= numPoints; i++) {
        const t = i / numPoints;
        const x = (1 - t) ** 3 * p0.x +
                  3 * (1 - t) ** 2 * t * p1.x +
                  3 * (1 - t) * t ** 2 * p2.x +
                  t ** 3 * p3.x;

        const y = (1 - t) ** 3 * p0.y +
                  3 * (1 - t) ** 2 * t * p1.y +
                  3 * (1 - t) * t ** 2 * p2.y +
                  t ** 3 * p3.y;

        bezierPoints.push({ x, y });
    }

    return bezierPoints;
}




async function runRaceSimulation(containerId, seed, trackSize, trackgenOutput) {
    try {
        await executeCommand(`docker cp ${MAPELITE_PATH} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`);
        await executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`);
        console.log(`Race simulation completed inside Docker container ${containerId}`);

        // Parse the trackgenOutput
        const { length, deltaX, deltaY, deltaAngle, deltaAngleDegrees } = parseTrackgenOutput(trackgenOutput);

        // Create JSON structure
        const jsonContent = {
            seed,
            trackSize,
            length,
            deltaX,
            deltaY,
            deltaAngleDegrees
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
    const deltaAngleMatch = trackgenOutput.match(/Delta Ang\s*=\s*(-?[\d.]+)\s*\((-?[\d.]+)\)/);

    return {
        length: lengthMatch ? parseFloat(lengthMatch[1]) : null,
        deltaX: deltaXMatch ? parseFloat(deltaXMatch[1]) : null,
        deltaY: deltaYMatch ? parseFloat(deltaYMatch[1]) : null,
        deltaAngle: deltaAngleMatch ? parseFloat(deltaAngleMatch[1]) : null,
        deltaAngleDegrees: deltaAngleMatch ? parseFloat(deltaAngleMatch[2]) : null
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
