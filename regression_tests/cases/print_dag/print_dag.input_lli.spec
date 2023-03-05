digraph DirectedGraph {
  0 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>Init 100 |+&gt;</b></td></tr><tr><td><font color="darkgray">node: 0</font></td></tr></table>>];
  1 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:X,4:X</b></td></tr><tr><td><font color="darkgray">node: 1</font></td></tr></table>>];
  2 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 5:Z,6:Z</b></td></tr><tr><td><font color="darkgray">node: 2</font></td></tr></table>>];
  3 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 2:Z,3:Z</b></td></tr><tr><td><font color="darkgray">node: 3</font></td></tr></table>>];
  4 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 100</b></td></tr><tr><td><font color="darkgray">node: 4</font></td></tr></table>>];
  5 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>HGate 1</b></td></tr><tr><td><font color="darkgray">node: 5</font></td></tr></table>>];
  6 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>HGate 1</b></td></tr><tr><td><font color="darkgray">node: 6</font></td></tr></table>>];
  7 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>RotateSingleCellPatch 1</b></td></tr><tr><td><font color="darkgray">node: 7</font></td></tr></table>>];
  8 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>MultiBodyMeasure 0:Z,100:Z</b></td></tr><tr><td><font color="darkgray">node: 8</font></td></tr></table>>];
  0 -> 4;
  0 -> 8;
  1 -> 8;
  4 -> 8;
  5 -> 6;
  5 -> 7;
  6 -> 7;
}
