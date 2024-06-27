









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (2,2),(1,2);BellPrepare (0,3),(1,3)];
BellBasedCNOT 0 25 26 27 [BellMeasure (1,3),(1,2);MergeContract (0,2),(0,3)];MeasureSinglePatch 25 Z;RequestYState 28 0 [PrepareY (3,2)];
MultiBodyMeasure 28:Z,0:Z [TwoPatchMeasure (3,2),(2,2)];MeasureSinglePatch 28 X;
ZGate 0;

