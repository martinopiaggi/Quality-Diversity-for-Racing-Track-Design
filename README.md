# Quality Diversity for Racing Track Design

MSc thesis project to evolve diverse, high‑quality racing tracks with MAP‑Elites, run headless sims, and analyze with a data‑driven stack. Take inspiration from the approach; it’s a thesis prototype, not a product. The full methodology, engineering trade‑offs, and TORCS quirks are in the thesis.

- [Blog article](https://blog.martino.im/Quality-Diversity-for-Racing-Tracks-Design)
- [Executive Summary & Full Thesis](https://www.politesi.polimi.it/handle/10589/239983)
- [Live visualizer](https://pcgtrack.netlify.app/)

A fork with a continuation of this project can be found at: [Novelty Search for Racing Track Design](https://github.com/zibasPk/Novelty-Search-for-Racing-Track-Design)

## What it is

- MAP‑Elites search over a learned behavior space (not hand‑tuned features)
- Voronoi and Convex Hull generators (genotype → spline → TORCS XML)
- Headless, containerized simulations with custom telemetry
- UMAP‑guided, interpretable diversity; web visualizer for real‑time browse/debug
- Final output: **a track layout represented by a closed spline**, exported to TORCS‑compatible XML
- Procedural gen is very interesting. QD makes it diverse. Voronoi makes it weird in the best way.

## How it works (super concise)

- Genotype → Phenotype: 2D seed points → Voronoi/Convex Hull → closed spline → TORCS track XML
- Behavior space: sample splines → UMAP to 2D descriptors (similar shapes stay close)
- MAP‑Elites: populate a grid over that space; mutate/crossover; replace cell champions if outperformed
- Evaluate: headless races; telemetry (speed, curvature, overtakes, incidents, closure error)
- Analyze: aggregate, cluster, export to visualizer

## Borrowable bits

- Voronoi genotype + crossovers (Random‑Line Partitioning, Relative Reconstruction)
- Normalized overtakes (robust to geometry artifacts)
- UMAP behavior space for QD (no “#turns” features)
- Per‑sim container pattern for isolation and easy parallelism

## Caveats

- TORCS is dated (no elevation/banking).
- Per‑sim containers add overhead by design.
- Metrics need normalization and validity checks.

## Roadmap (ideas to steal)

- Modern engine (Unity/Unreal) for elevation, banking, richer geometry
- Surrogate models to reduce sim cost
- Designer‑in‑the‑loop for “fun”/aesthetics
- Advanced QD (e.g., DCG‑MAP‑Elites), hybrid gradient methods
- Stronger geometry validity (self‑intersection, curvature/grade limits)

## License

MIT License.
