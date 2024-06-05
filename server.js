import express from 'express';
import serveIndex from 'serve-index';
import path from 'path';
import { promises as fs } from 'fs';

const app = express();
const __dirname = path.resolve();
const BASE_DIR = __dirname;

// Middleware to serve static files and directory listing
app.use(express.static(BASE_DIR));
app.use(serveIndex(BASE_DIR, { icons: true }));

// Serve the gallery.html file on the root route
app.get('/', (req, res) => {
  res.sendFile(path.join(BASE_DIR, 'gallery.html'));
});

app.get('/load-track-data', async (req, res) => {
  try {
    const TESTS_DIR = path.join(BASE_DIR, 'testing/tests');
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
