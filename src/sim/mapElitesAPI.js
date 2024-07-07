import express from 'express';
import { generateTrack } from '../trackGen/trackGenerator.js';
import { crossover } from '../genetic/crossoverVoronoi.js';
import { crossover as crossoverConvexHull } from '../genetic/crossoverConvexHull.js';
import { mutation, mutationConvexHull } from '../genetic/mutation.js';
import { BBOX, JSON_DEBUG } from '../utils/constants.js';
import { simulate } from './simulateTrack.js';


const app = express();
app.use(express.json());


app.post('/generate', async (req, res) => {
    try {
        const { id, mode, trackSize } = req.body;
        //id is used as seed for the generation of the track
        const { track, generator } = await generateTrack(mode, BBOX, id, trackSize, JSON_DEBUG);
        
        const response = {
            id: id,
            mode: mode,
            dataSet: generator.dataSet,
            selectedCells: generator.selectedCells.map(cell => ({ x: cell.site.x, y: cell.site.y })),
            trackSize: generator.trackSize
        };
        
        res.json(response);
    } catch (error) {
        console.error('Error in /generate:', error);
        res.status(500).json({ error: error.message });
    }
});

app.post('/evaluate', async (req, res) => {
    try {
        const { id, mode, dataSet, selectedCells } = req.body;
        const simulationResult = await simulate(mode, selectedCells.length, dataSet, selectedCells, id,JSON_DEBUG);
        res.json({
            fitness: simulationResult.fitness
        });
    } catch (error) {
        console.error('Error in /evaluate:', error);
        res.status(500).json({ error: error.message });
    }
});


app.post('/crossover', async (req, res, next) => {
    console.log("Crossover endpoint called")
    try {
        const { parent1, parent2, mode } = req.body;
        if (!parent1 || !parent2 || !parent1.dataSet || !parent2.dataSet || 
            !parent1.selectedCells || !parent2.selectedCells) {
            return res.status(400).json({ error: 'Invalid parent data' });
        }

        // Generate tracks for both parents
        const { generator: trackGenerator1 } = await generateTrack(mode, BBOX, parent1.id, parent1.trackSize, false, parent1.dataSet, parent1.selectedCells);
        const { generator: trackGenerator2 } = await generateTrack(mode, BBOX, parent2.id, parent2.trackSize, false, parent2.dataSet, parent2.selectedCells);

        let result;
        if (mode === 'voronoi') {
            console.log("CROSSOVER VORONOI")
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

app.post('/mutate', async (req, res, next) => {
    try {
        const {individual, intensityMutation = 10 } = req.body;
        if (!individual || !individual.dataSet) {
            return res.status(400).json({ error: 'Invalid individual data' });
        }
        // Generate the initial track
        const { generator: trackGenerator } = await generateTrack(individual.mode, BBOX, individual.id, individual.trackSize, false, individual.dataSet, individual.selectedCells);

        if (individual.mode === 'voronoi') {
            const mutatedData = mutation(trackGenerator, intensityMutation);
            res.json({
                mutated: {
                    dataSet: mutatedData.ds, 
                    selectedCells: mutatedData.sel
                }
            });
        } else if (individual.mode === 'convexHull') { 
            const mutatedData = mutationConvexHull(trackGenerator, intensityMutation);
            res.json({
                mutated: {
                    dataSet: mutatedData.ds
                }
            });
        }
        else{
            return res.status(400).json({ error: 'Invalid track generation mode in /mutate:'});
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