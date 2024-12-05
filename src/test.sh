TEST=../resource/benchmarks/ISCAS85/c432.bench
clear
make clean
if [[ $(uname) == 'Darwin' ]]; then
    make mac
else
    make
fi
./atpg $TEST
