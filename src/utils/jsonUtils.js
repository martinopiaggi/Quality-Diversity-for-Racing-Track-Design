// jsonUtils.js
import { promises as fs } from 'fs';
import path from 'path';

const OUTPUT_DIR = "../../data/tests"

async function readJsonFile(jsonFilePath) {
    try {
        const jsonData = await fs.readFile(jsonFilePath, 'utf8');
        return JSON.parse(jsonData);
    } catch (err) {
        return null;
    }
}

async function writeJsonFile(jsonFilePath, jsonContent) {
    await fs.mkdir(path.dirname(jsonFilePath), { recursive: true });
    await fs.writeFile(jsonFilePath, JSON.stringify(jsonContent, null, 2));
    console.log(`JSON file saved at: ${jsonFilePath}`);
}

export async function savePointsToJson(seed, dataSet, selectedCells = []) {
    const jsonFileName = `${seed}.json`;
    const jsonFilePath = path.join(OUTPUT_DIR, jsonFileName);
    
    let jsonContent = await readJsonFile(jsonFilePath);
    
    if (jsonContent) {
        if (selectedCells.length > 0) {
            jsonContent.selectedCells = selectedCells.map(point => ({
                x: point.x,
                y: point.y
            }));
        } else {
            delete jsonContent.selectedCells;
        }
        jsonContent.dataSet = dataSet.map(point => ({
            x: point.x,
            y: point.y
        }));
    } else {
        jsonContent = {
            id: seed,
            mode: null,
            trackSize: null,
            parents: {
                parent1: null,
                parent2: null
            },
            fitness: null,
            dataSet: dataSet.map(point => ({
                x: point.x,
                y: point.y
            }))
        };
        
        if (selectedCells.length > 0) {
            jsonContent.selectedCells = selectedCells.map(point => ({
                x: point.x,
                y: point.y
            }));
        }
    }
    
    await writeJsonFile(jsonFilePath, jsonContent);
}


export async function saveFitnessToJson(seed, mode, trackSize, length, deltaX, deltaY, deltaAngleDegrees) {
    const jsonFileName = `${seed}.json`;
    const jsonFilePath = path.join(OUTPUT_DIR, jsonFileName);
    
    let jsonContent = await readJsonFile(jsonFilePath);
    
    if (jsonContent) {
        jsonContent.mode = mode;
        jsonContent.trackSize = trackSize;
        jsonContent.fitness = {
            length,
            deltaX,
            deltaY,
            deltaAngleDegrees
        };
    } else {
        jsonContent = {
            id: seed,
            mode,
            trackSize,
            parents: {
                parent1: null,
                parent2: null
            },
            fitness: {
                length,
                deltaX,
                deltaY,
                deltaAngleDegrees
            },
            dataSet: []
        };
    }
    
    await writeJsonFile(jsonFilePath, jsonContent);
}