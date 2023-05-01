









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(6,5);BellPrepare (5,6),(5,5)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,5),(6,5);MergeContract (4,6),(5,6)];MeasureSinglePatch 25 Z;RequestYState 28 0;
BellBasedCNOT 0 28 29 30 [ExtendSplit (6,6),(6,5);BellPrepare (5,4),(5,5)];
BellBasedCNOT 0 28 29 30 [BellMeasure (5,5),(6,5);MergeContract (4,4),(5,4)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (6,6),(6,5);BellPrepare (5,4),(5,5)];
BellBasedCNOT 0 28 31 32 [BellMeasure (5,5),(6,5);MergeContract (4,4),(5,4)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 28 0;

