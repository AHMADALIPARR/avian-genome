/ AVIAN-MORPH genomic geometry (q)
/ Coordinate maps, affine transform, adjacency, angle derivation.
/ Same 3x3 + translation contract as src/c11/avian_geom and cbqn/geom_transform.bqn.

/ Default matrix (row-major) and translation from the geom fixture
M: 1 0.5 0 0 2 -0.25 0 0 1f
t: 1 -2 0.5f

/ Four genomic points (x y z)
P: (1 2 4f; -1 0 2f; 0 0 0f; 2 -4 8f)

/ Affine transform: out[r] = M[3*r+0]*x + M[3*r+1]*y + M[3*r+2]*z + t[r]
transform:{[M;t;xyz]
  x:xyz 0; y:xyz 1; z:xyz 2;
  {[r] (M[3*r]*x) + (M[3*r+1]*y) + (M[3*r+2]*z) + t[r]} each til 3
 }

transformed: transform[M;t] each P

/ Adjacency for the two-neuron circuit: A (points 0,1) -> B (points 2,3)
adjacency: (0 1; 2 3)

/ Derive (theta;phi) from a pair of transformed points
angles:{[ij]
  pa: transformed ij 0; pb: transformed ij 1;
  mx: 0.5 * pa[0] + pb[0];
  my: 0.5 * pa[1] + pb[1];
  radial: sqrt (mx*mx) + my*my;
  theta: 0f | (radial * 0.1f) & 3.141592653589793f;
  phi: atan2[my; mx];
  (theta; phi)
 }

paramsA: angles adjacency 0
paramsB: angles adjacency 1

/ Twin parameters used by the orchestrator (tectal at rest for CNOT copy)
twin: `phiA`phiB`thetaB`genomeThetaA`genomePhiA`genomeThetaB`genomePhiB !
  (0f; 0f; 0f; paramsA 0; paramsA 1; paramsB 0; paramsB 1)
