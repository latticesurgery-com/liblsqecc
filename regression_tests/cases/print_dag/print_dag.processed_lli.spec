digraph DirectedGraph {
  0 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:X,4:X</b></td></tr><tr><td><font color="darkgray">node: 0</font></td></tr></table>>];
  1 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 0</b></td></tr><tr><td><font color="darkgray">node: 1</font></td></tr></table>>];
  2 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 4</b></td></tr><tr><td><font color="darkgray">node: 2</font></td></tr></table>>];
  3 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:Z,4:Z</b></td></tr><tr><td><font color="darkgray">node: 3</font></td></tr></table>>];
  0 -> 1;
  0 -> 2;
  0 -> 3;
  1 -> 3;
  2 -> 3;
}
