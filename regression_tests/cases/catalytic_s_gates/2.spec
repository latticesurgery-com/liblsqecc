









<<<<<<< HEAD
RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (2,2),(2,1);BellPrepare (1,2),(1,1)];
BellBasedCNOT 0 25 26 27 [BellMeasure (1,1),(2,1);MergeContract (0,2),(1,2)];MeasureSinglePatch 25 Z;RequestYState 28 0;
BellBasedCNOT 0 28 29 30 [ExtendSplit (2,2),(2,1);BellPrepare (1,0),(1,1)];
BellBasedCNOT 0 28 29 30 [BellMeasure (1,1),(2,1);MergeContract (0,0),(1,0)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (2,2),(2,1);BellPrepare (1,0),(1,1)];
BellBasedCNOT 0 28 31 32 [BellMeasure (1,1),(2,1);MergeContract (0,0),(1,0)];
=======
RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(5,6);BellPrepare (4,7),(5,7)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,7),(5,6);MergeContract (4,6),(4,7)];MeasureSinglePatch 25 Z;RequestYState 28 0;BellBasedCNOT 0 28 29 30 [ExtendSplit (6,6),(7,6);BellPrepare (4,5),(5,5);BellPrepare (6,5),(7,5)];
BellBasedCNOT 0 28 29 30 [BellMeasure (5,5),(6,5);BellMeasure (7,5),(7,6);MergeContract (4,4),(4,5)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (6,6),(5,6);BellPrepare (4,5),(5,5)];
BellBasedCNOT 0 28 31 32 [BellMeasure (5,5),(5,6);MergeContract (4,4),(4,5)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);RequestYState 28 0;

