// jsonUtils.js
import { promises as fs } from 'fs';
import path from 'path';
import {OUTPUT_DIR} from './constants.js'; 

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
            trackSize: selectedCells.length,
            fitness: null,
            dataSet: dataSet.map(point => ({
                x: point.x,
                y: point.y
            })),
            selectedCells: selectedCells.map(point => ({
                x: point.x,
                y: point.y
            }))
        };
    }
    
    await writeJsonFile(jsonFilePath, jsonContent);
}

export async function saveFitnessToJson(seed, mode, trackSize, fitness) {
    // Create a unique suffix using the current timestamp
    const suffix = `${Date.now()}`;
    const fitnessFileName = `${seed}_${suffix}.json`;
    const fitnessFilePath = path.join(OUTPUT_DIR, fitnessFileName);

    // Attempt to read the original points file (without suffix) to get its dataSet and selectedCells.
    const pointsFileName = `${seed}.json`;
    const pointsFilePath = path.join(OUTPUT_DIR, pointsFileName);
    let originalPoints = await readJsonFile(pointsFilePath);

    // If the points file doesn't exist, default to empty arrays.
    if (!originalPoints) {
        originalPoints = {
            dataSet: [],
            selectedCells: []
        };
    }

    // Build the fitness JSON content. This object includes the new fitness fields
    // and the dataSet and selectedCells read from the original points file.
    const jsonContent = {
        id: seed,
        mode: mode,
        trackSize: trackSize,
        fitness: {
            length: fitness.length,
            deltaX: fitness.deltaX,
            deltaY: fitness.deltaY,
            deltaAngleDegrees: fitness.deltaAngleDegrees,
            speed_entropy: fitness.speed_entropy,
            acceleration_entropy: fitness.acceleration_entropy,
            braking_entropy: fitness.braking_entropy,
            positions_mean: fitness.positions_mean,
            avg_radius_mean: fitness.avg_radius_mean,
            gaps_mean: fitness.gaps_mean,
            right_bends: fitness.right_bends,
            avg_radius_var: fitness.avg_radius_var,
            total_overtakes: fitness.total_overtakes,
            straight_sections: fitness.straight_sections,
            gaps_var: fitness.gaps_var,
            left_bends: fitness.left_bends,
            positions_var: fitness.positions_var,
            curvature_entropy: fitness.curvature_entropy
        },
        // Preserve points data if available.
        dataSet: originalPoints.dataSet || [],
        selectedCells: originalPoints.selectedCells || []
    };

    // Save the fitness JSON to the unique file.
    await writeJsonFile(fitnessFilePath, jsonContent);
}
