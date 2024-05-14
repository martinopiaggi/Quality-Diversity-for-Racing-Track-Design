const fs = require('fs');
const { exec } = require('child_process');
const TrackGenerator = require('./voronoiTrackGenerator');

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 5;
const XML_TRACK_HEADER = fs.readFileSync("startTrackTemplate.xml", 'utf8');
const CLOSING_XML = "</section>\n</section>\n</params>";

// Globals 

let xml = '';

// Track generation
const seed = Math.random();
const trackGenerator = new TrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const splineTrack =  splineSmoothing(trackEdges);

// XML processing for Torcs 
processTrackEdges(splineTrack);

console.log("SEED: " + seed);
console.log("trackSize (# cells): " + TRACK_SIZE);

// Execute CLI commands
generateAndMoveTrackFiles();

function splineSmoothing(spline){
    for (let i = 0; i < 10; i++) {
        spline = generateCatmullRomSpline(spline, 5, i * 10);
        spline = pushApart(spline, 20);
        spline = fixAngles(spline);
    }
    spline = generateCatmullRomSpline(spline, 10, 0);
    spline = pushApart(spline, 1);
    return spline;
}

function processTrackEdges(track) {
    let minIndex = findMinCurvatureSegment(track, 20);
    track = track.slice(minIndex).concat(track.slice(0, minIndex));
    track.splice(0, 10);
    track.splice(track.length - 10, 10);
    exportTrackToXML(track, 0);
}

function findMinCurvatureSegment(track, segmentLength = 20) {
    let minAverageCurvature = Infinity;
    let minSegmentStartIndex = 0;
    let trackLength = track.length;

    for (let index = 0; index < trackLength; index++) {
        let totalCurvature = 0;

        for (let offset = 0; offset < segmentLength; offset++) {
            let curvatureIndex = (index + offset) % trackLength;
            let curvature = calculateCurvature(track, curvatureIndex);
            totalCurvature += curvature;
        }

        let averageCurvature = totalCurvature / segmentLength;
        if (averageCurvature < minAverageCurvature) {
            minAverageCurvature = averageCurvature;
            minSegmentStartIndex = index;
        }
    }

    return (minSegmentStartIndex + segmentLength) % trackLength;
}

function calculateCurvature(track, i) {
    const current = track[i];
    const next = track[(i + 1) % track.length];
    const nextNext = track[(i + 2) % track.length];

    const xp = (nextNext.x - current.x) / 2;
    const yp = (nextNext.y - current.y) / 2;
    const xpp = (nextNext.x - 2 * next.x + current.x);
    const ypp = (nextNext.y - 2 * next.y + current.y);

    const numerator = xp * ypp - yp * xpp;
    const denominator = Math.pow((xp * xp + yp * yp), 1.5);

    return denominator !== 0 ? Math.abs(numerator / denominator) : 0;
}

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
        const segment = calculateSegment(current, next);

        curvature = calculateCurvature(track, i);
        if (curvature < threshold) {
            previousLength += segment.length;
        } else {
            if (previousLength > 0) {
                addSection(segmentNumber, 'straight', previousLength, null);
                segmentNumber++;
                previousLength = 0;
            }
            const curv = calculateCurve(current, next, nextNext);
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

function calculateSegment(point1, point2) {
    const dx = point2.x - point1.x;
    const dy = point2.y - point1.y;
    return Math.sqrt(dx * dx + dy * dy);
}

function calculateCurve(p1, p2, p3) {
    function determinant(x1, y1, x2, y2, x3, y3) {
        return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
    }

    const D = 2 * determinant(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    if (D === 0) return null;

    const ux = ((p1.x ** 2 + p1.y ** 2) * (p2.y - p3.y) + (p2.x ** 2 + p2.y ** 2) * (p3.y - p1.y) + (p3.x ** 2 + p3.y ** 2) * (p1.y - p2.y)) / D;
    const uy = ((p1.x ** 2 + p1.y ** 2) * (p3.x - p2.x) + (p2.x ** 2 + p2.y ** 2) * (p1.x - p3.x) + (p3.x ** 2 + p3.y ** 2) * (p2.x - p1.x)) / D;
    const radius = Math.sqrt((p1.x - ux) ** 2 + (p1.y - uy) ** 2);

    const angle1 = Math.atan2(p1.y - uy, p1.x - ux);
    const angle3 = Math.atan2(p3.y - uy, p3.x - ux);

    let theta = Math.abs(angle3 - angle1) * (180 / Math.PI);
    if (theta > 180) theta = 360 - theta;

    const dir = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x) > 0 ? 'rgt' : 'lft';

    return { dir, radius, angle: theta };
}

function pushApart(points, minDistance = 5) {
    const minDistanceSquared = minDistance * minDistance;
    let i = 0;
    while (i < points.length) {
        let removed = false;
        for (let j = i + 1; j < points.length; j++) {
            const dx = points[j].x - points[i].x;
            const dy = points[j].y - points[i].y;
            const distanceSquared = dx * dx + dy * dy;
            if (distanceSquared < minDistanceSquared) {
                points.splice(j, 1);
                removed = true;
                break;
            }
        }
        if (!removed) {
            i++;
        }
    }
    return points;
}

function fixAngles(d) {
    const radDeg = 180 / Math.PI;
    const degRad = Math.PI / 180;
    const maxAngle = 80;

    for (let i = 0; i < d.length; ++i) {
        let previous = (i - 1 < 0) ? d.length - 1 : i - 1;
        let next = (i + 1) % d.length;

        let px = d[i].x - d[previous].x;
        let py = d[i].y - d[previous].y;
        let pl = Math.sqrt(px * px + py * py);
        px /= pl;
        py /= pl;

        let nx = d[next].x - d[i].x;
        let ny = d[next].y - d[i].y;
        let nl = Math.sqrt(nx * nx + ny * ny);
        nx /= nl;
        ny /= nl;

        let a = Math.atan2(px * ny - py * nx, px * nx + py * ny);
        if (Math.abs(a * radDeg) <= maxAngle) continue;

        let nA = maxAngle * Math.sign(a) * degRad;
        let diff = nA - a;
        let cos = Math.cos(diff);
        let sin = Math.sin(diff);

        let newX = nx * cos - ny * sin;
        let newY = nx * sin + ny * cos;
        newX *= nl;
        newY *= nl;

        d[next].x = d[i].x + newX;
        d[next].y = d[i].y + newY;
    }

    return d;
}

function generateCatmullRomSpline(data, steps, startIndex) {
    let spline = [];

    for (let i = startIndex; i < startIndex + data.length; i++) {
        let index = i % data.length;
        let p0 = data[(index + data.length - 1) % data.length];
        let p1 = data[index];
        let p2 = data[(index + 1) % data.length];
        let p3 = data[(index + 2) % data.length];

        for (let t = 0; t <= 1; t += 1 / steps) {
            let t2 = t * t;
            let t3 = t2 * t;

            let b1 = 0.5 * (-t3 + 2 * t2 - t);
            let b2 = 0.5 * (3 * t3 - 5 * t2 + 2);
            let b3 = 0.5 * (-3 * t3 + 4 * t2 + t);
            let b4 = 0.5 * (t3 - t2);

            let x = p0.x * b1 + p1.x * b2 + p2.x * b3 + p3.x * b4;
            let y = p0.y * b1 + p1.y * b2 + p2.y * b3 + p3.y * b4;

            spline.push({ x, y });
        }
    }
    return spline;
}

function generateAndMoveTrackFiles() {
    exec(`"C:\\Program Files (x86)\\torcs\\trackgen.exe" -c dirt -n output`, (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
        if (stderr) {
            console.error(`stderr: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);

        exec(`powershell -ExecutionPolicy Bypass -File ./copyFilesToTorcs.ps1`, (copyError, copyStdout, copyStderr) => {
            if (copyError) {
                console.error(`exec error: ${copyError}`);
                return;
            }
            if (copyStderr) {
                console.error(`stderr: ${copyStderr}`);
                return;
            }
            console.log(copyStdout);
            console.log(`Track copied to Torcs folder`);

            exec(`wtorcs.exe -nofuel -nodamage -r "./config/raceman/mapelite.xml"`, {
                cwd: "C:\\Program Files (x86)\\torcs"
            }, (torcsError, torcsStdout, torcsStderr) => {
                if (torcsError) {
                    console.error(`exec error: ${torcsError}`);
                    return;
                }
                if (torcsStderr) {
                    console.error(`stderr: ${torcsStderr}`);
                    return;
                }
                console.log(`TORCS Simulation stdout: ${torcsStdout}`);
                console.log(`TORCS simulation completed successfully.`);
            });
        });
    });
}
