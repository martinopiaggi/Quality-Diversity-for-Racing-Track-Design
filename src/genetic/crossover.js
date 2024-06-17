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

  let remappedUniquePoints2 =  getUniquePointsNearSites(parentSelected2).flat();
  
  remappedUniquePoints2 = remapPoints(centerPoint2.site,remappedUniquePoints2,farPoint1.site);
  
  combinedDataSet.push(...remappedUniquePoints2)
  combinedDataSet.push(...getUniquePointsNearSites(parentSelected1).flat())

  //remember that this function readds centerPoint2 to remappedSelectedParent2 !!
  const remappedSelectedParent2 = remapPoints(centerPoint2.site,parentSelected2.map(cell => cell.site),farPoint1.site);

  selectedCellSites.push(...parentSelected1.map(cell => cell.site),...remappedSelectedParent2)
  
  const borderPoints1 = sortByDistance(parent1.dataSet,farPoint1.site);
  const borderPoints2 = sortByDistance(parent2.dataSet,farPoint1.site);
  let i = Math.min(borderPoints1.length,borderPoints2.length);
  while(combinedDataSet.length<50 && i>=0){
    if(i<borderPoints1.length)combinedDataSet.push(borderPoints1[i])
    if(i<borderPoints2.length)combinedDataSet.push(borderPoints2[i])
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

function calculateGeometricCenter(points) {
  const sumX = points.reduce((sum, point) => sum + point.x, 0);
  const sumY = points.reduce((sum, point) => sum + point.y, 0);
  const count = points.length;

  return {
    x: sumX / count,
    y: sumY / count
  };
}


