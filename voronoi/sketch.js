let bbox = {
    xl: 0,
    xr: 600,
    yt: 0,
    yb: 600
};
let trackSize = 5;
let dataSet = [];
let voronoi = new Voronoi();
let diagram = null;
let trackEdges = [];
let selectedCells = [];
let selectedSites = [];
let originalEdges = [];


function setup() {
    createCanvas(bbox.xr, bbox.yb);
    generatePoints();
    diagram = voronoi.compute(dataSet, bbox);
    selectCellsForTrack(trackSize);
    findTrackEdges();
}

function draw() {
    background(0);
    drawVoronoi();
    drawPoints();
    drawOriginalEdges();
    drawSelectedSites();
    drawTrack();
}

function drawSelectedSites() {
    fill(0, 255, 0); // Set fill color to green
    noStroke();
    selectedSites.forEach(site => {
        ellipse(site.x, site.y, 8, 8); // Draw each site as a green ellipse
    });
}

function generatePoints() {
    for (let i = 0; i < 100; i++) {
        dataSet.push({
            x: random(bbox.xr),
            y: random(bbox.yb)
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
    return neighbors.length > 0 ? neighbors[floor(random(neighbors.length))] : null;
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



function drawOriginalEdges() {
    stroke(0, 255, 255); // Set stroke color to blue for debugging edges
    strokeWeight(2); // Slightly thicker line for visibility
    for (let edge of originalEdges) {
        line(edge.va.x, edge.va.y, edge.vb.x, edge.vb.y);
    }
    strokeWeight(1); // Reset stroke weight for other drawings
}

function drawVoronoi() {
    stroke(150);
    diagram.edges.forEach(edge => line(edge.va.x, edge.va.y, edge.vb.x, edge.vb.y));
}

function drawTrack() {
    stroke(255, 255, 0);
    strokeWeight(1);
    beginShape();
    trackEdges.forEach((v, i, arr) => {
        line(v.x, v.y, arr[(i + 1) % arr.length].x, arr[(i + 1) % arr.length].y);
        fill(255, 0, 0);
        fill(0);
    });
    endShape(CLOSE);
    strokeWeight(1);
}

function drawPoints() {
    fill(255);
    noStroke();
    dataSet.forEach(pt => ellipse(pt.x, pt.y, 5, 5));
}