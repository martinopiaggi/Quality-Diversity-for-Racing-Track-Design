const fs = require('fs');
const path = require('path');
const directoryPath = path.join("..", 'data', 'tests');

fs.readdir(directoryPath, function (err, files) {
    if (err) {
        return console.log('Unable to scan directory: ' + err);
    }
    const jsonFiles = files.filter(file => file.endsWith('.json'));
    fs.writeFileSync(path.join(directoryPath, 'index.json'), JSON.stringify(jsonFiles));
});
