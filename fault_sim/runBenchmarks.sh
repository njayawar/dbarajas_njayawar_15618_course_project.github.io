clear
make clean
make

rm ./benchmarks_results.txt
touch ./benchmarks_results.txt

# python3 gen_test_vectors.py

# Fault Simulation Runs
for CIRCUIT in c17 c25 c432 c7552
do
    BENCH=../resource/benchmarks/ISCAS85/$CIRCUIT.bench
    for TEST_VECTOR in $(ls ./test_vectors/*${CIRCUIT}*)
    do
        echo $TEST_VECTOR
        ./faultSimulation $BENCH $TEST_VECTOR
    done
done