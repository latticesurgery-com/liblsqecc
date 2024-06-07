digraph DirectedGraph {
  0 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 0</b></td></tr><tr><td><font color="darkgray">node: 0</font></td></tr></table>>];
  1 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 4</b></td></tr><tr><td><font color="darkgray">node: 1</font></td></tr></table>>];
  2 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:X,4:X</b></td></tr><tr><td><font color="darkgray">node: 2</font></td></tr></table>>];
  3 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 0</b></td></tr><tr><td><font color="darkgray">node: 3</font></td></tr></table>>];
  4 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 4</b></td></tr><tr><td><font color="darkgray">node: 4</font></td></tr></table>>];
  5 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:Z,4:Z</b></td></tr><tr><td><font color="darkgray">node: 5</font></td></tr></table>>];
  0 -> 2;
  0 -> 3;
  0 -> 5;
  1 -> 2;
  1 -> 4;
  1 -> 5;
  2 -> 3;
  2 -> 4;
  2 -> 5;
  3 -> 5;
  4 -> 5;
}
