BellPairInit 200 201 6:Z,2:Z [BellPrepare (1,3),(2,3)];Init 100 |+>;MultiBodyMeasure 0:X,4:X;MultiBodyMeasure 2:Z,3:Z;RotateSingleCellPatch 100;
BusyRegion (2,5),(2,6),StepsToClear(1);
BusyRegion (2,5),(2,6),StepsToClear(0);HGate 1;
HGate 1;RotateSingleCellPatch 1;
BusyRegion (1,2),(0,2),StepsToClear(1);
BusyRegion (1,2),(0,2),StepsToClear(0);MultiBodyMeasure 0:Z,100:Z;

