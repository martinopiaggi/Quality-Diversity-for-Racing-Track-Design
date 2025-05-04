export function crossover(parent1, parent2) {
  const ds1 = parent1.dataSet;
  const ds2 = parent2.dataSet;
  const points_count = parent1.dataSet.length;

  const { slope, intercept } = randomSlopeThroughCenter(ds1, ds2);

  const half1 = ds1.filter(p => p.y <= slope * p.x + intercept);
  const half2 = ds2.filter(p => p.y >  slope * p.x + intercept);

  // Merge
  let ds = [...half1, ...half2];

  // ---- enforce fixed length ----
  if (ds.length > points_count) {
    // Fisher‑Yates shuffle then slice — random but reproducible if rng seeded
    for (let i = ds.length - 1; i > 0; --i) {
      const j = Math.floor(Math.random() * (i + 1));
      [ds[i], ds[j]] = [ds[j], ds[i]];
    }
    ds = ds.slice(0, points_count);
  } else if (ds.length < points_count) {
    // sample with replacement until length matches
    while (ds.length < points_count) {
      ds.push(ds[Math.floor(Math.random() * ds.length)]);
    }
  }

  return { ds, lineParameters: { slope, intercept } };
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