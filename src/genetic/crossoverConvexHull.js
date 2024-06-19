export function crossover(parent1, parent2) {
    // Extract dataset from each parent
    const dataSet1 = parent1.dataSet;
    const dataSet2 = parent2.dataSet;
  
    // Perform ordinary least squares regression to find the best separation line
    const { slope, intercept } = ordinaryLeastSquares(dataSet1, dataSet2);
  
    // Determine which half of the dataset to use for each parent based on the separation line
    const halfDataSet1 = dataSet1.filter(data => data.y <= slope * data.x + intercept);
    const halfDataSet2 = dataSet2.filter(data => data.y > slope * data.x + intercept);
  
    // Combine the datasets
    const combinedDataSet = [...halfDataSet1, ...halfDataSet2];
  
    return { ds: combinedDataSet, lineParameters: { slope, intercept } };
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