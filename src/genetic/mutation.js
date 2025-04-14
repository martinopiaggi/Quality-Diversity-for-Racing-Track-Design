export function mutationVoronoi(individual, intensity) {
  const selectedCells = individual.selectedCells.map(cell => ({...cell.site}));
  const dataSet = [...individual.dataSet];
  const randomIndex = Math.floor(Math.random() * selectedCells.length);
  const deltaX = intensity * Math.random();
  const deltaY = intensity * Math.random();
  const dataSetIndex = dataSet.findIndex(point => 
      point.x === selectedCells[randomIndex].x && 
      point.y === selectedCells[randomIndex].y
  );
  selectedCells[randomIndex].x += deltaX;
  selectedCells[randomIndex].y += deltaY;
  
  if (dataSetIndex !== -1) {
  
    // Remove the point from the dataset
    dataSet.splice(dataSetIndex, 1);
  }
  // Add the mutated point to the dataset
  dataSet.push(selectedCells[randomIndex]);
  return { ds: dataSet, sel: selectedCells };
}

//let's move randomly a point in the convexHull
export function mutationConvexHull(individual, intensity) {
  const dataSetHull = [...individual.dataSetHull];
  const dataSet = [...individual.dataSet];
  
  const randomIndex = Math.floor(Math.random() * dataSetHull.length);
  const originalPoint = {...dataSetHull[randomIndex]};
  
  dataSetHull[randomIndex].x += intensity * Math.random();
  dataSetHull[randomIndex].y += intensity * Math.random();
  
  const dataSetIndex = dataSet.findIndex(point => 
    point.x === originalPoint.x && point.y === originalPoint.y
  );
  
  if (dataSetIndex !== -1) {
    dataSet[dataSetIndex] = dataSetHull[randomIndex];
  }
  
  return { ds: dataSet};
}
