BellPairInit 200 201 6:X,2:X [BellPrepare (1,3),(2,3)];Init 100 |+>;RotateSingleCellPatch 100;
BusyRegion (2,5),(2,6),StepsToClear(2);
BusyRegion (2,5),(2,6),StepsToClear(1);
BusyRegion (2,5),(2,6),StepsToClear(0);HGate 1;
HGate 1;RotateSingleCellPatch 1;
BusyRegion (1,2),(0,2),StepsToClear(2);
BusyRegion (1,2),(0,2),StepsToClear(1);
BusyRegion (1,2),(0,2),StepsToClear(0);

