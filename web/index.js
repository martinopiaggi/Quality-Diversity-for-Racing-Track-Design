import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const baseDir = path.resolve(__dirname, '..');
const publicDir = path.join(__dirname, 'public');

function generateIndexHtml(dir) {
  const relativePath = path.relative(publicDir, dir);
  const files = fs.readdirSync(dir);
  const fileList = files.map(file => {
    const filePath = path.join(relativePath, file);
    return `<li><a href="/${filePath}">${file}</a></li>`;
  }).join('');

  const indexHtml = `
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Directory Listing</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 20px;
      }

      h1 {
        margin-bottom: 20px;
      }

      ul {
        list-style-type: none;
        padding: 0;
      }

      li {
        margin-bottom: 10px;
      }

      a {
        text-decoration: none;
        color: #0066cc;
      }

      a:hover {
        text-decoration: underline;
      }
    </style>
  </head>
  <body>
    <h1>Directory Listing for /${relativePath}</h1>
    <ul>${fileList}</ul>
  </body>
  </html>
  `;

  fs.writeFileSync(path.join(dir, 'index.html'), indexHtml);
}

function walkDir(dir) {
  const relativePath = path.relative(baseDir, dir);
  const publicPath = path.join(publicDir, relativePath);

  if (dir === publicDir) {
    // Skip processing the public directory itself
    return;
  }

  if (!fs.existsSync(publicPath)) {
    fs.mkdirSync(publicPath, { recursive: true });
  }

  generateIndexHtml(publicPath);

  const files = fs.readdirSync(dir);
  files.forEach(file => {
    const filePath = path.join(dir, file);
    const stat = fs.statSync(filePath);

    if (stat.isDirectory()) {
      // Exclude the .git directory, node_modules directory, and public directory
      if (file !== '.git' && file !== 'node_modules' && file !== 'public') {
        walkDir(filePath);
      }
    } else {
      // Exclude unnecessary files, but include track_visualizer.html and voronoi_cells_visualizer.html
      if (
        !filePath.endsWith('.gitignore') &&
        !filePath.endsWith('package-lock.json') &&
        !filePath.endsWith('index.js') &&
        !filePath.endsWith('server.js') &&
        !filePath.endsWith('netlify.toml')
      ) {
        fs.copyFileSync(filePath, path.join(publicPath, file));
      }
    }
  });
}

walkDir(baseDir);