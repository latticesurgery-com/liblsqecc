









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
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);RequestYState 28 0;
=======
RequestMagicState 25 0;MultiBodyMeasure 0:Z,25:Z;MeasureSinglePatch 25 X;RequestYState 26 0;Init 27 |+> 0:Z;
MultiBodyMeasure 0:Z,27:Z;
MultiBodyMeasure 27:X,26:X;MeasureSinglePatch 27 X;
HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);Init 28 |+> 0:Z;MultiBodyMeasure 0:Z,28:Z;
MultiBodyMeasure 28:X,26:X;MeasureSinglePatch 28 X;
HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 26 0;
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)

