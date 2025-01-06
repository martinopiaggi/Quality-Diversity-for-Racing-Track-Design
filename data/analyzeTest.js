const fs = require('fs/promises');
const path = require('path');

const JSON_DIR = './tests';

async function collectData() {
    try {
        const files = await fs.readdir(JSON_DIR);
        const jsonFiles = files.filter(file => file.endsWith('.json'));

        // Initialize accumulators for all metrics
        const metrics = {
            length: { total: 0, sum_squares: 0 },
            deltaX: { total: 0, sum_squares: 0 },
            deltaY: { total: 0, sum_squares: 0 },
            deltaAngleDegrees: { total: 0, sum_squares: 0 },
            speed_entropy: { total: 0, sum_squares: 0 },
            acceleration_entropy: { total: 0, sum_squares: 0 },
            braking_entropy: { total: 0, sum_squares: 0 },
            positions_mean: { total: 0, sum_squares: 0 },
            avg_radius_mean: { total: 0, sum_squares: 0 },
            gaps_mean: { total: 0, sum_squares: 0 },
            right_bends: { total: 0, sum_squares: 0 },
            avg_radius_var: { total: 0, sum_squares: 0 },
            total_overtakes: { total: 0, sum_squares: 0 },
            straight_sections: { total: 0, sum_squares: 0 },
            gaps_var: { total: 0, sum_squares: 0 },
            left_bends: { total: 0, sum_squares: 0 },
            positions_var: { total: 0, sum_squares: 0 },
            curvature_entropy: { total: 0, sum_squares: 0 }
        };

        let count = 0;
        let perfectTrackCount = 0;
        let veryBadTrackCount = 0;
        let perfectTracks = [];
        let veryBadTracks = [];

        // First pass: collect sums for averages
        for (const file of jsonFiles) {
            const filePath = path.join(JSON_DIR, file);
            const data = JSON.parse(await fs.readFile(filePath, 'utf-8'));
            
            if (!data.fitness) continue; // Skip if no fitness data

            const fitness = data.fitness;
            count++;

            // Update all metrics
            for (const [key, value] of Object.entries(metrics)) {
                if (fitness[key] !== null && fitness[key] !== undefined) {
                    value.total += fitness[key];
                }
            }

            // Track quality checks
            if (Math.abs(fitness.deltaX) < 1 && 
                Math.abs(fitness.deltaY) < 1 && 
                Math.abs(fitness.deltaAngleDegrees) < 10) {
                perfectTrackCount++;
                perfectTracks.push({
                    seed: data.id,
                    trackSize: data.trackSize,
                    mode: data.mode,
                    metrics: { ...fitness }
                });
            }

            if ((Math.abs(fitness.deltaX) + Math.abs(fitness.deltaY) > 6)) {
                veryBadTrackCount++;
                veryBadTracks.push({
                    seed: data.id,
                    trackSize: data.trackSize,
                    mode: data.mode,
                    metrics: { ...fitness }
                });
            }
        }

        // Calculate averages
        const averages = {};
        for (const [key, value] of Object.entries(metrics)) {
            averages[key] = value.total / count;
        }

        // Second pass: calculate variances
        for (const file of jsonFiles) {
            const filePath = path.join(JSON_DIR, file);
            const data = JSON.parse(await fs.readFile(filePath, 'utf-8'));
            
            if (!data.fitness) continue;
            
            const fitness = data.fitness;

            // Update sum of squares for all metrics
            for (const [key, value] of Object.entries(metrics)) {
                if (fitness[key] !== null && fitness[key] !== undefined) {
                    value.sum_squares += Math.pow(fitness[key] - averages[key], 2);
                }
            }
        }

        // Calculate standard deviations
        const stdDevs = {};
        for (const [key, value] of Object.entries(metrics)) {
            stdDevs[key] = Math.sqrt(value.sum_squares / (count - 1));
        }

        // Print results
        console.log(`\nAnalysis Results (${count} valid tracks):`);
        console.log('\nBasic Statistics:');
        for (const [key, avg] of Object.entries(averages)) {
            console.log(`${key}:`);
            console.log(`  Average: ${avg.toFixed(4)}`);
            console.log(`  Std Dev: ${stdDevs[key].toFixed(4)}`);
        }

        console.log('\nTrack Quality Analysis:');
        console.log(`Perfect Tracks: ${perfectTrackCount}`);
        console.log(`Problematic Tracks: ${veryBadTrackCount}`);

        if (perfectTrackCount > 0) {
            console.log('\nPerfect Tracks Details:');
            perfectTracks.forEach(track => {
                console.log(`\nSeed: ${track.seed}`);
                console.log(`Track Size: ${track.trackSize}`);
                console.log(`Mode: ${track.mode}`);
                console.log('Key Metrics:');
                console.log(`  Length: ${track.metrics.length.toFixed(2)}`);
                console.log(`  Speed Entropy: ${track.metrics.speed_entropy.toFixed(2)}`);
                console.log(`  Curvature Entropy: ${track.metrics.curvature_entropy.toFixed(2)}`);
            });
        }

        if (veryBadTrackCount > 0) {
            console.log('\nProblematic Tracks Details:');
            veryBadTracks.forEach(track => {
                console.log(`\nSeed: ${track.seed}`);
                console.log(`Track Size: ${track.trackSize}`);
                console.log(`Mode: ${track.mode}`);
                console.log('Issues:');
                console.log(`  Delta X: ${track.metrics.deltaX.toFixed(2)}`);
                console.log(`  Delta Y: ${track.metrics.deltaY.toFixed(2)}`);
                console.log(`  Delta Angle: ${track.metrics.deltaAngleDegrees.toFixed(2)}`);
            });
        }

    } catch (err) {
        console.error(`Error: ${err.message}`);
    }
}

collectData();