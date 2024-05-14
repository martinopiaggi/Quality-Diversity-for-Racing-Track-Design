const fs = require('fs');
const utils = require('./utils');
const XML_TRACK_HEADER = fs.readFileSync("utils/startTrackTemplate.xml", 'utf8');
const CLOSING_XML = "</section>\n</section>\n</params>";
let xml = '';

function exportTrackToXML(track, startIndex = 0) {
    let previousLength = 0;
    const threshold = 0.001;
    let segmentNumber = 0;
    let curvature = 0;
    const segInitEndLength = 10;

    addSection(segmentNumber, 'straight', segInitEndLength, null);
    segmentNumber++;

    for (let index = startIndex; index < startIndex + track.length - 2; index++) {
        let i = (index) % track.length;
        let i_next = (index + 1) % track.length;
        let i_nextnext = (index + 2) % track.length;
        const current = track[i];
        const next = track[i_next];
        const nextNext = track[i_nextnext];
        const segment = utils.calculateSegment(current, next);

        curvature = utils.calculateCurvature(track, i);
        if (curvature < threshold) {
            previousLength += segment.length;
        } else {
            if (previousLength > 0) {
                addSection(segmentNumber, 'straight', previousLength, null);
                segmentNumber++;
                previousLength = 0;
            }
            const curv = utils.calculateCurve(current, next, nextNext);
            if (curv) {
                addSection(segmentNumber, 'curve', segment.length, curv);
                segmentNumber++;
                index++;
            }
        }
    }

    let additionalStr = segInitEndLength;
    if (previousLength > 0) {
        additionalStr = previousLength;
    }

    addSection(segmentNumber, 'straight', additionalStr, null);
    segmentNumber++;

    fs.mkdir("./tracks/dirt/output/", { recursive: true }, (dirErr) => {
        if (dirErr) {
            console.error('Error creating directory:', dirErr);
            return;
        }
    });

    fs.writeFile('./tracks/dirt/output/output.xml', XML_TRACK_HEADER + xml + CLOSING_XML, (err) => {
        if (err) {
            console.error('Failed to save XML:', err);
        } else {
            console.log('The file has been saved!');
        }
    });
}

function addSection(index, type, length, curv) {
    if (type === 'curve') {
        xml += `  <section name="c${index}">\n`;
        xml += `    <attstr name="type" val="${curv.dir}"/>\n`;
        xml += `    <attnum name="radius" unit="m" val="${curv.radius}"/>\n`;
        xml += `    <attnum name="arc" unit="deg" val="${curv.angle}"/>\n`;
        xml += '  </section>\n';
    } else {
        xml += `  <section name="s${index}">\n`;
        xml += `    <attstr name="type" val="str"/>\n`;
        xml += `    <attnum name="lg" unit="m" val="${length}"/>\n`;
        xml += '  </section>\n';
    }
}

module.exports = {
    exportTrackToXML
};