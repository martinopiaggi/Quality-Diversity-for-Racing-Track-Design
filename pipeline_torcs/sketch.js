//modules
const fs = require('fs');
const Voronoi = require('./rhill-voronoi-core')
const seedrandom = require('./seedrandom');
const { exec } = require('child_process');

let bbox = {
    xl: 0,
    xr: 600,
    yt: 0,
    yb: 600
};
let seed = Math.random();
let randomGen = new seedrandom(seed);
let trackSize = 5;
let dataSet = [];
let voronoi = new Voronoi();
let diagram = null;
let trackEdges = [];
let selectedCells = [];
let selectedSites = [];
let xml = '';
const xmlTrackHeader = fs.readFileSync("startTrackTemplate.xml", 'utf8');
const closingXml = "</section>\n</section>\n</params>"

console.log("SEED: "+seed)
console.log("trackSize (# cells): "+trackSize)

//TRACK GENERATION:


generatePoints();
diagram = voronoi.compute(dataSet, bbox);
selectCellsForTrack(trackSize);
findTrackEdges();
  
for(let i = 0; i<10;i++){
  trackEdges = generateCatmullRomSpline(trackEdges,5,i*10); 
  trackEdges = pushApart(trackEdges,20);
  trackEdges = fixAngles(trackEdges);
}

trackEdges = generateCatmullRomSpline(trackEdges,10,0);
trackEdges = pushApart(trackEdges,1);

let minIndex = findMinCurvatureSegment(trackEdges,20);
  
trackEdges = trackEdges.slice(minIndex).concat(trackEdges.slice(0, minIndex));

trackEdges.splice(0,10)
trackEdges.splice(trackEdges.length-10,10)

exportTrackToXML(trackEdges,0); //second argument is starting index of XML


// Execute Torcs Trackgen command + move files into Torcs folder
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

    // If the track generation is successful, move to copying the files
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
    });
});



// --- FUNCTIONS : 

function findMinCurvatureSegment(trackEdges, segmentLength = 20) {
    let minAverageCurvature = Infinity;
    let minSegmentStartIndex = 0;
    let trackLength = trackEdges.length;

    for (let index = 0; index < trackLength; index++) {
        let totalCurvature = 0;

        // total curvature for a segment of 'segmentLength'
        for (let offset = 0; offset < segmentLength; offset++) {
            let curvatureIndex = (index + offset) % trackLength;
            let curvature = calculateCurvature(trackEdges, curvatureIndex);
            totalCurvature += curvature;
        }

        // Calculate average curvature for this segment
        let averageCurvature = totalCurvature / segmentLength;

        // Update the minimum average curvature and its corresponding start index
        if (averageCurvature < minAverageCurvature) {
            minAverageCurvature = averageCurvature;
            minSegmentStartIndex = index;
        }
    }

    return (minSegmentStartIndex+segmentLength)%trackLength; // Return the end index of the segment with the lowest average curvature
}


function exportTrackToXML(trackEdges,startIndex = 0) {
    let previousLength = 0;
    const threshold = 0.001; 
    let segmentNumber = 0;
    let curvature = 0;
    let segInitEndLength = 10; 
    //init segment of 5 meters 
    addSection(segmentNumber, 'straight', segInitEndLength, null);
    segmentNumber++;
  
    for (let index = startIndex; index < startIndex + trackEdges.length - 2; index++) {
        let i = (index) % trackEdges.length;
        let i_next = (index+1) % trackEdges.length;
        let i_nextnext = (index+2) % trackEdges.length;
        const current = trackEdges[i];
        const next = trackEdges[i_next];
        const nextNext = trackEdges[i_nextnext];
        const segment = calculateSegment(current, next);
        
        curvature = calculateCurvature(trackEdges,i)
      
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
                index++; // Skip the next point because it's part of the current curve
            }
        }
    }

    
    let additionalStr = segInitEndLength;
    if (previousLength > 0) {
      let additionalStr = previousLength;
    }
    
    addSection(segmentNumber, 'straight', additionalStr, null);
    segmentNumber++;
    
    fs.mkdir("./tracks/dirt/output/", { recursive: true }, (dirErr) => {
    if (dirErr) {
        console.error('Error creating directory:', dirErr);
        return;
    }})

    fs.writeFile('./tracks/dirt/output/output.xml', xmlTrackHeader + xml + closingXml, (err) => {
        if (err) {
            console.error('Failed to save XML:', err);
        } else {
            console.log('The file has been saved!');
        }
    });
}


function calculateCurvature(trackEdges, i) {
    const current = trackEdges[i];
    const next = trackEdges[(i + 1) % trackEdges.length];
    const nextNext = trackEdges[(i + 2) % trackEdges.length];

    // First "derivative"
    const xp = (nextNext.x - current.x) / 2; 
    const yp = (nextNext.y - current.y) / 2; 

    // Second derivative 
    const xpp = (nextNext.x - 2 * next.x + current.x); 
    const ypp = (nextNext.y - 2 * next.y + current.y); 

    // Calculate curvature using the curvature formula
    const numerator = xp * ypp - yp * xpp;
    const denominator = Math.pow((xp * xp + yp * yp), 1.5);

    return denominator !== 0 ? Math.abs(numerator / denominator) : 0;
}



function addSection(index, type, length, curv) {
        if (type === ('curve')) {   
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
    // Function to calculate the determinant of a matrix
    function determinant(x1, y1, x2, y2, x3, y3) {
        return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
    }

    // Calculate the circumcenter using the determinant
    const D = 2 * determinant(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    if (D === 0){
        return null;  //collinear points
    }

    const ux = ((p1.x ** 2 + p1.y ** 2) * (p2.y - p3.y) + (p2.x ** 2 + p2.y ** 2) * (p3.y - p1.y) + (p3.x ** 2 + p3.y ** 2) * (p1.y - p2.y)) / D;
    const uy = ((p1.x ** 2 + p1.y ** 2) * (p3.x - p2.x) + (p2.x ** 2 + p2.y ** 2) * (p1.x - p3.x) + (p3.x ** 2 + p3.y ** 2) * (p2.x - p1.x)) / D;

    const radius = Math.sqrt((p1.x - ux) ** 2 + (p1.y - uy) ** 2);

    const angle1 = Math.atan2(p1.y - uy, p1.x - ux);
    const angle3 = Math.atan2(p3.y - uy, p3.x - ux);

    let theta = Math.abs(angle3 - angle1) * (180 / Math.PI);
    if (theta > 180) {
      theta = 360 - theta; 
    }

    // Determine the direction using cross product (right-hand rule)
    const dir = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x) > 0 ? 'rgt' : 'lft';

    return {
        dir: dir,
        radius: radius,
        angle: theta
    };
}


function generatePoints() {
    for (let i = 0; i < 50; i++) {
        dataSet.push({
            x: randomGen() * bbox.xr, //randomGern gives [0,1]
            y: randomGen() * bbox.yb
        });
    }
}

function selectCellsForTrack(numCells) {
    let bboxCenter = {
        x: (bbox.xr + bbox.xl) / 2,
        y: (bbox.yb + bbox.yt) / 2
    };
    let minDist = Infinity;
    let startCellIndex = -1;

    diagram.cells.forEach((cell, index) => {
        let site = cell.site;
        let dist = (site.x - bboxCenter.x) ** 2 + (site.y - bboxCenter.y) ** 2;
        if (dist < minDist) {
            minDist = dist;
            startCellIndex = index;
        }
    });

    if (startCellIndex !== -1) {
        let currentCell = diagram.cells[startCellIndex];
        while (selectedCells.length < numCells && currentCell) {
            if (!selectedCells.includes(currentCell)) {
                selectedCells.push(currentCell);
                selectedSites.push(currentCell.site);
            }
            currentCell = getNextCell(currentCell);
        }
    }
}


function getNextCell(cell) {
    let neighbors = [];
    for (let edge of cell.halfedges) {
        let twin = edge.edge.lSite !== cell.site ? edge.edge.lSite : edge.edge.rSite;
        if (twin) {
            neighbors.push(diagram.cells[twin.voronoiId]);
        }
    }
    return neighbors.length > 0 ? neighbors[Math.floor(randomGen()*neighbors.length)] : null;
}

function findTrackEdges() {
    bboxCenter = {
        x: bbox.xr / 2,
        y: bbox.yb / 2
    };
    let edgeMap = new Map();

  selectedCells.flatMap(cell => cell.halfedges.map(halfedge => halfedge.edge)).forEach(edge => edgeMap.set(edge, (edgeMap.get(edge) || 0) + 1));


  let externalEdges = Array.from(edgeMap).filter(([_, count]) => count === 1).map(([edge, _]) => edge);


    let currentEdge = externalEdges[0];
    let currentVertex = currentEdge.vb;


    while (externalEdges.length > 0) {
        trackEdges.push(currentVertex);

        externalEdges = externalEdges.filter(edge => edge !== currentEdge);

        let found = false;
        // Find the next edge that shares the current vertex
        for (let i = 0; i < externalEdges.length; i++) {
            if (comparePoints(externalEdges[i].vb, currentVertex)) {
                currentEdge = externalEdges[i];
                currentVertex = currentEdge.va;
                found = true;
                break;
            }
            if (comparePoints(externalEdges[i].va, currentVertex)) {
                currentEdge = externalEdges[i];
                currentVertex = currentEdge.vb;
                found = true;
                break;
            }
        }
      if(!found){
        break;
      }
    }
}

function comparePoints(point1, point2) {
    return point1.x === point2.x && point1.y === point2.y;
}

function pushApart(points,minDistance = 5) {
    let minDistanceSquared = minDistance * minDistance;  // Square of the minimum distance for comparison
    let i = 0;
    while (i < points.length) {
        let removed = false;
        for (let j = i + 1; j < points.length; j++) {
            let dx = points[j].x - points[i].x;
            let dy = points[j].y - points[i].y;
            let distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < minDistanceSquared) {
                points.splice(j, 1);  // Remove the point at index j
                removed = true;
                break;  // Break the inner loop
            }
        }
        if (!removed) {
            i++;  // Only increment i if no point was removed
        }
    }
    return points;
}



/*
Loop Over Data Points: Adjusts each point relative to its neighbors.
Vector Calculation: Computes vectors to the previous and next points and normalizes them.
Angle Calculation: Uses the perpendicular dot product (also known as the 2D cross product) to determine the angle between the two vectors.
Angle Check and Adjustment: If the angle is too sharp (greater than 100 degrees or less than -100 degrees), it adjusts the next point so that the angle between the vectors is exactly 100 degrees in the direction of the original angle.
*/
function fixAngles(d) {
    const radDeg = 180 / Math.PI;  // Conversion factor from radians to degrees
    const degRad = Math.PI / 180;  // Conversion factor from degrees to radians
    const maxAngle = 80;  // Maximum angle allowed

    for (let i = 0; i < d.length; ++i) {
        let previous = (i - 1 < 0) ? d.length - 1 : i - 1;
        let next = (i + 1) % d.length;

        // Calculate normalized vector to previous point
        let px = d[i].x - d[previous].x;
        let py = d[i].y - d[previous].y;
        let pl = Math.sqrt(px * px + py * py);
        px /= pl;
        py /= pl;

        // Calculate normalized vector to next point (reversed direction)
        let nx = d[next].x - d[i].x;
        let ny = d[next].y - d[i].y;
        let nl = Math.sqrt(nx * nx + ny * ny);
        nx /= nl;
        ny /= nl;

        // Calculate angle between vectors using perp dot product
        let a = Math.atan2(px * ny - py * nx, px * nx + py * ny);  // Angle in radians

        if (Math.abs(a * radDeg) <= maxAngle) continue;  // Check if within acceptable range

        // Adjust the angle to maxAngle
        let nA = maxAngle * Math.sign(a) * degRad;  // New angle in radians
        let diff = nA - a;  // Difference to adjust
        let cos = Math.cos(diff);
        let sin = Math.sin(diff);

        // Rotate and scale the next vector
        let newX = nx * cos - ny * sin;
        let newY = nx * sin + ny * cos;
        newX *= nl;
        newY *= nl;

        // Set new position for the next point
        d[next].x = d[i].x + newX;
        d[next].y = d[i].y + newY;
    }
  
  return d;
}


function generateCatmullRomSpline(data, steps, startIndex) {
  let spline = [];

  for (let i = startIndex; i < startIndex + data.length; i++) {
    let index = i % data.length; // Circular indexing adjusted for the start index
    let p0 = data[(index + data.length - 1) % data.length]; // Previous point
    let p1 = data[index]; // Current point
    let p2 = data[(index + 1) % data.length]; // Next point
    let p3 = data[(index + 2) % data.length]; // Second next point

    for (let t = 0; t <= 1; t += 1/steps) {
      let t2 = t * t;
      let t3 = t2 * t;

      // Catmull-Rom to cubic Bezier conversion coefficients
      let b1 = 0.5 * (-t3 + 2*t2 - t);
      let b2 = 0.5 * (3*t3 - 5*t2 + 2);
      let b3 = 0.5 * (-3*t3 + 4*t2 + t);
      let b4 = 0.5 * (t3 - t2);

      // Calculating the x and y coordinates of the curve
      let x = p0.x * b1 + p1.x * b2 + p2.x * b3 + p3.x * b4;
      let y = p0.y * b1 + p1.y * b2 + p2.y * b3 + p3.y * b4;

      spline.push({x: x, y: y});
    }
  }
  return spline;
}
