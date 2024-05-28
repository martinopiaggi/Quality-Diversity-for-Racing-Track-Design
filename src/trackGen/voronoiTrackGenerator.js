import Voronoi from '../libraries/rhill-voronoi-core.js';
import { prng_alea } from '../libraries/esm-seedrandom/alea.min.mjs';

export class VoronoiTrackGenerator {
    constructor(bbox, seed,size) {
        this.bbox = bbox;
        this.randomGen = prng_alea(seed);
        this.dataSet = this.generatePoints();
        this.voronoi = new Voronoi();
        this.diagram = this.voronoi.compute(this.dataSet, this.bbox);
        this.selectedCells = this.selectCellsForTrack(size);
        this.trackEdges = this.findTrackEdges();
    }

    generatePoints() {
        let points = [];
        for (let i = 0; i < 50; i++) {
            points.push({
                x: this.randomGen() * this.bbox.xr,
                y: this.randomGen() * this.bbox.yb
            });
        }
        return points;
    }

    selectCellsForTrack(numCells) {
        let bboxCenter = { x: (this.bbox.xr + this.bbox.xl) / 2, y: (this.bbox.yb + this.bbox.yt) / 2 };
        let minDist = Infinity;
        let startCellIndex = -1;
        let selectedCells = [];

        this.diagram.cells.forEach((cell, index) => {
            let site = cell.site;
            let dist = (site.x - bboxCenter.x) ** 2 + (site.y - bboxCenter.y) ** 2;
            if (dist < minDist) {
                minDist = dist;
                startCellIndex = index;
            }
        });

        if (startCellIndex !== -1) {
            let currentCell = this.diagram.cells[startCellIndex];
            while (selectedCells.length < numCells && currentCell) {
                if (!selectedCells.includes(currentCell)) {
                    selectedCells.push(currentCell);
                }
                currentCell = this.getNextCell(currentCell);
            }
        }
        return selectedCells;
    }

    getNextCell(cell) {
        let neighbors = [];
        for (let edge of cell.halfedges) {
            let twin = edge.edge.lSite !== cell.site ? edge.edge.lSite : edge.edge.rSite;
            if (twin) {
                neighbors.push(this.diagram.cells[twin.voronoiId]);
            }
        }
        return neighbors.length > 0 ? neighbors[Math.floor(this.randomGen() * neighbors.length)] : null;
    }

    findTrackEdges() {
        let trackPoints = [];
        let edgeMap = new Map();
        this.selectedCells.flatMap(cell => cell.halfedges.map(halfedge => halfedge.edge)).forEach(edge => edgeMap.set(edge, (edgeMap.get(edge) || 0) + 1));
        let externalEdges = Array.from(edgeMap).filter(([_, count]) => count === 1).map(([edge, _]) => edge);

        let currentEdge = externalEdges[0];
        let currentVertex = currentEdge.vb;

        while (externalEdges.length > 0) {
            trackPoints.push(currentVertex);
            externalEdges = externalEdges.filter(edge => edge !== currentEdge);

            let found = false;
            for (let i = 0; i < externalEdges.length; i++) {
                if (this.comparePoints(externalEdges[i].vb, currentVertex)) {
                    currentEdge = externalEdges[i];
                    currentVertex = currentEdge.va;
                    found = true;
                    break;
                }
                if (this.comparePoints(externalEdges[i].va, currentVertex)) {
                    currentEdge = externalEdges[i];
                    currentVertex = currentEdge.vb;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        return trackPoints;
    }

    comparePoints(point1, point2) {
        return point1.x === point2.x && point1.y === point2.y;
    }
}
