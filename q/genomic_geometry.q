/ q / kdb+ genomic geometry. Real q code; input is a table.
/ Required columns: chr (symbol), pos (long), ref (symbol), alt (symbol), effect (float).

.avian.require:{[ok;msg] if[not ok;'msg]; :1b};
.avian.validate:{[v]
  .avian.require[all `chr`pos`ref`alt`effect in cols v;"missing variant columns"];
  .avian.require[all 0<=v`pos;"negative genomic position"];
  .avian.require[all (v`effect)=v`effect;"NaN genomic effect"];
  :v
 };

/ Base-pair positions mapped to angles on a cylinder, effect -> radius and height.
/ This is an explicit geometric policy, not an empirical genomic phenotype model.
.avian.coordinates:{[v]
  v:.avian.validate v;
  if[0=count v; :([]chr:`symbol$();pos:`long$();x:`float$();y:`float$();z:`float$())];
  p:`float$v`pos;
  u:(p-min p)%1f|max p-min p;
  a:6.283185307179586*u;
  radius:1f+abs v`effect;
  :([]chr:v`chr;pos:v`pos;x:radius*cos a;y:radius*sin a;z:v`effect)
 };

/ Adjacency matrix: strict Euclidean radius; excludes the diagonal.
.avian.adjacency:{[xyz;r]
  .avian.require[r>=0;"negative radius"];
  n:count xyz;
  if[n=0; :0 0#0b];
  idx:til n;
  pairs:idx cross idx;
  i:first each pairs;
  j:last each pairs;
  dx:(xyz`x) i-(xyz`x) j;
  dy:(xyz`y) i-(xyz`y) j;
  dz:(xyz`z) i-(xyz`z) j;
  :n n#(i<>j)&(dx*dx)+dy*dy+dz*dz<r*r
 };

/ Deterministic, bounded gate parameter policy.
.avian.parameters:{[v]
  v:.avian.validate v;
  p:0f|1f&0.5+0.5*v`effect;
  :([]chr:v`chr;pos:v`pos;gateWeight:p;rotation:6.283185307179586*p)
 };

/ Example input:
/ v:([]chr:`chr1`chr1`chr2;pos:100 200 500;ref:`A`C`G;alt:`T`G`A;effect:0.1 -0.2 0.7);
/ .avian.coordinates v
/ .avian.parameters v
