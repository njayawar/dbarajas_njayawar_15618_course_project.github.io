#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <climits>
#include <string>
#include <vector>
#include <thread>

#include <unistd.h>
#include <omp.h>
#include <getopt.h>

extern int MAX_PARALLEL_OBJECTIVES;
extern int MAX_ACTIVE_TASKS;

extern int theTaskCnt;
extern int theMaxTaskCnt;

extern bool theSolutionFound;

std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelSignals(Circuit& aCircuit);
std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelDecisions(Circuit& aCircuit);
std::unordered_map<std::string, SignalType> runPODEMRecursiveSerial(Circuit& aCircuit);
