import Voronoi from '../lib/rhill-voronoi-core.js';
import { prng_alea } from '../lib/esm-seedrandom/alea.min.mjs';

export class VoronoiTrackGenerator {
constructor(bbox, seed, trackSize, dataSet = [], selectedVoronoiSites = []) {
        this.bbox = bbox;
        this.randomGen = prng_alea(seed);
        this.voronoi = new Voronoi();
        this.trackSize = trackSize;
        this.dataSet = dataSet.length > 0 ? dataSet : this.generatePoints();
        this.diagram = this.voronoi.compute(this.dataSet, this.bbox);
        
        this.patchPath = [];
        this.selectedCells = selectedVoronoiSites.length > 0 ?  
            this.sitesFromInput(selectedVoronoiSites) : this.selectCellsForTrack(trackSize);
        this.trackEdges = this.findTrackEdges();
    }

    generatePoints() {
        return Array.from({length: 50}, () => ({
            x: this.randomGen() * this.bbox.xr,
            y: this.randomGen() * this.bbox.yb
        }));
    }

    sitesFromInput(points) {
        const selectedCells = points
            .map(p => VoronoiDiagramHelper.findCellByCoordinates(p.x, p.y, this.diagram.cells))
            .filter(cell => cell);

        this.ensureConnectedCells(selectedCells);
        return selectedCells;
    }

    ensureConnectedCells(selectedCells) {
        const disjointSet = new DisjointSet(selectedCells);

        // Union adjacent cells
        for (let cell of selectedCells) {
            const neighbors = VoronoiDiagramHelper.getNeighborCells(cell, this.diagram);
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
        if (components.size > 1) {
            const componentRoots = Array.from(components.keys());
            for (let i = 1; i < componentRoots.length; i++) {
                const path = PathFinder.findShortestPath(componentRoots[i-1], componentRoots[i], this.diagram);
                if (path) {
                    for (let cell of path) {
                        if (!selectedCells.some(c => VoronoiDiagramHelper.comparePoints(c.site, cell.site))) {
                            selectedCells.push(cell);
                            this.patchPath.push(cell);
                        }
                    }
                }
            }
        }
    }

    selectCellsForTrack(numCells) {
        const bboxCenter = { 
            x: (this.bbox.xr + this.bbox.xl) / 2, 
            y: (this.bbox.yb + this.bbox.yt) / 2 
        };

        const startCell = this.diagram.cells.reduce((closest, cell) => {
            const dist = (cell.site.x - bboxCenter.x) ** 2 + (cell.site.y - bboxCenter.y) ** 2;
            return dist < closest.dist ? { cell, dist } : closest;
        }, { cell: null, dist: Infinity }).cell;

        const selectedCells = [];
        let currentCell = startCell;

        while (selectedCells.length < numCells && currentCell) {
            if (!selectedCells.includes(currentCell)) {
                selectedCells.push(currentCell);
            }
            currentCell = this.getNextCell(currentCell);
        }

        return selectedCells;
    }

    getNextCell(cell) {
        const neighbors = VoronoiDiagramHelper.getNeighborCells(cell, this.diagram);
        return neighbors.length > 0 ? neighbors[Math.floor(this.randomGen() * neighbors.length)] : null;
    }

    findTrackEdges() {
        const edgeMap = new Map();
        this.selectedCells.flatMap(cell => cell.halfedges.map(halfedge => halfedge.edge))
            .forEach(edge => edgeMap.set(edge, (edgeMap.get(edge) || 0) + 1));
        
        let externalEdges = Array.from(edgeMap)
            .filter(([_, count]) => count === 1)
            .map(([edge, _]) => edge);
    
        const trackPoints = [];
        let currentEdge = externalEdges[Math.floor(externalEdges.length/2)];
        let currentVertex = currentEdge.vb;
    
        while (externalEdges.length > 0) {
            trackPoints.push(currentVertex);
            externalEdges = externalEdges.filter(edge => edge !== currentEdge);
    
            const nextEdge = externalEdges.find(edge => 
                VoronoiDiagramHelper.comparePoints(edge.vb, currentVertex) || 
                VoronoiDiagramHelper.comparePoints(edge.va, currentVertex)
            );
    
            if (!nextEdge) break;
    
            currentEdge = nextEdge;
            currentVertex = VoronoiDiagramHelper.comparePoints(nextEdge.vb, currentVertex) ? nextEdge.va : nextEdge.vb;
        }
    
        return trackPoints;
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



class VoronoiDiagramHelper {
    static getNeighborCells(cell, diagram) {
        return cell.halfedges
            .map(halfedge => halfedge.edge.lSite !== cell.site ? halfedge.edge.lSite : halfedge.edge.rSite)
            .filter(site => site)
            .map(site => diagram.cells[site.voronoiId]);
    }

    static comparePoints(point1, point2) {
        return point1.x === point2.x && point1.y === point2.y;
    }

    static findCellByCoordinates(x, y, cells) {
        return cells.find(cell => VoronoiDiagramHelper.comparePoints(cell.site, {x, y}));
    }
}

class PathFinder {
    static findShortestPath(startCell, endCell, diagram) {
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
                const neighbors = VoronoiDiagramHelper.getNeighborCells(currentCell, diagram);
                for (const neighbor of neighbors) {
                    if (!visited.has(neighbor)) {
                        queue.push([...path, neighbor]);
                    }
                }
            }
        }

        return null; // No path found
    }
}
