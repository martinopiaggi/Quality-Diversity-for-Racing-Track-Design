export function mutation(solution,intensity) {
  // Extract dataset from each parent
  const selectedCells = solution.selectedCells.map(cell => cell.site);
  // Take a random point in solution.selectedCells
  const randomIndex = Math.floor(Math.random() * selectedCells.length);
  selectedCells[randomIndex].x += intensity*Math.random();
  selectedCells[randomIndex].y += intensity*Math.random();
  return selectedCells;
}