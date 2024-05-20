const { exec, execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const TIMEOUT = 15000; // 15 seconds timeout
const TOTAL_SIMULATIONS = 100;
let failures = 0;
let results = [];

// Regex to extract and make json
const extractInfo = (stdout) => {
  let length = 0;
  let seed = null;

  const lengthMatch = stdout.match(/length\s*=\s*([\d.]+)/);
  if (lengthMatch) length = parseFloat(lengthMatch[1]);

  const seedMatch = stdout.match(/SEED:\s+([\d.]+)/);
  if (seedMatch) seed = parseFloat(seedMatch[1]);

  return { seed , length };
};

// Run a single simulation and return a Promise
const runSimulation = (index) => {
  return new Promise((resolve) => {
    let stdout = '';
    let stderr = '';
    let resolved = false;

    const child = exec('sudo node pipelineTorcs.js', { cwd: './pipeline_torcs' });

    child.stdout.on('data', (data) => {
      stdout += data;
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
      child.kill('SIGKILL');
      failures++;
      console.log(`FAILURE, Output before timeout:\n${stdout}`);
      try {
        execSync('taskkill /IM wtorcs.exe /F'); // Kill torcs process if hanging
      } catch (err) {}
      handleResult(false);
    }, TIMEOUT);
  });
};

// Function to run all simulations sequentially
const runAllSimulations = async () => {
  for (let i = 0; i < TOTAL_SIMULATIONS; i++) {
    console.log(`Starting simulation ${i}`);
    await runSimulation(i);
  }
  fs.writeFileSync(path.join(__dirname, 'results.json'), JSON.stringify(results, null, 2));
  console.log('Results saved to results.json');
  console.log(`Total failures: ${failures}`);
  process.exit(0);
};

runAllSimulations();
