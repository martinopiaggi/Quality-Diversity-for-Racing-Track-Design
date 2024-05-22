import { exec } from 'child_process';
import { VoronoiTrackGenerator } from './voronoiTrackGenerator.js';
import * as utils from './utils/utils.js';
import * as xml from './utils/xmlTorcsGenerator.js';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 2;

//paths using docker image
const dockerImageName = 'torcs';
const dockerContainerName = 'torcs-container';
const tracksPath = './tracks/dirt/output';
const mapelitePath = 'mapelite.xml'; 


// Track generation
const seed = 0.023881638499472446 //Math.random();
const trackGenerator = new VoronoiTrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const splineTrack = utils.splineSmoothing(trackEdges);

// XML processing for Torcs 
processTrackEdges(splineTrack);

console.log("SEED: " + seed);
console.log("trackSize (# cells): " + TRACK_SIZE);

// Execute CLI commands
generateAndMoveTrackFiles();

function processTrackEdges(track) {
    const segmentLength = 20;
    let minIndex = utils.findMinCurvatureSegment(track, segmentLength);
    //let minIndex = utils.findMaxCurveBeforeStraight(track,40)
    //make the track start from minIndex
    track = track.slice(minIndex).concat(track.slice(0, minIndex));
    track.splice(0, segmentLength/4); 
    //track.splice(track.length - 5, 5);
    xml.exportTrackToXML(track, 0);
}

function generateAndMoveTrackFiles() {
        // Step 1: Run the Docker container interactively with a random name
        executeCommand(`docker run -d -it ${dockerImageName}`, (containerId) => {
            console.log(`Docker container started with ID: ${containerId}`);

            // Step 2: Copy track files to Docker container
            executeCommand(`docker cp ${tracksPath} ${containerId}:/usr/share/games/torcs/tracks/dirt/output`, () => {

                // Step 3: Generate track files inside the Docker container 
                // xvfb-run is used for virtual display (otherwise trackgen quit)
                executeCommand(`docker exec ${containerId} xvfb-run /usr/games/trackgen -c dirt -n output`, () => {

                    // Step 4: Copy mapelite.xml to Docker container
                    executeCommand(`docker cp ${mapelitePath} ${containerId}:/usr/share/games/torcs/config/raceman/mapelite.xml`, () => {

                        // Step 5: Run TORCS simulation inside Docker container
                        executeCommand(`docker exec ${containerId} /usr/games/torcs -r /usr/share/games/torcs/config/raceman/mapelite.xml`, () => {

                            // Step 6: Clean up by stopping and removing the Docker container
                            executeCommand(`docker stop ${containerId} && docker rm ${containerId}`, () => {
                                console.log(`Docker container ${containerId} stopped and removed.`);
                            });
                        });
                    });
                });
            });
        });
    }

function executeCommand(command, callback) {
    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error: ${error.message}`);
            return;
        }
        if (stderr) {
            console.error(`stderr: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);
        if (callback) callback(stdout.trim());
    });
}