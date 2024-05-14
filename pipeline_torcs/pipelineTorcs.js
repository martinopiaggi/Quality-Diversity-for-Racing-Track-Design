const fs = require('fs');
const { exec } = require('child_process');
const TrackGenerator = require('./voronoiTrackGenerator');
const utils = require('./utils/utils');
const xml = require('./xmlTorcsGenerator');

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 5;

// Track generation
const seed = Math.random();
const trackGenerator = new TrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const splineTrack = utils.splineSmoothing(trackEdges);

// XML processing for Torcs 
processTrackEdges(splineTrack);

console.log("SEED: " + seed);
console.log("trackSize (# cells): " + TRACK_SIZE);

// Execute CLI commands
generateAndMoveTrackFiles();

function processTrackEdges(track) {
    let minIndex = utils.findMinCurvatureSegment(track, 20);
    track = track.slice(minIndex).concat(track.slice(0, minIndex));
    track.splice(0, 10);
    track.splice(track.length - 10, 10);
    xml.exportTrackToXML(track, 0);
}

function generateAndMoveTrackFiles() {
    exec(`"C:\\Program Files (x86)\\torcs\\trackgen.exe" -c dirt -n output`, (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
        if (stderr) {
            console.error(`stderr: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);

        exec(`powershell -ExecutionPolicy Bypass -File ./copyFilesToTorcs.ps1`, (copyError, copyStdout, copyStderr) => {
            if (copyError) {
                console.error(`exec error: ${copyError}`);
                return;
            }
            if (copyStderr) {
                console.error(`stderr: ${copyStderr}`);
                return;
            }
            console.log(copyStdout);
            console.log(`Track copied to Torcs folder`);

            exec(`wtorcs.exe -nofuel -nodamage -r "./config/raceman/mapelite.xml"`, {
                cwd: "C:\\Program Files (x86)\\torcs"
            }, (torcsError, torcsStdout, torcsStderr) => {
                if (torcsError) {
                    console.error(`exec error: ${torcsError}`);
                    return;
                }
                if (torcsStderr) {
                    console.error(`stderr: ${torcsStderr}`);
                    return;
                }
                console.log(`TORCS Simulation stdout: ${torcsStdout}`);
                console.log(`TORCS simulation completed successfully.`);
            });
        });
    });
}
