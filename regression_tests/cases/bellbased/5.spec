BellPairInit 200 201 6:X,5:Z [BellPrepare (8,11),(9,11);BellPrepare (10,11),(10,10);BellPrepare (10,9),(10,8);BellPrepare (10,7),(11,7)];
BellPairInit 200 201 6:X,5:Z [BellMeasure (9,11),(10,11);BellMeasure (10,10),(10,9);BellMeasure (10,8),(10,7);Move (11,7),(11,6)];Init 100 |+>;MultiBodyMeasure 0:X,4:X;MultiBodyMeasure 5:Z,6:Z;MultiBodyMeasure 2:Z,3:Z;RotateSingleCellPatch 100;
BusyRegion StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);HGate 1;HGate 1;
HGate 1;RotateSingleCellPatch 1;
BusyRegion StepsToClear(1);
BusyRegion (6,8),(6,9),StepsToClear(0);MultiBodyMeasure 0:Z,100:Z;

