for jj in 3 5
do
  echo "Running grover_$jj"
  lsqecc_slicer -i cases/end_to_end/grover/grover_$jj.lli -L compact --graceful --noslices | \
    sed "s/Made patch computation. Took [0-9]*.[0-9e\-]*s./Made patch computation. Took <time_removed_by_case_script>/"
  echo
done