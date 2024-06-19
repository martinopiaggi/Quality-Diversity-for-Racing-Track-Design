export function crossover(parent1, parent2) {
    // Extract dataset from each parent
    const dataSet1 = parent1.dataSet;
    const dataSet2 = parent2.dataSet;
  
    // Perform ordinary least squares regression to find the best separation line
    const { slope, intercept } = randomSlopeThroughCenter(dataSet1, dataSet2);
  
    // Determine which half of the dataset to use for each parent based on the separation line
    const halfDataSet1 = dataSet1.filter(data => data.y <= slope * data.x + intercept);
    const halfDataSet2 = dataSet2.filter(data => data.y > slope * data.x + intercept);
  
    // Combine the datasets
    const combinedDataSet = [...halfDataSet1, ...halfDataSet2];
  
    return { ds: combinedDataSet, lineParameters: { slope, intercept } };
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