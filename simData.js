import { exec } from 'child_process';
import fs from 'fs';

const TIMEOUT = 30000; // 30 seconds timeout
const TOTAL_SIMULATIONS = 10;
const CONCURRENCY_LIMIT = 5; // Number of parallel simulations
let failures = 0;
let results = [];
let containers = new Map(); // Map to store index and container ID

// Regex to extract and make json
const extractInfo = (stdout) => {
  let length = 0;
  let seed = null;

  const lengthMatch = stdout.match(/length\s*=\s*([\d.]+)/);
  if (lengthMatch) length = parseFloat(lengthMatch[1]);

  const seedMatch = stdout.match(/SEED:\s+([\d.]+)/);
  if (seedMatch) seed = parseFloat(seedMatch[1]);

  return { seed, length };
};

// Run a single simulation and return a Promise
const runSimulation = (index) => {
  return new Promise((resolve) => {
    let stdout = '';
    let stderr = '';
    let resolved = false;

    const child = exec('sudo node pipelineTorcs.js');

    child.stdout.on('data', (data) => {
      stdout += data;
      // Capture and store container ID
      const containerMatch = data.match(/Docker container started with ID: (\w+)/);
      if (containerMatch) {
        containers.set(index, containerMatch[1]);
      }
    });

    child.stderr.on('data', (data) => {
      stderr += data;
    });

    const handleResult = (success) => {
      if (!resolved) {
        const info = extractInfo(stdout);
        results.push({ success, ...info });
        resolved = true;
        resolve();
      }
    };

    child.on('exit', (code) => {
      clearTimeout(timeout);
      const success = code === 0;
      handleResult(success);
    });

    // Set a timeout to kill the process if it takes too long
    const timeout = setTimeout(() => {
      const containerId = containers.get(index);
      if (containerId) {
        exec(`docker rm --force ${containerId}`, (err, stdout, stderr) => {
          if (err) {
            console.error(`Error stopping/removing container ${containerId}: ${err.message}`);
          } else {
            console.log(`Container ${containerId} removed due to timeout.`);
          }
        });
      }
      child.kill('SIGKILL');
      failures++;
      handleResult(false);
    }, TIMEOUT);
  });
};

// Function to run all simulations in parallel with a concurrency limit
const runAllSimulations = async () => {
  const runningSimulations = new Set();

  const addSimulation = async (index) => {
    const promise = runSimulation(index).then(() => {
      runningSimulations.delete(promise);
    });
    runningSimulations.add(promise);

    if (runningSimulations.size >= CONCURRENCY_LIMIT) {
      await Promise.race(runningSimulations);
    }
  };

  for (let i = 0; i < TOTAL_SIMULATIONS; i++) {
    console.log(`Starting simulation ${i}`);
    await addSimulation(i);
  }

  await Promise.all(runningSimulations);

  fs.writeFileSync('./results.json', JSON.stringify(results, null, 2));
  console.log('Results saved to results.json');
  console.log(`Total failures: ${failures}`);
  process.exit(0);
};

runAllSimulations();
