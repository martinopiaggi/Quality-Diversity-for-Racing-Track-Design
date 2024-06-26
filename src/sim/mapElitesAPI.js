import express from 'express';
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
    try {
        const { track, mode } = req.body;
        if (!Array.isArray(track)) {
            return res.status(400).json({ error: 'Invalid track data' });
        }
        const fitness = await simulate(mode);
        res.json({ fitness });
        console.log(fitness)
    } catch (error) {
        next(error);
    }
});

app.post('/crossover', validateInput, async (req, res, next) => {
    try {
        const { parent1, parent2, mode } = req.body;
        if (!Array.isArray(parent1) || !Array.isArray(parent2)) {
            return res.status(400).json({ error: 'Invalid parent data' });
        }
        let offspring;
        if (mode === 'voronoi') {
            offspring = await crossover(parent1, parent2);
        } else {
            offspring = await crossoverConvexHull(parent1, parent2);
        }
        res.json({ offspring });
    } catch (error) {
        next(error);
    }
});

app.post('/mutate', validateInput, async (req, res, next) => {
    try {
        const { individual, mode } = req.body;
        if (!Array.isArray(individual)) {
            return res.status(400).json({ error: 'Invalid individual data' });
        }
        let mutated;
        if (mode === 'voronoi') {
            mutated = await mutation(individual);
        } else {
            mutated = await mutationConvexHull(individual);
        }
        res.json({ mutated });
    } catch (error) {
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