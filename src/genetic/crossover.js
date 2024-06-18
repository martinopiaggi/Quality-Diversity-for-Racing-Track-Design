export function crossover(parent1, parent2) {
  // Extract dataset from each parent
  const dataSet1 = parent1.dataSet;
  const dataSet2 = parent2.dataSet;

  // Extract the vertex information from the selected cells
  const selectedVertices1 = getSelectedVertices(parent1.selectedCells);
  const selectedVertices2 = getSelectedVertices(parent2.selectedCells);

  // Perform ordinary least squares regression to find the best separation line
  const { slope, intercept } = ordinaryLeastSquares(selectedVertices1, selectedVertices2);

  // Determine which half of the dataset to use for each parent based on the separation line
  const halfDataSet1 = dataSet1.filter(data => data.y <= slope * data.x + intercept);
  const halfDataSet2 = dataSet2.filter(data => data.y > slope * data.x + intercept);

  // Combine the datasets
  const combinedDataSet = [...halfDataSet1, ...halfDataSet2];

  // Combine the selected cells from both parents
  let combinedSelectedCells = [...parent1.selectedCells, ...parent2.selectedCells];

  // Extract the site information from the selected cells
  combinedSelectedCells = combinedSelectedCells.map(cell => cell.site);

  const selected1 = combinedSelectedCells.filter(data => data.y <= slope * data.x + intercept);
  const selected2 = combinedSelectedCells.filter(data => data.y > slope * data.x + intercept);
  
  combinedSelectedCells = [...selected1, ...selected2];
 

  return { ds: combinedDataSet, sel: combinedSelectedCells, lineParameters: { slope, intercept } };
}

function getSelectedVertices(selectedCells) {
  const vertices = [];
  selectedCells.forEach(cell => {
    cell.halfedges.forEach(halfedge => {
      const va = halfedge.edge.va ;
      const vb  = halfedge.edge.vb ;
      vertices.push(va);
      vertices.push(vb);
    });
  });
  return vertices;
}

function ordinaryLeastSquares(vertices1, vertices2) {
  // Combine the vertices
  const combinedVertices = [...vertices1, ...vertices2];

  // Calculate the means of x and y coordinates
  const meanX = combinedVertices.reduce((acc, vertex) => acc + vertex.x, 0) / combinedVertices.length;
  const meanY = combinedVertices.reduce((acc, vertex) => acc + vertex.y, 0) / combinedVertices.length;

  // Calculate the variance and covariance
  let varX = 0;
  let varY = 0;
  let covXY = 0;
  for (const vertex of combinedVertices) {
    const dx = vertex.x - meanX;
    const dy = vertex.y - meanY;
    varX += dx * dx;
    varY += dy * dy;
    covXY += dx * dy;
  }
  varX /= combinedVertices.length;
  varY /= combinedVertices.length;
  covXY /= combinedVertices.length;

  // Calculate the slope and intercept of the OLS regression line
  const slope = covXY / varX;
  const intercept = meanY - slope * meanX;

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
  console.log(farPoint1)
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
  console.log(initPoint)
  console.log(points)
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


  console.log(combinedDataSet, selectedCellSites);
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
