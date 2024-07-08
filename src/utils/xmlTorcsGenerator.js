import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import * as utils from './utils.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const XML_TRACK_HEADER = fs.readFileSync(path.join(__dirname, 'startTrackTemplate.xml'), 'utf8');
const CLOSING_XML = "</section>\n</section>\n</params>";
let xml = '';

//return XML data ready for trackGen parsing  
// saveXMLalsoLocally is used for testing, it prints at local level the XML as "output.xml"
export function exportTrackToXML(track, startIndex = 0, saveXMLalsoLocally = false) {
    xml = '';     
    let previousLength = 0;
    const threshold = 0.001;
    let segmentNumber = 0;
    let curvature = 0;

    for (let index = startIndex; index < startIndex + track.length - 2; index++) {
        let i = (index) % track.length;
        let i_next = (index + 1) % track.length;
        let i_nextnext = (index + 2) % track.length;
        const current = track[i];
        const next = track[i_next];
        const nextNext = track[i_nextnext];
        const segmentLength = utils.calculateSegment(current, next);

        curvature = utils.calculateCurvature(track, i);
        if (curvature < threshold) {
            previousLength += segmentLength;
        } else {
            if (previousLength > 0) {
                addSection(segmentNumber, 'straight', previousLength, null);
                segmentNumber++;
                previousLength = 0;
            }
            const curv = utils.calculateCurve(current, next, nextNext);
            if (curv) {
                addSection(segmentNumber, 'curve', 0, curv);
                segmentNumber++;
                index++;
            }
        }
    }
    
    if (previousLength > 0) {
        addSection(segmentNumber, 'straight', previousLength, null);
        segmentNumber++;
    }

    const finalTrackOutput = XML_TRACK_HEADER + xml + CLOSING_XML

    if(saveXMLalsoLocally){
        fs.writeFile('output.xml', finalTrackOutput , (err) => {
            if (err) {
                console.error('Failed to save XML:', err);
            }
        });
    }
    
    return finalTrackOutput;
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