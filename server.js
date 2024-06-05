import express from 'express';
import { promises as fs } from 'fs';
import path from 'path';

const app = express();
const __dirname = path.resolve();
const TESTS_DIR = path.join(__dirname, 'testing/tests');

// Middleware to serve static files
app.use(express.static(__dirname));

// Serve the gallery.html file on the root route
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'gallery.html'));
});

app.get('/load-track-data', async (req, res) => {
  try {
    const files = (await fs.readdir(TESTS_DIR)).filter(file => file.endsWith('.json'));
    const trackDataPromises = files.map(async file => {
      const data = await fs.readFile(path.join(TESTS_DIR, file), 'utf-8');
      return JSON.parse(data);
    });
    const trackData = await Promise.all(trackDataPromises);
    res.json(trackData);
  } catch (error) {
    console.error('Error loading track data:', error);
    res.status(500).json({ error: 'Failed to load track data' });
  }
});

app.listen(3000, () => {
  console.log('Server is running on http://localhost:3000');
});
