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

#include "cframe.h"

// Global counter of total threads running
int MAX_THREADS;
int MAX_ACTIVE_TASKS;
int MAX_PARALLEL_OBJECTIVES;
char PARALLEL_MODE;

bool mySolutionFound = false;
int myTaskCnt = 0;
int myMaxTaskCnt = 0;


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


bool errorAtPO(Circuit& aCircuit){
    for (auto& myOutput : aCircuit.theCircuitOutputs) {
        if ((aCircuit.theCircuitState[myOutput] == SignalType::D) || (aCircuit.theCircuitState[myOutput] == SignalType::D_b)){
            return true;
        }
    }
    return false;
}


SignalType getNonControllingValue(std::string aGate){
    if (aGate == "AND" || aGate == "and" || aGate == "NAND" || aGate == "nand"){
        return SignalType::ONE;
    }
    if (aGate == "OR" || aGate == "or" || aGate == "NOR" || aGate == "nor"){
        return SignalType::ZERO;
    }
    if (aGate == "XOR" || aGate == "xor" || aGate == "XNOR" || aGate == "xnor"){
        return SignalType::ZERO;
    }
    std::cout << "Error: Gate type " << aGate << " should not be on the DFrontier" << std::endl;
    return SignalType::ZERO;
}


std::vector<std::pair<std::string, SignalType>> getMultipleObjectives(Circuit& aCircuit){
    std::vector<std::pair<std::string, SignalType>> myObjectives = std::vector<std::pair<std::string, SignalType>>();
    if (aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X){
        SignalType mySAObjective = (aCircuit.theFaultValue == SignalType::D) ? SignalType::ONE : SignalType::ZERO;
        myObjectives.push_back(std::pair<std::string, SignalType>(aCircuit.theFaultLocation, mySAObjective));
        return myObjectives;
    }
    for (std::string myDFrontierGate : aCircuit.theDFrontier) {
        if (myObjectives.size() >= static_cast<std::size_t>(MAX_PARALLEL_OBJECTIVES)) {
            break;
        }
        for (auto& myDFrontierGateInput : aCircuit.theCircuit[myDFrontierGate].inputs) {
            if ((aCircuit.theCircuitState[myDFrontierGateInput] == SignalType::X)) {
                if (myObjectives.size() >= static_cast<std::size_t>(MAX_PARALLEL_OBJECTIVES)) {
                    break;
                }
                myObjectives.push_back(std::pair<std::string, SignalType>(myDFrontierGateInput, getNonControllingValue(aCircuit.theCircuit[myDFrontierGate].gateType)));
            }
        }
    }

    if (myObjectives.size() == 0){
        std::cout << "Error: Unable to create objective when it should have been possible" << std::endl;
    }

    return myObjectives;
}


std::pair<std::string, SignalType> getObjective(Circuit& aCircuit){
    if (aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X){
        SignalType mySAObjective = (aCircuit.theFaultValue == SignalType::D) ? SignalType::ONE : SignalType::ZERO;
        return std::pair<std::string, SignalType>(aCircuit.theFaultLocation, mySAObjective);
    }
    std::string myDFrontierGate = *(aCircuit.theDFrontier.begin());
    for (auto& myDFrontierGateInput : aCircuit.theCircuit[myDFrontierGate].inputs) {
        if (aCircuit.theCircuitState[myDFrontierGateInput] == SignalType::X){
            return std::pair<std::string, SignalType>(myDFrontierGateInput, getNonControllingValue(aCircuit.theCircuit[myDFrontierGate].gateType));
        }
    }
    std::cout << "Error: Unable to create objective when it should have been possible" << std::endl;
    return std::pair<std::string, SignalType>("", SignalType::X);
}


std::pair<std::string, SignalType> doBacktrace(Circuit& aCircuit, std::pair<std::string, SignalType> anObjective){
    std::string myBacktraceSignal = anObjective.first;
    SignalType myBacktraceValue = anObjective.second;

    while (std::ranges::find(aCircuit.theCircuitInputs, myBacktraceSignal) == aCircuit.theCircuitInputs.end()){
        Gate myGate = aCircuit.theCircuit[myBacktraceSignal];
        bool myGateBubble = (myGate.gateType == "NAND") || (myGate.gateType == "nand") || (myGate.gateType == "NOR") || (myGate.gateType == "nor") || (myGate.gateType == "XNOR") || (myGate.gateType == "xnor") || (myGate.gateType == "NOT") || (myGate.gateType == "not");

        std::string myBacktraceSignalPrev = myBacktraceSignal; // DEBUG code
        for (auto& myBacktraceGateInput : aCircuit.theCircuit[myBacktraceSignal].inputs){
            if (aCircuit.theCircuitState[myBacktraceGateInput] == SignalType::X){
                myBacktraceSignal = myBacktraceGateInput;
                break;
            }
        }

        if (myBacktraceSignal == myBacktraceSignalPrev){ // DEBUG code
            std::cout << "Error: unable to perform backtrace (find input value == X)" << std::endl;
        }

        if (myGateBubble){
            myBacktraceValue = (myBacktraceValue == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        }
    }

    return std::pair<std::string, SignalType>(myBacktraceSignal, myBacktraceValue);
}


std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelDecisions(Circuit& aCircuit){

    // aCircuit.printCircuitState();
    if (mySolutionFound) {
        return std::unordered_map<std::string, SignalType>();
    }

    if (errorAtPO(aCircuit)){
        mySolutionFound = true;
        return aCircuit.getCurrCircuitInputValues();
    }
    if (aCircuit.theDFrontier.empty() && !(aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X)){
        return std::unordered_map<std::string, SignalType>();
    }
    std::pair<std::string, SignalType> myObjective = getObjective(aCircuit);

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;

    // START OMP implementation
    const int myNumTasks = 2;
    std::unordered_map<std::string, SignalType> myPODEMResults[myNumTasks];
    std::vector<Circuit> myCircuits = std::vector<Circuit>(myNumTasks);
    // #pragma omp parallel
    // #pragma omp single
    // {

    // std::cout << "Number of active tasks: " << myTaskCnt << std::endl;

    if (myTaskCnt < MAX_ACTIVE_TASKS){

        #pragma omp critical
        {
            myTaskCnt += 2;
            if (myTaskCnt > myMaxTaskCnt) {
                myMaxTaskCnt = myTaskCnt;
            }
        }

        #pragma taskgroup
        {
            // std::cout << "Spawning tasks from thread " << omp_get_thread_num() << std::endl;
            #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
            {
                // std::cout << "Executing task 0 in thread " << omp_get_thread_num() << " at nested level " << omp_get_level() << std::endl;
                myCircuits[0] = aCircuit;
                myCircuits[0].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                myPODEMResults[0] = runPODEMRecursiveParallelDecisions(myCircuits[0]);
                // finishedTasks0 = true;
                #pragma omp critical
                {
                    myTaskCnt--;
                }
            }

            #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
            {
                // std::cout << "Executing task 1 in thread " << omp_get_thread_num() << " at nested level " << omp_get_level() << std::endl;
                myCircuits[1] = aCircuit;
                myCircuits[1].setAndImplyCircuitInput(myDecision.first, (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE);
                myPODEMResults[1] = runPODEMRecursiveParallelDecisions(myCircuits[1]);
                // finishedTasks1 = true;
                #pragma omp critical
                {
                    myTaskCnt--;
                }
            }
            // std::cout << "Thread waiting at taskwait " << omp_get_thread_num() << std::endl;
            #pragma omp taskwait
        }
        // }
        // std::cout << "Thread proceeding after taskwait " << omp_get_thread_num() << std::endl;

        // while (!finishedTasks0 || !finishedTasks1) {
        //     #pragma omp taskyield
        // }

        if (!myPODEMResults[0].empty()) {
            aCircuit = myCircuits[0];
            return myPODEMResults[0];
        } else if (!myPODEMResults[1].empty()) {
            aCircuit = myCircuits[1];
            return myPODEMResults[1];
        } else {
            aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
            return std::unordered_map<std::string, SignalType>();
        }

    // END OMP implementation
    } else {

        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }
        aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
        return std::unordered_map<std::string, SignalType>();

    }

    // START serial implementation
    // aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
    // if(!myPODEMResult.empty()){
    //     return myPODEMResult;
    // }

    // myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
    // aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
    // if(!myPODEMResult.empty()){
    //     return myPODEMResult;
    // }
    // aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
    // return std::unordered_map<std::string, SignalType>();
    // END serial implementation
}


std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelSignals(Circuit& aCircuit){

    // aCircuit.printCircuitState();
    if (mySolutionFound) {
        return std::unordered_map<std::string, SignalType>();
    }

    if (errorAtPO(aCircuit)){
        mySolutionFound = true;
        return aCircuit.getCurrCircuitInputValues();
    }
    if (aCircuit.theDFrontier.empty() && !(aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X)){
        return std::unordered_map<std::string, SignalType>();
    }
    std::vector<std::pair<std::string, SignalType>> myObjectives = getMultipleObjectives(aCircuit);
    int myObjectivesSize = myObjectives.size();

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;


    // START OMP implementation
    const int myNumTasks = MAX_PARALLEL_OBJECTIVES;
    std::unordered_map<std::string, SignalType> myPODEMResults[myNumTasks];
    std::vector<Circuit> myCircuits = std::vector<Circuit>(myNumTasks);

    // std::cout << "Number of active tasks: " << myTaskCnt << std::endl;

    if (myTaskCnt < MAX_ACTIVE_TASKS){

        #pragma omp critical
        {
            myTaskCnt += myObjectives.size();
            if (myTaskCnt > myMaxTaskCnt) {
                myMaxTaskCnt = myTaskCnt;
            }
        }

        #pragma taskgroup
        {
            // std::cout << "Spawning tasks from thread " << omp_get_thread_num() << std::endl;
            for (int i = 0; i < myObjectivesSize; i++) {
                std::pair<std::string, SignalType> myObjective = myObjectives[i];
                #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
                {
                    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

                    myCircuits[i] = aCircuit;
                    myCircuits[i].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                    myPODEMResults[i] = runPODEMRecursiveParallelSignals(myCircuits[i]);

                    if(myPODEMResults[i].empty()){
                        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
                        myCircuits[i].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                        myPODEMResults[i] = runPODEMRecursiveParallelSignals(myCircuits[i]);
                    }

                    #pragma omp critical
                    {
                        myTaskCnt--;
                    }
                }
            }
            // std::cout << "Thread waiting at taskwait " << omp_get_thread_num() << std::endl;
            #pragma omp taskwait
        }
        // }
        // std::cout << "Thread proceeding after taskwait " << omp_get_thread_num() << std::endl;

        // while (!finishedTasks0 || !finishedTasks1) {
        //     #pragma omp taskyield
        // }

        for (int i = 0; i < myObjectivesSize; i++) {
            if (!myPODEMResults[i].empty()) {
                aCircuit = myCircuits[i];
                return myPODEMResults[i];
            }
        }

        return std::unordered_map<std::string, SignalType>();

    } else {
        std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjectives[0]);

        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }
        aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
        return std::unordered_map<std::string, SignalType>();

    }
    // END OMP implementation

    // START serial implementation
    // aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
    // if(!myPODEMResult.empty()){
    //     return myPODEMResult;
    // }

    // myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
    // aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
    // if(!myPODEMResult.empty()){
    //     return myPODEMResult;
    // }
    // aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
    // return std::unordered_map<std::string, SignalType>();
    // END serial implementation
}


std::unique_ptr<std::unordered_map<std::string, SignalType>> startPODEM(Circuit& aCircuit, std::pair<std::string, SignalType> anSSLFault){
    aCircuit.setCircuitFault(anSSLFault.first, anSSLFault.second);
    aCircuit.resetCircuit();
    mySolutionFound = false;

   std::unordered_map<std::string, SignalType> myTestVector;
    #pragma omp parallel
    #pragma omp single
    {
        // std::cout << "Coordinator Thread " << omp_get_thread_num() << std::endl;
        if (PARALLEL_MODE == 's' || PARALLEL_MODE == 'S'){
            myTestVector = runPODEMRecursiveParallelSignals(aCircuit);
        } else {
            myTestVector = runPODEMRecursiveParallelDecisions(aCircuit);
        }
    }

    if (!myTestVector.empty()) {
        return std::make_unique<std::unordered_map<std::string, SignalType>>(myTestVector);
    } else {
        return NULL;
    }
}


std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> runATPG(Circuit& aCircuit) {

    double myTotalComputationTime = 0.0;

    // Unique_ptr to vector of results for each SSL fault ATPG
    // Each result entry consists of: | SSL fault (pair of string and SignalType) | Computation type (double) | Generated Test Vector (Unordered map of input signal names and values) - return empty map if SSL fault undetectable |
    std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> myATPGData = std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>();

    std::vector<std::pair<std::string, SignalType>> mySSLFaults = std::vector<std::pair<std::string, SignalType>>();

    std::vector<std::string> myTest = std::vector<std::string>();
    for (auto& mySignalPair : aCircuit.theCircuit){
        mySSLFaults.push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D));
        mySSLFaults.push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D_b));
    }
    // mySSLFaults.push_back(std::pair<std::string, SignalType>("213_BRANCH0_259", SignalType::D));

    std::size_t myNumFaults = mySSLFaults.size();

    while (!mySSLFaults.empty()){
        std::cout << "\nProgress: " << (myNumFaults - mySSLFaults.size()) << " / " << myNumFaults << " faults complete" << std::endl;
        std::pair<std::string, SignalType> myTargetSSLFault = mySSLFaults.back();

        std::cout << "Info: Running PODEM to detect fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;

        const auto mySingleSSLATPGStartTime = std::chrono::steady_clock::now();
        std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = startPODEM(aCircuit, myTargetSSLFault);
        const auto mySingleSSLATPGTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mySingleSSLATPGStartTime).count();
        myTotalComputationTime += mySingleSSLATPGTime;

        if (myTestVector != NULL){
            myATPGData.push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, *myTestVector));

            std::cout << "\n--- Found test vector for signal " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << " ---" << std::endl;
            for (auto& [myTestVectorInputSignal, myTestVectorInputValue] : *myTestVector){
                std::cout << std::setw(30) << myTestVectorInputSignal << ": " << getSignalStateString(myTestVectorInputValue) << std::endl;
            }

        } else {
            myATPGData.push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, std::unordered_map<std::string, SignalType>()));
            std::cout << "Info: Unable to generate test vector for fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;
        }

        std::cout << "Computation time for single SSL fault ATPG (sec): " << std::fixed << std::setprecision(10) << mySingleSSLATPGTime << '\n';
        mySSLFaults.pop_back();
    }

    std::cout << "\nTotal ATPG Computation Time (sec): " << std::fixed << std::setprecision(10) << myTotalComputationTime << '\n';

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

    std::cout << "\nBench File: " << myCircuitFile << std::endl;
    std::cout << "Threads: " << MAX_THREADS << std::endl;
    std::cout << "Max Active Tasks: " << MAX_ACTIVE_TASKS << std::endl;
    std::cout << "Max Parallel Objectives: " << MAX_PARALLEL_OBJECTIVES << std::endl;
    if (PARALLEL_MODE == 's' || PARALLEL_MODE == 'S'){
        std::cout << "Parallel Mode: Across Signals" << std::endl << std::endl;
    } else {
        std::cout << "Parallel Mode: Across Decisions" << std::endl << std::endl;
    }


    // end parsing of commandline options //////////////////////////////////////

    omp_set_num_threads(MAX_THREADS);

    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(myCircuitFile);

    std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> myATPGData;

    myATPGData = runATPG(*myCircuit);

    for (auto& mySSLTestResult : myATPGData) {
        std::cout << std::get<0>(mySSLTestResult).first << "," << (std::get<0>(mySSLTestResult).second == SignalType::D ? '0' : '1') << "," << std::get<1>(mySSLTestResult) << "," << (!std::get<2>(mySSLTestResult).empty()) << std::endl;
    }

    std::cout << "Max live tasks " << myMaxTaskCnt << std::endl;

    return 0;
}