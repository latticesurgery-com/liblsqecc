BellPairInit 200 201 0:Z,3:X [BellPrepare (0,6),(0,5);BellPrepare (0,4),(0,3);BellPrepare (0,2),(0,1)];
BellPairInit 200 201 0:Z,3:X [Move (0,6),(1,6);BellMeasure (0,5),(0,4);BellMeasure (0,3),(0,2)];Init 100 |+>;RotateSingleCellPatch 100;
BusyRegion (2,5),(2,6),StepsToClear(2);
BusyRegion (2,5),(2,6),StepsToClear(1);
BusyRegion (2,5),(2,6),StepsToClear(0);HGate 1;
HGate 1;RotateSingleCellPatch 1;
BusyRegion (1,2),(1,3),StepsToClear(2);
BusyRegion (1,2),(1,3),StepsToClear(1);
BusyRegion (1,2),(1,3),StepsToClear(0);

