import { VoronoiTrackGenerator } from './voronoiTrackGenerator.js';
import * as utils from './utils/utils.js';

// Constants
const BBOX = { xl: 0, xr: 600, yt: 0, yb: 600 };
const TRACK_SIZE = 2;
//Track generation
const seed = Math.random();
const trackGenerator = new VoronoiTrackGenerator(BBOX, seed, TRACK_SIZE);
const trackEdges = trackGenerator.trackEdges;
const diagram = trackGenerator.diagram;
const splineTrack = utils.splineSmoothing(trackEdges);



function setup() {
    createCanvas(BBOX.xr, BBOX.yb);
    console.log("SEED: " + seed);
    console.log("trackSize (# cells): " + TRACK_SIZE);
}

function draw() {
  background(0);
  strokeWeight(0.5);
  drawVoronoi();
  noFill();
  stroke(0, 255, 0);
  strokeWeight(3);
  drawSpline(splineTrack);
}

// Make setup and draw available globally
window.setup = setup;
window.draw = draw;


function drawVoronoi() {
    stroke(255);
    diagram.edges.forEach(edge => line(edge.va.x, edge.va.y, edge.vb.x, edge.vb.y));
}

function drawSpline(spline) {
    beginShape();
    let startColor = color(0, 255, 255); 
    let endColor = color(255, 0, 255);
    for (let i = 0; i < spline.length; i++) {
        stroke(lerpColor(startColor,endColor,i/spline.length))
        //vertex(spline[i].x, spline[i].y); 
        point(spline[i].x, spline[i].y)  
    }
    endShape(CLOSE); 
  }