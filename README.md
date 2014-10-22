Surface Ratio

Usage:
  
  surfaceratio input-pial-surface output-surfaceratio-vector
  
  or
  
  surfaceratio input-pial-surface output-surfaceratio-vector radius
  
  In the first case a sphere radius of 20mm is used.
  On input, FreeSurfer and BrainVisa mesh formats are accepted.
  On output, the surface ratio vector can be written as a 'sratio' file, which
  is encoded in FreeSurfer 'curv' format, or in 'sratiofloat' format, easier to read from
  C code, in which the first line indicates the number of values (ascii),
  the second line indicates if the encoding is Intel or Motorola (ascii), and the remaning
  data is a block of float values.
  The different possibilities are recognised from the file extensions (.orig, .pial, .white,
  .mesh, .sratio and .sratiofloat).
