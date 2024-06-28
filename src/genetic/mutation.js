export function mutation(individual,intensity) {
  const selectedCells = individual.selectedCells.map(cell => cell.site);
  const randomIndex = Math.floor(Math.random() * selectedCells.length);
  selectedCells[randomIndex].x += intensity*Math.random();
  selectedCells[randomIndex].y += intensity*Math.random();
  return selectedCells;
}

export function mutationConvexHull(individual,intensity) {
  const randomIndex = Math.floor(Math.random() * individual.dataSetHull.length);
  individual.dataSetHull[randomIndex].x += intensity*Math.random();
  individual.dataSetHull[randomIndex].y += intensity*Math.random();
  return individual.dataSetHull;
}