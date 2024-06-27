import express from 'express';
import { generateTrack, getGenerator } from '../trackGen/trackGenerator.js';
import { crossover } from '../genetic/crossoverVoronoi.js';
import { crossover as crossoverConvexHull } from '../genetic/crossoverConvexHull.js';
import { mutation, mutationConvexHull } from '../genetic/mutation.js';
import { simulate } from './simulateTrack.js';
import { BBOX } from '../utils/constants.js';


const app = express();
app.use(express.json());

// Input validation middleware
const validateInput = (req, res, next) => {
    const { mode } = req.body;
    if (!mode || (mode !== 'voronoi' && mode !== 'convexHull')) {
        return res.status(400).json({ error: 'Invalid or missing mode' });
    }
    next();
};

app.post('/evaluate', validateInput, async (req, res, next) => {
    console.log("Received request to /evaluate");
    console.log("Request body:", JSON.stringify(req.body, null, 2));
    
    try {
        const { id, mode, dataSet, selectedCells, trackSize } = req.body;
        console.log("Mode:", mode);
        console.log("DataSet:", dataSet ? "Present" : "Not present");
        console.log("SelectedCells:", selectedCells ? "Present" : "Not present");
        console.log("TrackSize:", trackSize);
        
        let fitnessData;

        if (dataSet && Array.isArray(dataSet) && selectedCells && Array.isArray(selectedCells)) {
            console.log("Valid track data provided");
            fitnessData = await simulate(mode, trackSize, dataSet, selectedCells, id);
            console.log('Fitness data:', fitnessData);
            res.json({ fitness: fitnessData });
        } else {
            console.log("No valid track data provided");
            res.status(400).json({ error: "Invalid or missing track data" });
        }
    } catch (error) {
        console.error('Error in /evaluate:', error);
        next(error);
    }
});

app.post('/crossover', validateInput, async (req, res, next) => {
    console.log("ei")
    try {
        const { parent1, parent2, mode } = req.body;
        if (!parent1 || !parent2 || !parent1.dataSet || !parent2.dataSet || 
            !parent1.selectedCells || !parent2.selectedCells) {
            return res.status(400).json({ error: 'Invalid parent data' });
        }

        // Generate tracks for both parents
        await generateTrack(mode, BBOX, parent1.id, parent1.trackSize, false, parent1.dataSet, parent1.selectedCells);
        const trackGenerator1 = getGenerator();

        await generateTrack(mode, BBOX, parent2.id, parent2.trackSize, false, parent2.dataSet, parent2.selectedCells);
        const trackGenerator2 = getGenerator();

        let result;
        if (mode === 'voronoi') {
            result = crossover(trackGenerator1, trackGenerator2, true);
            res.json({
                offspring: {
                    ds: result.ds,
                    sel: result.sel
                }
            });
        } else { // convexHull
            result = crossoverConvexHull(trackGenerator1, trackGenerator2, true);
            res.json({
                offspring: {
                    ds: result.ds
                }
            });
        }
    } catch (error) {
        console.error('Error in /crossover:', error);
        next(error);
    }
});

app.post('/mutate', validateInput, async (req, res, next) => {
    try {
        const { mode, individual, intensityMutation = 10 } = req.body;
        if (!individual || !individual.dataSet || (mode === 'voronoi' && !individual.selectedCells)) {
            return res.status(400).json({ error: 'Invalid individual data' });
        }
        console.log(individual)
        // Generate the initial track
        await generateTrack(mode, BBOX, individual.id, individual.trackSize, true, individual.dataSet, individual.selectedCells);
        const trackGenerator = getGenerator();

        if (mode === 'voronoi') {
            const mutatedData = mutation(trackGenerator, intensityMutation);
            console.log(mutatedData)
            res.json({
                mutated: {
                    ds: trackGenerator.dataSet, //assumption: voronoi mutation doesn't change the dataset (aka in the dataset there aren't the selectedCells (now mutated))
                    sel: mutatedData
                }
            });
        } else { // convexHull
            const mutatedData = mutationConvexHull(trackGenerator.dataSetHull, intensityMutation);
            res.json({
                mutated: {
                    ds: mutatedData
                }
            });
        }
    } catch (error) {
        console.error('Error in /mutate:', error);
        next(error);
    }
});

// Global error handler
app.use((error, req, res, next) => {
    console.error('Error:', error);
    res.status(500).json({ error: 'Internal server error' });
});

const PORT = 4242;
app.listen(PORT, () => {
    console.log(`MAP-Elites API running on port ${PORT}`);
});