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
#include <ranges>

#include <unistd.h>
#include <omp.h>
#include <getopt.h>

#include "cframe.h"
#include "podem.h"

// Global counter of total threads running
int MAX_THREADS;
char PARALLEL_MODE;

int MAX_ACTIVE_TASKS;
int MAX_PARALLEL_OBJECTIVES;

bool theSolutionFound = false;
int theTaskCnt = 0;
int theMaxTaskCnt = 0;
double theTotalComputationTime = 0;


// Print usage information
void usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -b  --bench <FILE>                  Run specified function on input\n");
    printf("  -t  --max_threads <INT>             Number of threads to use\n");
    printf("  -a  --max_active_tasks <INT>        Number of active tasks to use\n");
    printf("  -o  --max_parallel_objectives <INT> Number of parallel objectives when parallelizing across decisions\n");
    printf("  -m  --parallel_mode <char>          's' or 'd' parallelize across decisions or signals\n");
    printf("  -?  --help                          This message\n");
}


// Line parser for string splitting
static std::vector<std::string> tokenize_line(std::string s) {
    std::ranges::replace(s, '/', ' ');
    std::ranges::replace(s, '.', ' ');

    std::istringstream stream(s);

    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}


// Initiates the recursive PODEM algorithm based on parallization strategy
std::unique_ptr<std::unordered_map<std::string, SignalType>> startPODEM(Circuit& aCircuit, std::pair<std::string, SignalType> anSSLFault){
    // Set fault and initialize counters
    aCircuit.setCircuitFault(anSSLFault.first, anSSLFault.second);
    aCircuit.resetCircuit();
    theSolutionFound = false;
    theTaskCnt = 0;
    theMaxTaskCnt = 0;

   std::unordered_map<std::string, SignalType> myTestVector;
    #pragma omp parallel
    #pragma omp single
    {
        // std::cout << "Coordinator Thread " << omp_get_thread_num() << std::endl;
        if (PARALLEL_MODE == 's' || PARALLEL_MODE == 'S'){
            myTestVector = runPODEMRecursiveParallelSignals(aCircuit);
        } else if (PARALLEL_MODE == 'd' || PARALLEL_MODE == 'D') {
            myTestVector = runPODEMRecursiveParallelDecisions(aCircuit);
        } else {
            myTestVector = runPODEMRecursiveSerial(aCircuit);
        }
    }

    // Return ATPG success or failure
    if (!myTestVector.empty()) {
        return std::make_unique<std::unordered_map<std::string, SignalType>>(myTestVector);
    } else {
        return NULL;
    }
}


// Begin ATPG on given circuit and return comprehensive results
std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> runATPG(Circuit& aCircuit) {

    double myTotalComputationTime = 0.0;

    // Unique_ptr to vector of results for each SSL fault ATPG
    // Each result entry consists of: | SSL fault (pair of string and SignalType) | Computation type (double) | Generated Test Vector (Unordered map of input signal names and values) - return empty map if SSL fault undetectable |
    std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> myATPGData = std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>();

    std::vector<std::pair<std::string, SignalType>> mySSLFaults = std::vector<std::pair<std::string, SignalType>>();

    // Add all possible signal faults
    std::vector<std::string> myTest = std::vector<std::string>();
    for (auto& mySignalPair : aCircuit.theCircuit){
        mySSLFaults.push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D));
        mySSLFaults.push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D_b));
    }
    // Single test fault
    // mySSLFaults.push_back(std::pair<std::string, SignalType>("213_BRANCH0_259", SignalType::D));

    std::size_t myNumFaults = mySSLFaults.size();
    (void) myNumFaults;

    // Report results
    while (!mySSLFaults.empty()){
        std::pair<std::string, SignalType> myTargetSSLFault = mySSLFaults.back();

        #ifdef DEBUG
        std::cout << "\nProgress: " << (myNumFaults - mySSLFaults.size()) << " / " << myNumFaults << " faults complete" << std::endl;
        std::cout << "Info: Running PODEM to detect fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;
        #endif

        const auto mySingleSSLATPGStartTime = std::chrono::steady_clock::now();
        std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = startPODEM(aCircuit, myTargetSSLFault);
        const auto mySingleSSLATPGTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mySingleSSLATPGStartTime).count();
        myTotalComputationTime += mySingleSSLATPGTime;

        if (myTestVector != NULL){
            myATPGData.push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, *myTestVector));

            #ifdef DEBUG
            std::cout << "\n--- Found test vector for signal " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << " ---" << std::endl;
            for (auto& [myTestVectorInputSignal, myTestVectorInputValue] : *myTestVector){
                std::cout << std::setw(30) << myTestVectorInputSignal << ": " << getSignalStateString(myTestVectorInputValue) << std::endl;
            }
            #endif

        } else {
            myATPGData.push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, std::unordered_map<std::string, SignalType>()));
            #ifdef DEBUG
            std::cout << "Info: Unable to generate test vector for fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;
            #endif
        }

        #ifdef DEBUG
        std::cout << "Computation time for single SSL fault ATPG (sec): " << std::fixed << std::setprecision(10) << mySingleSSLATPGTime << '\n';
        #endif

        mySSLFaults.pop_back();
    }

    theTotalComputationTime = myTotalComputationTime;

    return myATPGData;
}


int main(int argc, char** argv) {

    // parse commandline options ////////////////////////////////////////////
    int opt;
    static struct option long_options[] = {
        {"bench",            1, 0, 'b'},
        {"max_threads",      1, 0, 't'},
        {"max_active_tasks", 1, 0, 'a'},
        {"max_parallel_objectives", 1, 0, 'o'},
        {"parallel_mode",    1, 0, 'm'},
        {"help",             0, 0, '?'},
        {0 ,0, 0, 0}
    };

    std::string myCircuitFile;
    PARALLEL_MODE = '0';

    while ((opt = getopt_long(argc, argv, "b:t:a:o:m:?", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'b':
            myCircuitFile = std::string(optarg);
            break;
        case 't':
            MAX_THREADS = atoi(optarg);
            break;
        case 'a':
            MAX_ACTIVE_TASKS = atoi(optarg);
            break;
        case 'o':
            MAX_PARALLEL_OBJECTIVES = atoi(optarg);
            break;
        case 'm':
            PARALLEL_MODE = *optarg;
            break;
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (argc != 11) {
        usage(argv[0]);
        return 1;
    }

    #ifdef DEBUG
    std::cout << "\nBench File: " << myCircuitFile << std::endl;
    std::cout << "Threads: " << MAX_THREADS << std::endl;
    std::cout << "Max Active Tasks: " << MAX_ACTIVE_TASKS << std::endl;
    std::cout << "Max Parallel Objectives: " << MAX_PARALLEL_OBJECTIVES << std::endl;
    if (PARALLEL_MODE == 's' || PARALLEL_MODE == 'S'){
        std::cout << "Mode: Parallel Across Signals" << std::endl << std::endl;
    } else if (PARALLEL_MODE == 'd' || PARALLEL_MODE == 'D') {
        std::cout << "Mode: Parallel Across Decisions" << std::endl << std::endl;
    } else {
        std::cout << "Mode: Serial" << std::endl << std::endl;
    }
    #endif
    // end parsing of commandline options //////////////////////////////////////

    omp_set_num_threads(MAX_THREADS);

    // Parse circuit
    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(myCircuitFile);

    // Run ATPG
    std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> myATPGData;
    myATPGData = runATPG(*myCircuit);

    // Print details
    std::cout << "\n-------------- Total ATPG Computation Time (sec): " << std::fixed << std::setprecision(10) << theTotalComputationTime << " --------------" << std::endl;

    std::cout << "\nBench File: " << myCircuitFile << std::endl;
    std::cout << "Threads: " << MAX_THREADS << std::endl;
    std::cout << "Max Active Tasks: " << MAX_ACTIVE_TASKS << std::endl;
    std::cout << "Max Parallel Objectives: " << MAX_PARALLEL_OBJECTIVES << std::endl;
    if (PARALLEL_MODE == 's' || PARALLEL_MODE == 'S'){
        std::cout << "Mode: Parallel Across Signals" << std::endl << std::endl;
    } else if (PARALLEL_MODE == 'd' || PARALLEL_MODE == 'D') {
        std::cout << "Mode: Parallel Across Decisions" << std::endl << std::endl;
    } else {
        std::cout << "Mode: Serial" << std::endl << std::endl;
    }

    for (auto& mySSLTestResult : myATPGData) {
        std::cout << std::get<0>(mySSLTestResult).first << "," << (std::get<0>(mySSLTestResult).second == SignalType::D ? '0' : '1') << "," << std::get<1>(mySSLTestResult) << "," << (!std::get<2>(mySSLTestResult).empty()) << std::endl;
    }

    #ifdef DEBUG
    std::cout << "Max live tasks " << theMaxTaskCnt << std::endl;
    #endif

    // Write statistics to results file
    std::vector<std::string> myTokenizedCircuitFileName = tokenize_line(myCircuitFile);

    std::string myBenchName = myTokenizedCircuitFileName[myTokenizedCircuitFileName.size()-2];

    std::string myOutputFileName = "./results/output_b_" + myBenchName + "_t_" + std::to_string(MAX_THREADS) + "_a_" + std::to_string(MAX_ACTIVE_TASKS) + "_o_" + std::to_string(MAX_PARALLEL_OBJECTIVES) + "_m_" + PARALLEL_MODE;
    std::ofstream myOutputFile(myOutputFileName);

    if (!myOutputFile) {
        std::cout << "Error: Unable to open output file for writing" << std::endl;
    }

    myOutputFile << theTotalComputationTime << std::endl;

    for (auto& mySSLTestResult : myATPGData) {
        myOutputFile << std::get<0>(mySSLTestResult).first << "," << (std::get<0>(mySSLTestResult).second == SignalType::D ? '0' : '1') << "," << std::get<1>(mySSLTestResult) << "," << (!std::get<2>(mySSLTestResult).empty()) << std::endl;
    }

    myOutputFile.close();

    return 0;
}
