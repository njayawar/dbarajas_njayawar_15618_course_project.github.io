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

#include <unistd.h>
#include <omp.h>

#include "cframe.h"


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


std::unordered_map<std::string, SignalType> runPODEMRecursive(Circuit& aCircuit){

    if (errorAtPO(aCircuit)){
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
    const int myNumDecisions = 2;
    std::unordered_map<std::string, SignalType> myPODEMResults[myNumDecisions];
    const int myNumThreads = myNumDecisions + 1;
    Circuit myCircuits[myNumThreads];
    #pragma omp parallel num_threads(myNumThreads)
    {
        int myThreadNum = omp_get_thread_num();
        if (myThreadNum == 0) {
            myDecision.second = SignalType::ZERO;
        } else if (myThreadNum == 1) {
            myDecision.second = SignalType::ONE;
        } else {
            myDecision.second = SignalType::X;
        }
        myCircuits[myThreadNum] = aCircuit;
        myCircuits[myThreadNum].setAndImplyCircuitInput(myDecision.first, myDecision.second);
        if (myThreadNum == 0 || myThreadNum == 1) {
            myPODEMResults[myThreadNum] = runPODEMRecursive(myCircuits[myThreadNum]);
        }
    }

    if (!myPODEMResults[0].empty()) {
        // std::cout << "Case 0: " << (*aCircuit == *myCircuits[0]) << std::endl;
        aCircuit = myCircuits[0];
        return myPODEMResults[0];
    } else if (!myPODEMResults[1].empty()) {
        // std::cout << "Case 1: " << (*aCircuit == *myCircuits[1]) << std::endl;
        aCircuit = myCircuits[1];
        return myPODEMResults[1];
    } else {
        // std::cout << "Case 2" << std::endl;
        aCircuit = myCircuits[2];
        return std::unordered_map<std::string, SignalType>();
    }
    // END OMP implementation

    // aCircuit->setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // std::unique_ptr<std::unordered_map<std::string, SignalType>> myPODEMResult = runPODEMRecursive(aCircuit);
    // if(myPODEMResult != NULL){
    //     return myPODEMResult;
    // }

    // myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
    // aCircuit->setAndImplyCircuitInput(myDecision.first, myDecision.second);
    // myPODEMResult = runPODEMRecursive(aCircuit);
    // if(myPODEMResult != NULL){
    //     return myPODEMResult;
    // }
    // aCircuit->setAndImplyCircuitInput(myDecision.first, SignalType::X);
    // return NULL;
}


std::unique_ptr<std::unordered_map<std::string, SignalType>> startPODEM(Circuit& aCircuit, std::pair<std::string, SignalType> anSSLFault){
    aCircuit.setCircuitFault(anSSLFault.first, anSSLFault.second);
    aCircuit.resetCircuit();
    auto myTestVector = runPODEMRecursive(aCircuit);
    if (myTestVector.empty()) {
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

    if (argc != 2) {
        std::cout << "Need to supply 1 input circuit file" << std::endl;
        return -1;
    }

    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(argv[1]);

    std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>> myATPGData;

    myATPGData = runATPG(*myCircuit);

    for (auto& mySSLTestResult : myATPGData) {
        std::cout << std::get<0>(mySSLTestResult).first << "," << (std::get<0>(mySSLTestResult).second == SignalType::D ? '0' : '1') << "," << std::get<1>(mySSLTestResult) << "," << (!std::get<2>(mySSLTestResult).empty()) << std::endl;
    }

    return 0;
}