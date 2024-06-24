import {BBOX } from "../utils/constants.js"

export function crossover(parent1, parent2) {
  // Extract dataset from each parent
  let dataSet1 = parent1.dataSet;
  let dataSet2 = parent2.dataSet;
  let halfDataSet1  = []
  let halfDataSet2  = []


  let parent1selected = parent1.selectedCells.map(cell => cell.site)
  let parent2selected = parent2.selectedCells.map(cell => cell.site)
  
  const { slope, intercept } = randomSlopeThroughCenter(parent1selected, parent2selected);

  const criteria = (data) => data.y <= slope * data.x + intercept;

  // If the line is not steep, separate based on the y-coordinate
  let selected1 = parent1selected.filter(data => criteria(data));
  let selected2 = parent2selected.filter(data => !criteria(data));

  //in case the separation line gives us an half set without selected cells 
  //we swap the conditions 
  if((selected1.length || selected2.length) == 0 ){
    selected1 = parent1selected.filter(data => !criteria(data));
    selected2 = parent2selected.filter(data => criteria(data));

    halfDataSet1 = dataSet1.filter(data => !criteria(data));
    halfDataSet2 = dataSet2.filter(data => criteria(data));
  }
  else{
    halfDataSet1 = dataSet1.filter(data => criteria(data));
    halfDataSet2 = dataSet2.filter(data => !criteria(data));
  }

  // Combine the corresponding halves 
  let combinedSelectedCells = [...selected1, ...selected2];
  const combinedDataSet = [...halfDataSet1, ...halfDataSet2];


  //PATCH / HEURISTIC to select cells to fix cases where the cells are not linked
  const selectableCell = checkForSelectableCellBetweenClosestPair(selected1,selected2,combinedDataSet);
  if(selectableCell){
    combinedSelectedCells.push(...selectableCell)
  }

  //in case to reduce number of cells:
  //const lengthOrig = Math.ceil((parent1selected.length + parent2selected.length)/2);
  //while(combinedSelectedCells.length>lengthOrig)combinedSelectedCells.pop();
  
  return { ds: combinedDataSet, sel: combinedSelectedCells, lineParameters: { slope, intercept } };
}

export function checkForSelectableCellBetweenClosestPair(set1, set2, allCells) {
  let closestPair = findClosestPair(set1, set2);
  console.log(set1)
  console.log(set2)
  if (!closestPair) return false;

  return selectableCellBetween(closestPair.cell1, closestPair.cell2, allCells);
}

function findClosestPair(set1, set2) {
  let minDistance = Infinity;
  let closestPair = null;

  for (const cell1 of set1) {
    for (const cell2 of set2) {
        const distance = calculateDistance(cell1, cell2);
        if (distance < minDistance) {
          minDistance = distance;
          closestPair = { cell1, cell2 };
        }
    }
  }
  console.log(closestPair)
  return closestPair;
}

function selectableCellBetween(cell1, cell2, allCells) {
  // Define the area between the two points
  const halo = 10;
  const minX = Math.min(cell1.x, cell2.x) - halo;
  const maxX = Math.max(cell1.x, cell2.x) + halo;
  const minY = Math.min(cell1.y, cell2.y) - halo;
  const maxY = Math.max(cell1.y, cell2.y)+ halo;

  console.log(minX,maxX,minY,maxY)
  console.log(allCells)
  // Filter cells within the area
  const cellsBetween = allCells.filter(cell => 
    cell.x > minX && cell.x < maxX &&
    cell.y > minY && cell.y < maxY &&
    cell !== cell1 && cell !== cell2
  );
  console.log(cellsBetween)
  return cellsBetween;
}


function randomSlopeThroughCenter(vertices1, vertices2) {
  // Combine the vertices
  const combinedVertices = [...vertices1, ...vertices2];

  // Calculate the center coordinates
  const centerX = combinedVertices.reduce((acc, vertex) => acc + vertex.x, 0) / combinedVertices.length;
  const centerY = combinedVertices.reduce((acc, vertex) => acc + vertex.y, 0) / combinedVertices.length;

    // Generate a random angle in radians between -π/2 and π/2
    const angle = Math.random() * Math.PI - Math.PI / 2;

    // Calculate the slope using the tangent of the angle
    const slope = Math.tan(angle);

  // Calculate the intercept based on the center coordinates and slope
  const intercept = centerY - slope * centerX;

  return { slope, intercept };
}



// ---

export function crossover2(parent1, parent2) {
  const selectedCellSites = [];
  const distanceThreshold = BBOX.xr*0.02;
  let combinedDataSet = [];
  const parentSelected1 = [...parent1.selectedCells];
  const parentSelected2 = [...parent2.selectedCells];
  

  const centerPoint1 = parentSelected1.shift();
  const centerPoint2 = parentSelected2.shift();

  //parentSelected1.pop();  to regularize and reduce ? 
  //parentSelected2.pop();  to regularize and reduce ? 

  let remappedUniquePoints2 =  getUniquePointsNearSites(parentSelected2).flat().filter(point => point != null);
  
  remappedUniquePoints2 = remapPoints(centerPoint2.site,remappedUniquePoints2,centerPoint1.site);
  
  combinedDataSet.push(...remappedUniquePoints2)
  
  combinedDataSet.push(...getUniquePointsNearSites(parentSelected1).flat().filter(point => point != null))

  //remember that this function reads centerPoint2 to remappedSelectedParent2 !!
  let remappedSelectedParent2 = remapPoints(centerPoint2.site,parentSelected2.map(cell => cell.site),centerPoint1.site);
  
  remappedSelectedParent2 = remappedSelectedParent2.filter(point => {
    for (const site of parentSelected1.map(cell => cell.site)) {
      const distance = calculateDistance(point, site);
      if (distance <= distanceThreshold) {
        return false;
      }
    }
    return true;
    });

  
  //push to selectedCellSites
  selectedCellSites.push(...parentSelected1.map(cell => cell.site),...remappedSelectedParent2)

  //remove points in the combinedDataset that are too close to the selectedCellSites 
  combinedDataSet = combinedDataSet.filter(point => {
  for (const site of selectedCellSites) {
    const distance = calculateDistance(point, site);
    if (distance <= distanceThreshold) {
      return false;
    }
  }
  return true;
  });

  const centerOfCanvas = {x: BBOX.xr/2, y: BBOX.yb/2};
  const borderPoints1 = sortByDistance(parent1.dataSet,centerOfCanvas).filter(point => point != null);
  const borderPoints2 = sortByDistance(parent2.dataSet,centerOfCanvas).filter(point => point != null);
  let i = Math.min(borderPoints1.length, borderPoints2.length) - 1;
  while (combinedDataSet.length < 50 && i >= 0) {
    if (i < borderPoints1.length) {
      combinedDataSet.push(borderPoints1[i]);
    }
    if (i < borderPoints2.length && combinedDataSet.length < 50 ) {
      combinedDataSet.push(borderPoints2[i]);
    }
    i--;
  }


  return { ds: combinedDataSet, sel: selectedCellSites };
}


function remapPoints(initPoint, points,  remapPoint){
  let relativeDistances = points.map(point => ({
    x: point.x - initPoint.x,
    y: point.y - initPoint.y
  }));

  
  relativeDistances = relativeDistances.filter(cell => (cell.x != 0) && (cell.y != 0) );
  relativeDistances.push({x:0,y:0});
  const remappedPoints = relativeDistances.map(distance => ({
    x: remapPoint.x + distance.x,
    y: remapPoint.y + distance.y
  }));

  return remappedPoints;

}

function getUniquePointsNearSites(objects) {
  const uniquePointsPerSite = objects.map(obj => {
    const uniquePoints = new Set();
    obj.halfedges.forEach(halfedge => {
      const { lSite, rSite } = halfedge.edge;
      uniquePoints.add(JSON.stringify(lSite));
      uniquePoints.add(JSON.stringify(rSite));
    });

    return Array.from(uniquePoints).map(JSON.parse);
  });

  return uniquePointsPerSite;
}

function sortByDistance(dataSet, center) {
  return dataSet.sort((a, b) => {
    const distA = calculateDistance(a, center);
    const distB = calculateDistance(b, center);
    return distA - distB;
  });
}

function calculateDistance(point, center) {
  const dx = point.x - center.x;
  const dy = point.y - center.y;
  return Math.sqrt(dx * dx + dy * dy);
}


// --- crossover 3 "middle point"

export function crossover3(parent1, parent2) {
  const parentSelected1 = [...parent1.selectedCells];
  const parentSelected2 = [...parent2.selectedCells];
  const selected = [];

  for (let i = 0; i < Math.ceil(parentSelected1.length/2); i++) {
    const point1 = parentSelected1[i].site;
    const point2 = findNearestPoint(point1, parent2.dataSet);
    const middlePoint = getMiddlePoint(point1, point2);
    if (!isPointAlreadyInSet(middlePoint, selected)) {
      selected.push(middlePoint);
    }
  }

  for (let i = 0; i < Math.ceil(parentSelected2.length/2); i++) {
    const point1 = parentSelected2[i].site;
    const point2 = findNearestPoint(point1, parent1.dataSet);
    const middlePoint = getMiddlePoint(point1, point2);
    if (!isPointAlreadyInSet(middlePoint, selected)) {
      selected.push(middlePoint);
    }
  }
  
  // For each cell in parentSelected1, find the nearest point in parent2.dataSet
  // and add the middle point to mergedSelectedSites  
  const relevantPoints1 = getUniquePointsNearSites(parentSelected1).flat().filter(point => point != null);
  const relevantPoints2 = getUniquePointsNearSites(parentSelected2).flat().filter(point => point != null);
  let combinedDataSet = []

  for (let i = 0; i < Math.ceil(relevantPoints1.length/2); i++) {
    const point1 = relevantPoints1[i];
    const point2 = findNearestPoint(point1, parent2.dataSet);
    const middlePoint = getMiddlePoint(point1, point2);
    if (!isPointAlreadyInSet(middlePoint, combinedDataSet)) {
      combinedDataSet.push(middlePoint);
    }
  }
  
  for (let i = 0; i < Math.ceil(relevantPoints2.length/2); i++) {
    const point1 = relevantPoints2[i];
    const point2 = findNearestPoint(point1, parent1.dataSet);
    const middlePoint = getMiddlePoint(point1, point2);
    if (!isPointAlreadyInSet(middlePoint, combinedDataSet)) {
      combinedDataSet.push(middlePoint);
    }
  }


  const centerOfCanvas = {x: BBOX.xr/2, y: BBOX.yb/2};
  const borderPoints1 = sortByDistance(parent1.dataSet,centerOfCanvas).filter(point => point != null);
  const borderPoints2 = sortByDistance(parent2.dataSet,centerOfCanvas).filter(point => point != null);
  let i = Math.min(borderPoints1.length, borderPoints2.length) - 1;
  while (combinedDataSet.length < 50 && i >= 0) {
    if (i < borderPoints1.length) {
      combinedDataSet.push(borderPoints1[i]);
    }
    if (i < borderPoints2.length && combinedDataSet.length < 50 ) {
      combinedDataSet.push(borderPoints2[i]);
    }
    i--;
  }

  return { ds: combinedDataSet, sel: selected };
}

// Helper function to find the nearest point in an array of points
function findNearestPoint(point, points) {
  let nearestPoint = null;
  let minDistance = Infinity;
  
  for (let i = 0; i < points.length; i++) {
    const currentPoint = points[i].site || points[i];
    const distance = calculateDistance(point, currentPoint);
    if (distance < minDistance) {
      minDistance = distance;
      nearestPoint = currentPoint;
    }
  }
  
  return nearestPoint;
}

function isPointAlreadyInSet(point, set) {
  return set.some(existingPoint => 
    Math.abs(existingPoint.x - point.x) < 1e-6 && 
    Math.abs(existingPoint.y - point.y) < 1e-6
  );
}


// Helper function to calculate the middle point between two points
function getMiddlePoint(point1, point2) {
  const x = (point1.x + point2.x) / 2;
  const y = (point1.y + point2.y) / 2;
  return { x, y };
}
