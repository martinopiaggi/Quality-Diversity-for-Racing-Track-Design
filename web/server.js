import express from 'express';
import serveIndex from 'serve-index';
import path from 'path';
import { fileURLToPath } from 'url';
import { promises as fs } from 'fs';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PUBLIC_DIR = path.join(__dirname, 'public');
const TESTS_DIR = path.join(PUBLIC_DIR, 'data', 'tests');

// Custom template for directory listings
const template = `
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="utf-8">
    <title><%- title %></title>
    <style>
      body {
        font-family: Arial, sans-serif;
      }
      ul {
        list-style: none;
        padding: 0;
      }
      li {
        margin-bottom: 10px;
      }
      a {
        text-decoration: none;
        color: #333;
      }
    </style>
  </head>
  <body>
    <h1><%- title %></h1>
    <ul>
      <% files.forEach(function(file) { %>
        <% if (file.name !== 'index.html') { %>
          <li><a href="<%= file.href %>"><%= file.name %></a></li>
        <% } %>
      <% }); %>
    </ul>
  </body>
  </html>
`;

// Middleware to serve static files and directory listing from the public directory
app.use(express.static(PUBLIC_DIR));
app.use('/data/tests', express.static(TESTS_DIR), serveIndex(TESTS_DIR, { template, icons: true }));

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

// Catch-all route to serve index.html for any unmatched routes
app.get('*', (req, res) => {
  res.sendFile(path.join(PUBLIC_DIR, 'index.html'));
});

app.listen(3000, () => {
  console.log('Server is running on http://localhost:3000');
});