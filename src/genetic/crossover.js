export function crossover(parent1, parent2) {
  // Extract dataset from each parent
  const dataSet1 = parent1.dataSet;
  const dataSet2 = parent2.dataSet;

  // Calculate the geometric centers for each parent's selected cells
  const center1 = calculateGeometricCenter(parent1.selectedCells);
  const center2 = calculateGeometricCenter(parent2.selectedCells);

  // Calculate the slope and y-intercept of the separation line
  const slope = (center2.y - center1.y) / (center2.x - center1.x);
  const yIntercept = center1.y - slope * center1.x;

  // Determine which half of the dataset to use for each parent based on the separation line
  const halfDataSet1 = dataSet1.filter(data => data.y <= slope * data.x + yIntercept);
  const halfDataSet2 = dataSet2.filter(data => data.y > slope * data.x + yIntercept);

  // Combine the selected halves
  const combinedDataSet = [...halfDataSet1, ...halfDataSet2];

  // Combine the selected cells from both parents
  let combinedSelectedCells = [...parent1.selectedCells, ...parent2.selectedCells];

  // Extract the site information from the selected cells
  combinedSelectedCells = combinedSelectedCells.map(cell => cell.site);
  
  // Ensure that the selected cells are present in the combined dataset
  combinedSelectedCells.forEach(site => {
    const { x, y } = site;
    if (!combinedDataSet.some(data => data.x === x && data.y === y)) {
      combinedDataSet.push({ x, y});
    }
  });
  
  return { ds: combinedDataSet, sel: combinedSelectedCells };
}

function calculateGeometricCenter(selectedCells) {
  const sumX = selectedCells.reduce((acc, cell) => acc + cell.site.x, 0);
  const sumY = selectedCells.reduce((acc, cell) => acc + cell.site.y, 0);
  const count = selectedCells.length;
  return { x: sumX / count, y: sumY / count };
}