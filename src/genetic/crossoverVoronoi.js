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
  const combinedSelectedCells = [...selected1, ...selected2];
  const combinedDataSet = [...halfDataSet1, ...halfDataSet2];

  return { ds: combinedDataSet, sel: combinedSelectedCells, lineParameters: { slope, intercept } };
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
  const combinedDataSet = [];
  const parentSelected1 = [...parent1.selectedCells];
  const parentSelected2 = [...parent2.selectedCells];
  
  const farPoint1 = parentSelected1.shift();
  const centerPoint2 = parentSelected2.shift();

  let remappedUniquePoints2 =  getUniquePointsNearSites(parentSelected2).flat().filter(point => point != null);
  
  remappedUniquePoints2 = remapPoints(centerPoint2.site,remappedUniquePoints2,farPoint1.site);
  
  combinedDataSet.push(...remappedUniquePoints2)
  
  combinedDataSet.push(...getUniquePointsNearSites(parentSelected1).flat().filter(point => point != null))

  //remember that this function readds centerPoint2 to remappedSelectedParent2 !!
  const remappedSelectedParent2 = remapPoints(centerPoint2.site,parentSelected2.map(cell => cell.site),farPoint1.site);

  selectedCellSites.push(...parentSelected1.map(cell => cell.site),...remappedSelectedParent2)

  const borderPoints1 = sortByDistance(parent1.dataSet,farPoint1.site).filter(point => point != null);
  const borderPoints2 = sortByDistance(parent2.dataSet,farPoint1.site).filter(point => point != null);
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
  const selectedCellSites = [];
  const combinedDataSet = [];
  const parentSelected1 = [...parent1.selectedCells];
  const parentSelected2 = [...parent2.selectedCells];
  
  const maxSelected = parentSelected1.length >  parentSelected2.length ? 
    parentSelected1 : parentSelected2 ;

  const minSelected = parentSelected1.length <=  parentSelected2.length ? 
  parentSelected1 : parentSelected2 ;
  
  // For each cell in parentSelected1, find the nearest point in parentSelected2
  // and add the middle point to mergedSelectedSites
  
  const mergedSelectedSites = [];
  for (let i = 0; i < maxSelected.length; i++) {
    const cell1 = maxSelected[i];
    const cell2 = findNearestPoint(cell1.site, minSelected);
    const middlePoint = getMiddlePoint(cell1.site, cell2);
    mergedSelectedSites.push(middlePoint);
  }
  
    // Combine the selected cell sites
    selectedCellSites.push(...mergedSelectedSites);

  // For each point in relevantPoints1, find the nearest point in relevantPoints2
  // and add the middle point to mergedPoints
  const mergedPoints = [];
  const relevantPoints1 = getUniquePointsNearSites(parentSelected1).flat().filter(point => point != null);
  const relevantPoints2 = getUniquePointsNearSites(parentSelected2).flat().filter(point => point != null);
  const minRelevantLength = Math.min(relevantPoints1.length, relevantPoints2.length);
  
  for (let i = 0; i < minRelevantLength; i++) {
    const point1 = relevantPoints1[i];
    const point2 = findNearestPoint(point1, relevantPoints2);
    relevantPoints2.pop(point2)
    const middlePoint = getMiddlePoint(point1, point2);
    mergedPoints.push(middlePoint);
  }
  

  // Combine the merged points and relevant points
  combinedDataSet.push(...mergedPoints,...selectedCellSites);
  

  const borderPoints1 = sortByDistance(parent1.dataSet,selectedCellSites[0]).filter(point => point != null);
  const borderPoints2 = sortByDistance(parent2.dataSet,selectedCellSites[0]).filter(point => point != null);
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

// Helper function to calculate the middle point between two points
function getMiddlePoint(point1, point2) {
  const x = (point1.x + point2.x) / 2;
  const y = (point1.y + point2.y) / 2;
  return { x, y };
}
