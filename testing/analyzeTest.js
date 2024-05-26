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
        let count = 0;

        for (const file of jsonFiles) {
            const filePath = path.join(JSON_DIR, file);
            const data = JSON.parse(await fs.readFile(filePath, 'utf-8'));

            totalDeltaX += data.deltaX;
            totalDeltaY += data.deltaY;
            totalLength += data.length;
            count++;
        }

        const averageDeltaX = totalDeltaX / count;
        const averageDeltaY = totalDeltaY / count;
        const averageLength = totalLength / count;

        console.log(`Number of JSON files: ${count}`);
        console.log(`Average deltaX: ${averageDeltaX}`);
        console.log(`Average deltaY: ${averageDeltaY}`);
        console.log(`Average length: ${averageLength}`);
    } catch (err) {
        console.error(`Error: ${err.message}`);
    }
}

collectData();
