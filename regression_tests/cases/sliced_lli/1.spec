Init 100 |+>;MultiBodyMeasure 0:X,4:X;MultiBodyMeasure 5:Z,6:Z;MultiBodyMeasure 2:Z,3:Z;RotateSingleCellPatch 100;
BusyRegion OccupiedRegion:(2|5)OccupiedRegion:(2|6),StepsToClear(1);
BusyRegion OccupiedRegion:(2|5)OccupiedRegion:(2|6),StepsToClear(0);HGate 1;
HGate 1 #WaitAtMostFor 2;RotateSingleCellPatch 1;
BusyRegion OccupiedRegion:(1|2)OccupiedRegion:(1|3),StepsToClear(1);
BusyRegion OccupiedRegion:(1|2)OccupiedRegion:(1|3),StepsToClear(0);

