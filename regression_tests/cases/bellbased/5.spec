BellPairInit 200 201 6:X,5:Z;
BusyRegion (8,9),(9,9),(10,9),(11,9),(11,8),(11,7),(11,6),StepsToClear(1);
BusyRegion (8,9),(9,9),(10,9),(11,9),(11,8),(11,7),(11,6),StepsToClear(0);MultiBodyMeasure 0:X,4:X;MultiBodyMeasure 5:Z,6:Z;MultiBodyMeasure 2:Z,3:Z;HGate 1;
HGate 1;RotateSingleCellPatch 1;
BusyRegion (6,8),(6,9),StepsToClear(1);
BusyRegion (6,8),(6,9),StepsToClear(0);RequestYState 7 0;MultiBodyMeasure 0:Z,7:Z;

