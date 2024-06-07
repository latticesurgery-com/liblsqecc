









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(5,6);BellPrepare (4,5),(5,5)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,5),(5,6);MergeContract (4,6),(4,5)];MeasureSinglePatch 25 Z;RequestYState 28 0 [PrepareY (7,6)];
MultiBodyMeasure 28:Z,0:Z [TwoPatchMeasure (7,6),(6,6)];MeasureSinglePatch 28 X;
ZGate 0;

