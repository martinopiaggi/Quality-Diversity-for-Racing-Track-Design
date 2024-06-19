import { pushApart, fixAngles, generateCatmullRomSpline } from '../utils/utils.js';
import { prng_alea } from '../lib/esm-seedrandom/alea.min.mjs';

export class ConvexHullTrackGenerator{
    constructor(bbox, seed, size,dataSet = [], selected = []) {
        this.bbox = bbox;
        this.size = size;
        this.randomGen = prng_alea(seed);
        this.dataSet = dataSet.length > 0 ? dataSet : this.generatePoints()
        this.dataSetHull = selected.length > 0 ? this.useSelected(selected) : this.computeConvexHull()
        this.trackEdges = this.generateTrack();
    }

    generateTrack() {
        for (let i = 0; i < 3; i++) {
            this.dataSetHull = this.expandAndDisplaceDataSet(this.dataSetHull);
            this.dataSetHull = fixAngles(this.dataSetHull);
            this.dataSetHull = pushApart(this.dataSetHull);
        }
        return generateCatmullRomSpline(this.dataSetHull, 10, 0);
    }

    generatePoints() {
        const dataSet = [];
        for (let i = 0; i < this.size; i++) {
            dataSet.push({
                x: this.randomGen() * (this.bbox.xr - this.bbox.xl) / 2 + this.bbox.xr / 4,
                y: this.randomGen() * (this.bbox.yb - this.bbox.yt) / 2 + this.bbox.yb / 4
            });
        }
        return dataSet;
    }

    //this function not only sets selected as selected points to make the convexHull
    //but clean out from original datasets the convexHull points and add the new selected ones
    //this procedure is useful when mutation over external points is done from external genetic mutation operator
    useSelected(selected) {
        const convexHullFromDataset = this.computeConvexHull();
        
        // Remove points in convexHullFromDataset from this.dataSet
        this.dataSet = this.dataSet.filter(point => 
            !convexHullFromDataset.some(hullPoint => 
                hullPoint.x === point.x && hullPoint.y === point.y
            )
        );
        
        // Add selected points to this.dataSet
        this.dataSet.push(...selected);
        
        // Return selected points
        return selected;
    }

    computeConvexHull() {
        if (this.dataSet.length < 3) return;
        this.dataSet.sort((a, b) => a.x - b.x || a.y - b.y);
        let lower = this.convexHullHalf(this.dataSet);
        let upper = this.convexHullHalf(this.dataSet.slice().reverse());
        upper.pop();
        lower.pop();
        return lower.concat(upper);
    }

    convexHullHalf(points) {
        let stack = [];
        for (let p of points) {
            while (stack.length >= 2 && this.ccw(stack[stack.length - 2], stack[stack.length - 1], p) <= 0) {
                stack.pop();
            }
            stack.push(p);
        }
        return stack;
    }

    ccw(a, b, c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }

    expandAndDisplaceDataSet(d) {
        let rSet = new Array(d.length * 2);
        let disp = { x: 0, y: 1 };
        let maxDisp = 3.0;

        for (let i = 0; i < d.length; ++i) {
            let dispLen = this.randomGen() * maxDisp;
            let angle = this.randomGen() * 2 * Math.PI;
            disp.x = Math.cos(angle) * dispLen;
            disp.y = Math.sin(angle) * dispLen;

            rSet[i * 2] = { ...d[i] };

            let nextIndex = (i + 1) % d.length;
            let midPoint = { x: (d[i].x + d[nextIndex].x) / 2, y: (d[i].y + d[nextIndex].y) / 2 };
            midPoint.x += disp.x;
            midPoint.y += disp.y;
            rSet[i * 2 + 1] = midPoint;
        }

        return rSet;
    }
}

export default ConvexHullTrackGenerator;
