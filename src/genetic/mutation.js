export function mutation(individual, intensity) {
  const selectedCells = individual.selectedCells.map(cell => ({...cell.site}));
  const dataSet = [...individual.dataSet];
  
  const randomIndex = Math.floor(Math.random() * selectedCells.length);
  const deltaX = intensity * Math.random();
  const deltaY = intensity * Math.random();
  
  selectedCells[randomIndex].x += deltaX;
  selectedCells[randomIndex].y += deltaY;

  const dataSetIndex = dataSet.findIndex(point => 
    point.x === selectedCells[randomIndex].x - deltaX && 
    point.y === selectedCells[randomIndex].y - deltaY
  );
  
  if (dataSetIndex !== -1) {
    dataSet[dataSetIndex] = {...selectedCells[randomIndex]};
  }
  
  return { sel: selectedCells, ds: dataSet };
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