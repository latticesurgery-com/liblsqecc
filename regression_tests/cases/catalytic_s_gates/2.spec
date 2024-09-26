









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (2,2),(1,2);BellPrepare (0,3),(1,3)];
BellBasedCNOT 0 25 26 27 [MergeContract (0,2),(0,3);BellMeasure (1,3),(1,2)];MeasureSinglePatch 25 Z;RequestYState 28 0;BellBasedCNOT 0 28 29 30 [ExtendSplit (2,2),(3,2);BellPrepare (0,1),(1,1);BellPrepare (2,1),(3,1)];
BellBasedCNOT 0 28 29 30 [MergeContract (0,0),(0,1);BellMeasure (1,1),(2,1);BellMeasure (3,1),(3,2)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (2,2),(1,2);BellPrepare (0,1),(1,1)];
BellBasedCNOT 0 28 31 32 [MergeContract (0,0),(0,1);BellMeasure (1,1),(1,2)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);RequestYState 28 0;

