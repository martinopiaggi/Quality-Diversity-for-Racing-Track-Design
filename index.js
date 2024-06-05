import fs from 'fs';
import path from 'path';

const baseDir = path.resolve('.');

function generateIndexHtml(dir) {
  const files = fs.readdirSync(dir);
  const fileList = files.map(file => {
    const filePath = path.join(dir, file);
    const relativePath = path.relative(baseDir, filePath).replace(/\\/g, '/');
    const isJsonFile = file.endsWith('.json');
    const dataAttr = isJsonFile ? ` data-json-file="/${relativePath}"` : '';
    return `<li><a href="/${relativePath}"${dataAttr}>${file}</a></li>`;
  }).join('');

  const indexHtml = `
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Directory Listing</title>
  </head>
  <body>
    <h1>Directory Listing for ${path.relative(baseDir, dir).replace(/\\/g, '/')}</h1>
    <ul>${fileList}</ul>
  </body>
  </html>
  `;

  fs.writeFileSync(path.join(dir, 'index.html'), indexHtml);
}

function walkDir(dir) {
  generateIndexHtml(dir);
  const files = fs.readdirSync(dir);
  files.forEach(file => {
    const filePath = path.join(dir, file);
    if (fs.statSync(filePath).isDirectory()) {
      walkDir(filePath);
    }
  });
}

walkDir(baseDir);
