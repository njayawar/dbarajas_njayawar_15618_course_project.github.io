clear
make clean
make debug

GHC_THREADS=(8)
GHC_MAX_TASKS=(10)
PSC_THREADS=(16 32 64 128)
PARALLEL_MODE=n


# Sequential Runs
# for CIRCUIT in c17 c25 c432
# do
#     BENCH=../resource/benchmarks/ISCAS85/$CIRCUIT.bench
#     ./atpg -b $BENCH -t 1 -a 20 -o 2 -m $PARALLEL_MODE
#     echo
# done


# Parallel Runs (core count vs speedup)
# for CIRCUIT in c17 c25 c432
# do
#     BENCH=../resource/benchmarks/ISCAS85/$CIRCUIT.bench
#     for THREADS in "${GHC_THREADS[@]}"
#     do
#         for MAX_TASKS in "${GHC_MAX_TASKS[@]}"
#         do
#             ./atpg -b $BENCH -t $THREADS -a $MAX_TASKS -o 2 -m $PARALLEL_MODE
#             echo
#         done
#     done
# done

for CIRCUIT in c1908
do
    BENCH=../resource/benchmarks/ISCAS85/$CIRCUIT.bench
    # for THREADS in "${GHC_THREADS[@]}"
    # do
    #     for MAX_TASKS in "${GHC_MAX_TASKS[@]}"
    #     do
    #         ./atpg -b $BENCH -t $THREADS -a $MAX_TASKS -o 2 -m $PARALLEL_MODE
    #         echo
    #     done
    # done

    # ./atpg -b $BENCH -t 1 -a 20 -o 2 -m $PARALLEL_MODE
    # echo
    PARALLEL_MODE=d
    ./atpg -b $BENCH -t 8 -a 5 -o 2 -m $PARALLEL_MODE
    echo

    # ./atpg -b $BENCH -t 1 -a 5 -o 2 -m $PARALLEL_MODE
    # echo
    # ./atpg -b $BENCH -t 2 -a 10 -o 2 -m $PARALLEL_MODE
    # echo
    # ./atpg -b $BENCH -t 4 -a 10 -o 2 -m $PARALLEL_MODE
    # echo


done

