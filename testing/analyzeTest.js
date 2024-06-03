import fs from 'fs/promises';
import path from 'path';

const JSON_DIR = './tests'; // Directory containing JSON files

async function collectData() {
    try {
        const files = await fs.readdir(JSON_DIR);
        const jsonFiles = files.filter(file => file.endsWith('.json'));

        let totalDeltaX = 0;
        let totalDeltaY = 0;
        let totalLength = 0;
        let totalDeltaAngleDegrees = 0;
        let count = 0;
        let sumOfSquaresDeltaX = 0;
        let sumOfSquaresDeltaY = 0;
        let sumOfSquaresDeltaAngleDegrees = 0;
        let perfectTrackCount = 0;
        let veryBadTrackCount = 0;
        let perfectTracks = [];
        let veryBadTracks = [];

        for (const file of jsonFiles) {
            const filePath = path.join(JSON_DIR, file);
            const data = JSON.parse(await fs.readFile(filePath, 'utf-8'));

            totalDeltaX += data.deltaX;
            totalDeltaY += data.deltaY;
            totalLength += data.length;
            totalDeltaAngleDegrees += data.deltaAngleDegrees;

            if (Math.abs(data.deltaX) < 1 && Math.abs(data.deltaY) < 1 && Math.abs(data.deltaAngleDegrees) < 10) {
                perfectTrackCount++;
                perfectTracks.push({
                    seed: data.seed,
                    trackSize: data.trackSize,
                    mode: data.MODE,
                });
            }

            if ((Math.abs(data.deltaX) + Math.abs(data.deltaY) > 6)) {
                veryBadTrackCount++;
                veryBadTracks.push({
                    seed: data.seed,
                    trackSize: data.trackSize,
                    mode: data.MODE,
                    deltaX: data.deltaX,
                    deltaY: data.deltaY,
                    deltaAngleDegrees: data.deltaAngleDegrees
                });
            }

            count++;
        }

        const averageDeltaX = totalDeltaX / count;
        const averageDeltaY = totalDeltaY / count;
        const averageLength = totalLength / count;
        const averageDeltaAngleDegrees = totalDeltaAngleDegrees / count;

        for (const file of jsonFiles) {
            const filePath = path.join(JSON_DIR, file);
            const data = JSON.parse(await fs.readFile(filePath, 'utf-8'));

            sumOfSquaresDeltaX += Math.pow(data.deltaX - averageDeltaX, 2);
            sumOfSquaresDeltaY += Math.pow(data.deltaY - averageDeltaY, 2);
            sumOfSquaresDeltaAngleDegrees += Math.pow(data.deltaAngleDegrees - averageDeltaAngleDegrees, 2);
        }

        const varianceDeltaX = Math.sqrt(sumOfSquaresDeltaX / (count - 1));
        const varianceDeltaY = Math.sqrt(sumOfSquaresDeltaY / (count - 1));
        const varianceDeltaAngleDegrees = Math.sqrt(sumOfSquaresDeltaAngleDegrees / (count - 1));

        console.log(`Number of JSON files: ${count}`);
        console.log(`Average length: ${averageLength}`);
        console.log(`Average deltaX: ${averageDeltaX}`);
        console.log(`Average deltaY: ${averageDeltaY}`);
        console.log(`Average deltaAngleDegrees: ${averageDeltaAngleDegrees}`);
        console.log(`Variance deltaX: ${varianceDeltaX}`);
        console.log(`Variance deltaY: ${varianceDeltaY}`);
        console.log(`Variance deltaAngleDegrees: ${varianceDeltaAngleDegrees}`);
        console.log(`Number of perfect tracks: ${perfectTrackCount}`);

        if (perfectTrackCount > 0) {
            console.log('Perfect tracks details:');
            perfectTracks.forEach(track => {
                console.log(`Seed: ${track.seed}, Track Size: ${track.trackSize}, Mode: ${track.mode}`);
            });
        }

        console.log(`Number of very bad tracks: ${veryBadTrackCount}`);

        if (veryBadTrackCount > 0) {
            console.log('Very bad tracks details:');
            veryBadTracks.forEach(track => {
                console.log(`Seed: ${track.seed}, Track Size: ${track.trackSize}, Mode: ${track.mode}, deltaX: ${track.deltaX}, deltaY: ${track.deltaY}, deltaAngleDegrees: ${track.deltaAngleDegrees}`);
            });
        }

    } catch (err) {
        console.error(`Error: ${err.message}`);
    }
}

collectData();
