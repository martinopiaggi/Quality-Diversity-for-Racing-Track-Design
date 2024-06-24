import Voronoi from '../lib/rhill-voronoi-core.js';
import { prng_alea } from '../lib/esm-seedrandom/alea.min.mjs';

export class VoronoiTrackGenerator {
    constructor(bbox, seed, size, dataSet = [], selectedVoronoiSites = []) {
        this.bbox = bbox;
        this.randomGen = prng_alea(seed);
        this.dataSet = dataSet.length > 0 ? [...dataSet,...selectedVoronoiSites] : this.generatePoints();
        this.voronoi = new Voronoi();
        this.diagram = this.voronoi.compute(this.dataSet, this.bbox);
        this.patchPath = [];
        this.selectedCells = selectedVoronoiSites.length > 0 ?  
            this.sitesFromInput(selectedVoronoiSites) : this.selectCellsForTrack(size);
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


    sitesFromInput(points) {
        let selectedCells = [];
        points.forEach(p => {
            let cell = this.findCellByCoordinates(p.x, p.y);
            if (cell) {
                selectedCells.push(cell);
            }
        });

        // Check if cells are connected and connect them if not
        this.ensureConnectedCells(selectedCells);

        return selectedCells;
    }

    ensureConnectedCells(selectedCells) {
        const disjointSet = new DisjointSet(selectedCells);

        // Union adjacent cells
        for (let cell of selectedCells) {
            const neighbors = this.getNeighborCells(cell);
            for (let neighbor of neighbors) {
                if (selectedCells.includes(neighbor)) {
                    disjointSet.union(cell, neighbor);
                }
            }
        }

        // Find disconnected components
        const components = new Map();
        for (let cell of selectedCells) {
            const root = disjointSet.find(cell);
            if (!components.has(root)) {
                components.set(root, []);
            }
            components.get(root).push(cell);
        }

        // If more than one component, connect them
        console.log(components.size)
        if (components.size > 1) {
            const componentRoots = Array.from(components.keys());
            for (let i = 1; i < componentRoots.length; i++) {
                const path = this.findShortestPath(componentRoots[i-1], componentRoots[i]);
                if (path) {
                    for (let cell of path) {
                        //if not present add it
                        if (!selectedCells.some(c => this.comparePoints(c.site, cell.site))) {
                            selectedCells.push(cell); //add necessary cells to selectedCells
                            this.patchPath.push(cell); //this is just for visualization/debug purposes
                        }
                    }
                }
            }
        }
    }

    getNeighborCells(cell) {
        const neighbors = [];
        for (let halfedge of cell.halfedges) {
            const neighborSite = halfedge.edge.lSite !== cell.site ? halfedge.edge.lSite : halfedge.edge.rSite;
            if (neighborSite) {
                neighbors.push(this.diagram.cells[neighborSite.voronoiId]);
            }
        }
        return neighbors;
    }

    findShortestPath(startCell, endCell) {
        const queue = [[startCell]];
        const visited = new Set();

        while (queue.length > 0) {
            const path = queue.shift();
            const currentCell = path[path.length - 1];

            if (currentCell === endCell) {
                return path;
            }

            if (!visited.has(currentCell)) {
                visited.add(currentCell);
                const neighbors = this.getNeighborCells(currentCell);
                for (const neighbor of neighbors) {
                    if (!visited.has(neighbor)) {
                        queue.push([...path, neighbor]);
                    }
                }
            }
        }

        return null; // No path found
    }

    

    
    findCellByCoordinates(x, y) {
        for (let cell of this.diagram.cells) {
            if (this.comparePoints(cell.site,{x: x, y: y})) {
                return cell;
            }
        }
        return null;
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


class DisjointSet {
    constructor(elements) {
        this.parent = new Map(elements.map(e => [e, e]));
        this.rank = new Map(elements.map(e => [e, 0]));
    }

    find(element) {
        if (this.parent.get(element) !== element) {
            this.parent.set(element, this.find(this.parent.get(element)));
        }
        return this.parent.get(element);
    }

    union(x, y) {
        let xRoot = this.find(x);
        let yRoot = this.find(y);
        if (xRoot === yRoot) return;

        if (this.rank.get(xRoot) < this.rank.get(yRoot)) {
            this.parent.set(xRoot, yRoot);
        } else if (this.rank.get(xRoot) > this.rank.get(yRoot)) {
            this.parent.set(yRoot, xRoot);
        } else {
            this.parent.set(yRoot, xRoot);
            this.rank.set(xRoot, this.rank.get(xRoot) + 1);
        }
    }
}