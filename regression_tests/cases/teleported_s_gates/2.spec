









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(6,5);BellPrepare (5,6),(5,5)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,5),(6,5);MergeContract (4,6),(5,6)];MeasureSinglePatch 25 Z;RequestYState 28 0 [PrepareY (6,7)];
MultiBodyMeasure 28:Z,0:Z [TwoPatchMeasure (6,7),(6,6)];MeasureSinglePatch 28 X;
ZGate 0;

