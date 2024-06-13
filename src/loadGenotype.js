const fs = require('fs');

// Function to load JSON data from a file
function loadGenotypeFromFile(filePath) {
    try {
        const data = fs.readFileSync(filePath, 'utf8');
        return JSON.parse(data);
    } catch (err) {
        console.error('Error reading file:', err);
        process.exit(1);
    }
}

// Main function to handle logic based on command line args
function main() {
    const args = process.argv.slice(2); // Ignore the first two default elements
    let genotype;

    // Check if the --json flag is used
    const jsonIndex = args.indexOf('--json');
    if (jsonIndex > -1 && args.length > jsonIndex + 1) {
        const jsonFilePath = args[jsonIndex + 1];
        genotype = loadGenotypeFromFile(jsonFilePath);
        console.log('Loaded genotype from JSON:', genotype);
    } else {
        // If no --json flag or file path, assume default behavior or exit
        console.log('No JSON file specified, or missing --json argument.');
        // Here, either provide a default genotype or exit the process
        // For example:
        // genotype = { id: 1, points: [{ x: 0.1, y: 0.2 }], parents: { parent1: null, parent2: null }};
        process.exit(0);
    }

    // Continue with the rest of your program logic using the 'genotype' object
}

main();
