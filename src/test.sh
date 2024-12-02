TEST=../resource/benchmarks/ISCAS85/c25.bench
clear
make clean
if [[ $(uname) == 'Darwin' ]]; then
    make mac
else
    make
fi
./atpg $TEST
