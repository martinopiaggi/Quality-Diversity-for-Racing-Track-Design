import { exec, spawn } from 'child_process';
import { promisify } from 'util';

const execPromise = promisify(exec);
const TIMEOUT = 14000; // milliseconds timeout
const TOTAL_SIMULATIONS = 100;
const CONCURRENCY_LIMIT = 25; // Number of parallel simulations

async function runSimulation(simulationIndex) {
    let containerId = null;
    let simulationTimeout;

    try {
        console.log(`Starting simulation ${simulationIndex}`);
        
        // Run the pipelineTorcs script and get the container ID
        const pipelineProcess = spawn('node', ['pipelineTorcs.js'], { shell: true });
        let scriptOutput = '';

        // Capture the container ID from the pipeline script output
        const containerIdPromise = new Promise((resolve, reject) => {
            pipelineProcess.stdout.on('data', (data) => {
                scriptOutput += data.toString();
                const match = scriptOutput.match(/Docker container started with ID: (\w+)/);
                if (match) {
                    containerId = match[1];
                    console.log(`Container started for simulation ${simulationIndex}: ${containerId}`);
                    resolve(containerId);
                }
            });

            pipelineProcess.on('exit', () => {
                if (!containerId) {
                    reject(new Error('Container ID not captured before process exit.'));
                }
            });

            pipelineProcess.on('error', (err) => {
                reject(err);
            });
        });

        // Set up a timeout to kill the container if the simulation takes too long
        simulationTimeout = setTimeout(async () => {
            if (containerId) {
                await killContainer(containerId, simulationIndex);
            }
            pipelineProcess.kill();
        }, TIMEOUT);

        // Wait for the container ID and process to complete
        await containerIdPromise;

        const exitCode = await new Promise((resolve, reject) => {
            pipelineProcess.on('exit', (code) => {
                clearTimeout(simulationTimeout);
                resolve(code);
            });

            pipelineProcess.on('error', (err) => {
                clearTimeout(simulationTimeout);
                reject(err);
            });
        });

        if (exitCode !== 0) {
            throw new Error(`Pipeline script exited with code ${exitCode}`);
        }

    } catch (error) {
        if (error.message === 'Simulation timed out.') {
            console.error(`Simulation ${simulationIndex} timed out.`);
        } else {
            console.error(`Error in simulation ${simulationIndex}: ${error.message}`);
        }
    } finally {
        if (containerId) {
            await killContainer(containerId, simulationIndex);
        }
    }
}

async function killContainer(containerId, simulationIndex) {
    try {
        await execPromise(`docker rm --force ${containerId}`);
        console.log(`Container killed for simulation ${simulationIndex}: ${containerId}`);
    } catch (err) {
        console.error(`Failed to kill container for simulation ${simulationIndex} (${containerId}): ${err.message}`);
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
