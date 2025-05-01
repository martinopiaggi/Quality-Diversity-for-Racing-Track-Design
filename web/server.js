//To quickly deploy locally a web interface to look data using express.js
const express = require('express');
const path = require('path');
const app = express();

// Serve static files from 'web' directory itself
app.use(express.static(path.join(__dirname))); // __dirname points to the 'web' directory

// Serve files from 'root' located outside the 'web' directory
app.use('/src', express.static(path.join(__dirname, '..', 'src')));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
