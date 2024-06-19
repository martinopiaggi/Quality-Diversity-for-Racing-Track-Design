export function mutation(solution,intensity) {
  // Extract dataset from each parent
  const selectedCells = solution.selectedCells.map(cell => cell.site);
  // Take a random point in solution.selectedCells
  const randomIndex = Math.floor(Math.random() * selectedCells.length);
  selectedCells[randomIndex].x += intensity*Math.random();
  selectedCells[randomIndex].y += intensity*Math.random();
  return selectedCells;
}


export function mutationConvexHull(dataSet,intensity) {
  // Take a random point in dataSet
  const randomIndex = Math.floor(Math.random() * dataSet.length);
  dataSet[randomIndex].x += intensity*Math.random();
  dataSet[randomIndex].y += intensity*Math.random();
  return dataSet;
}