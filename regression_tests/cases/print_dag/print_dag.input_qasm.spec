digraph DirectedGraph {
  0 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>cx q[0],q[1];</b></td></tr><tr><td><font color="darkgray">node: 0</font></td></tr></table>>];
  1 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>crz(pi/16) q[0],q[2];</b></td></tr><tr><td><font color="darkgray">node: 1</font></td></tr></table>>];
  2 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>cx q[5],q[6];</b></td></tr><tr><td><font color="darkgray">node: 2</font></td></tr></table>>];
  3 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>crz(pi/16) q[6],q[7];</b></td></tr><tr><td><font color="darkgray">node: 3</font></td></tr></table>>];
  4 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[0];</b></td></tr><tr><td><font color="darkgray">node: 4</font></td></tr></table>>];
  5 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[1];</b></td></tr><tr><td><font color="darkgray">node: 5</font></td></tr></table>>];
  6 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[2];</b></td></tr><tr><td><font color="darkgray">node: 6</font></td></tr></table>>];
  7 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[5];</b></td></tr><tr><td><font color="darkgray">node: 7</font></td></tr></table>>];
  8 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[6];</b></td></tr><tr><td><font color="darkgray">node: 8</font></td></tr></table>>];
  9 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>h q[7];</b></td></tr><tr><td><font color="darkgray">node: 9</font></td></tr></table>>];
  10 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>cx q[7],q[8];</b></td></tr><tr><td><font color="darkgray">node: 10</font></td></tr></table>>];
  11 [shape="plaintext",label=<<table cellborder="0"><tr><td><b>cx q[6],q[8];</b></td></tr><tr><td><font color="darkgray">node: 11</font></td></tr></table>>];
  0 -> 4;
  0 -> 5;
  1 -> 4;
  1 -> 6;
  2 -> 3;
  2 -> 7;
  2 -> 8;
  2 -> 11;
  3 -> 8;
  3 -> 9;
  3 -> 10;
  8 -> 11;
  9 -> 10;
  10 -> 11;
}
