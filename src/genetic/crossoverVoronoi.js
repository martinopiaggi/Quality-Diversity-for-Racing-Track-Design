import {BBOX, NUMBER_OF_VORONOI_SITES } from "../utils/constants.js"

export function crossover(parent1, parent2, regularize = false) {
  // Extract dataset from each parent
  let dataSet1 = parent1.dataSet;
  let dataSet2 = parent2.dataSet;
  let halfDataSet1  = []
  let halfDataSet2  = []

  let parent1selected = parent1.selectedCells.map(cell => cell.site)
  let parent2selected = parent2.selectedCells.map(cell => cell.site)
  
  const center = computeGeometricCenter([...parent1selected, ...parent2selected]);

  const { slope, intercept } = randomSlopeThroughCenter(center);

  const criteria = (data) => data.y <= slope * data.x + intercept;

  // Calculate selections for both possible divisions
  const division1 = {
    selected1: parent1selected.filter(data => criteria(data)),
    selected2: parent2selected.filter(data => !criteria(data)),
  };

  const division2 = {
    selected1: parent1selected.filter(data => !criteria(data)),
    selected2: parent2selected.filter(data => criteria(data)),
  };

  // Choose the division that maximizes the minimum number of selected points
  const minDivision1 = Math.min(division1.selected1.length, division1.selected2.length);
  const minDivision2 = Math.min(division2.selected1.length, division2.selected2.length);

  let selected1, selected2;

  if (minDivision2 > minDivision1) {
    // Use division2
    selected1 = division2.selected1;
    selected2 = division2.selected2;
    halfDataSet1 = dataSet1.filter(data => !criteria(data));
    halfDataSet2 = dataSet2.filter(data => criteria(data));
  } else {
    // Use division1 (this includes the case where they're equal)
    selected1 = division1.selected1;
    selected2 = division1.selected2;
    halfDataSet1 = dataSet1.filter(data => criteria(data));
    halfDataSet2 = dataSet2.filter(data => !criteria(data));
  }


  if(regularize){
    //in case to reduce number of cells:
    const lengthOrig = Math.floor((parent1selected.length + parent2selected.length)/2);
    while ((selected1.length + selected2.length) > lengthOrig) {
      if (selected1.length > selected2.length) {
        selected1.pop();
      } else {
        selected2.pop();
      }
    }
  }
  
    // Combine the corresponding halves 
  let combinedSelectedCells = [...selected1, ...selected2];

  // Combine the corresponding halves of the dataset
  //keeping them with length NUMBER_OF_VORONOI_SITES
  let combinedDataSet = [];
  let i = NUMBER_OF_VORONOI_SITES - 1; // Start from the end of the arrays
  while (combinedDataSet.length < NUMBER_OF_VORONOI_SITES && i >= 0) {
    if (i < halfDataSet1.length && combinedDataSet.length < NUMBER_OF_VORONOI_SITES) {
      if(!isPointAlreadyInSet(halfDataSet1[i],combinedDataSet)){
        combinedDataSet.push(halfDataSet1[i]);
      }
    }
    if (i < halfDataSet2.length && combinedDataSet.length < NUMBER_OF_VORONOI_SITES) {
      if(!isPointAlreadyInSet(halfDataSet2[i],combinedDataSet)){
        combinedDataSet.push(halfDataSet2[i]);
      }
    }
    i--;
  }
 
  return { ds: combinedDataSet, sel: combinedSelectedCells, lineParameters: { slope, intercept } };
}

function randomSlopeThroughCenter(center) {
    // Generate a random angle in radians between -π/2 and π/2
    const angle = Math.random() * Math.PI - Math.PI / 2;
    const slope = Math.tan(angle);
    const intercept = center.y - slope * center.x;
    return { slope, intercept };
}


function computeGeometricCenter(vertices) {
  const x = vertices.reduce((acc, vertex) => acc + vertex.x, 0) / vertices.length;
  const y = vertices.reduce((acc, vertex) => acc + vertex.y, 0) / vertices.length;
  return { x, y };
}



// ---

export function crossover2(parent1, parent2,regularize=false) {
  const selectedCellSites = [];
  const distanceThreshold = BBOX.xr*0.02;
  let combinedDataSet = [];
  const parentSelected1 = [...parent1.selectedCells];
  const parentSelected2 = [...parent2.selectedCells];
  

  const centerPoint1 = parentSelected1.shift();
  const centerPoint2 = parentSelected2.shift();

  if (regularize) {
    //+ 2 because with shift() we just removed 2 cells
    const averageLength = Math.floor((parentSelected1.length + parentSelected2.length + 1) / 2);

    while ((parentSelected1.length + parentSelected2.length) > averageLength) {
      if (parentSelected1.length > parentSelected2.length) {
        parentSelected1.pop();
      } else {
        parentSelected2.pop();
      }
    }
  }

  let remappedUniquePoints2 =  getUniquePointsNearSites(parentSelected2).flat().filter(point => point != null);
  
  remappedUniquePoints2 = remapPoints(centerPoint2.site,remappedUniquePoints2,centerPoint1.site);
  
  combinedDataSet.push(...remappedUniquePoints2)
  
  combinedDataSet.push(...getUniquePointsNearSites(parentSelected1).flat().filter(point => point != null))

  //some points are repeated, use this to filter out repeated points
  const uniquePoints = [];
  const seenCoordinates = new Set();  
  combinedDataSet = combinedDataSet.filter(point => {
      const coordKey = `${point.x},${point.y}`;
      if (!seenCoordinates.has(coordKey)) {
          seenCoordinates.add(coordKey);
          uniquePoints.push(point);
          return true;
      }
      return false;
  });
  combinedDataSet = uniquePoints;

  
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

  //Remove points in the combinedDataset that are too close to the selectedCellSites 
  combinedDataSet = combinedDataSet.filter(point => {
  for (const site of selectedCellSites) {
    const distance = calculateDistance(point, site);
    if (distance <= distanceThreshold) {
      if(!isPointAlreadyInSet(point,selectedCellSites))
      return false;
    }
  }
  return true;
  });

  const centerOfCanvas = {x: BBOX.xr/2, y: BBOX.yb/2};
  const borderPoints1 = sortByDistance(parent1.dataSet,centerOfCanvas).filter(point => point != null);
  const borderPoints2 = sortByDistance(parent2.dataSet,centerOfCanvas).filter(point => point != null);
  let i = Math.floor(NUMBER_OF_VORONOI_SITES*0.7); //arbitrary value: still giving priority to most far points but not at the borders
  while (combinedDataSet.length < NUMBER_OF_VORONOI_SITES && i >= 0) {
    if (i < borderPoints1.length) {
      combinedDataSet.push(borderPoints1[i]);
    }
    if (i < borderPoints2.length && combinedDataSet.length < NUMBER_OF_VORONOI_SITES) {
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

  for (let i = 0; i < Math.floor(parentSelected2.length/2); i++) {
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
  while (combinedDataSet.length < NUMBER_OF_VORONOI_SITES && i >= 0) {
    if (i < borderPoints1.length) {
      combinedDataSet.push(borderPoints1[i]);
    }
    if (i < borderPoints2.length && combinedDataSet.length < NUMBER_OF_VORONOI_SITES) {
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
