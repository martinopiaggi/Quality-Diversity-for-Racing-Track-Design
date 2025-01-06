import { exec, spawn } from 'child_process';
import { promisify } from 'util';
import { simulate } from './simulateTrack.js';
import { JSON_DEBUG,  SIMULATION_TIMEOUT, } from '../utils/constants.js';

const TOTAL_SIMULATIONS = 100;
const CONCURRENCY_LIMIT = 5; // Number of parallel simulations

async function runSimulation(simulationIndex) {
    try {
        console.log(`Starting simulation ${simulationIndex}`);

        // Generate random parameters for the simulation
        const mode = Math.random() < 0.5 ? 'voronoi' : 'convexHull';
        const trackSize = mode === 'voronoi' ? Math.ceil(Math.random() * 4) + 1 : 50;
        const seed = Math.random();

        // Run the simulation
        const { fitness } = await Promise.race([
            simulate(mode, trackSize, [], [], seed, JSON_DEBUG, false),
            new Promise((_, reject) => 
                setTimeout(() => reject(new Error('Simulation timeout')), SIMULATION_TIMEOUT)
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
}

runSimulations().catch(err => console.error(`Unexpected error: ${err.message}`));