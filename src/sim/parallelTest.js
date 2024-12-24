import { exec, spawn } from 'child_process';
import { promisify } from 'util';
import { simulate } from './simulateTrack.js';
import { JSON_DEBUG } from '../utils/constants.js';

const execPromise = promisify(exec);
const TIMEOUT = 35000; // Increased to 35 seconds to account for the 30-second simulation timeout
const TOTAL_SIMULATIONS = 10;
const CONCURRENCY_LIMIT = 25; // Number of parallel simulations

async function runSimulation(simulationIndex) {
    try {
        console.log(`Starting simulation ${simulationIndex}`);

        // Generate random parameters for the simulation
        const mode = Math.random() < 0.5 ? 'voronoi' : 'convexHull';
        const trackSize = mode === 'voronoi' ? Math.ceil(Math.random() * 4) + 1 : 50;
        const seed = Math.random();

        // Run the simulation
        const { fitness } = await Promise.race([
            simulate(mode, trackSize, [], [], seed, JSON_DEBUG),
            new Promise((_, reject) => 
                setTimeout(() => reject(new Error('Simulation timeout')), TIMEOUT)
            )
        ]);

        console.log(`Simulation ${simulationIndex} completed. Fitness:`, fitness);
        return fitness;
    } catch (error) {
        console.error(`Error in simulation ${simulationIndex}: ${error.message}`);
    }
}

async function runSimulations() {
    const simulationPromises = [];
    for (let i = 0; i < TOTAL_SIMULATIONS; i++) {
        simulationPromises.push(runSimulation(i + 1));
        if (simulationPromises.length >= CONCURRENCY_LIMIT) {
            await Promise.all(simulationPromises);
            simulationPromises.length = 0;
        }
    }
    await Promise.all(simulationPromises); // Await any remaining simulations
    console.log('All simulations completed.');
}

runSimulations().catch(err => console.error(`Unexpected error: ${err.message}`));