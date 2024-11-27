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


bool errorAtPO(std::unique_ptr<Circuit>& aCircuit){
    for (auto& myOutput : *(aCircuit->theCircuitOutputs)) {
        if (((*aCircuit->theCircuitState)[myOutput] == SignalType::D) || ((*aCircuit->theCircuitState)[myOutput] == SignalType::D_b)){
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


std::pair<std::string, SignalType> getObjective(std::unique_ptr<Circuit>& aCircuit){
    if ((*aCircuit->theCircuitState)[aCircuit->theFaultLocation] == SignalType::X){
        SignalType mySAObjective = (aCircuit->theFaultValue == SignalType::D) ? SignalType::ONE : SignalType::ZERO;
        return std::pair<std::string, SignalType>(aCircuit->theFaultLocation, mySAObjective);
    }
    std::string myDFrontierGate = *(aCircuit->theDFrontier->begin());
    for (auto& myDFrontierGateInput : (*aCircuit->theCircuit)[myDFrontierGate]->inputs){
        if ((*aCircuit->theCircuitState)[myDFrontierGateInput] == SignalType::X){
            return std::pair<std::string, SignalType>(myDFrontierGateInput, getNonControllingValue((*aCircuit->theCircuit)[myDFrontierGate]->gateType));
        }
    }
    std::cout << "Error: Unable to create objective when it should have been possible" << std::endl;
    return std::pair<std::string, SignalType>("", SignalType::X);
}


std::pair<std::string, SignalType> doBacktrace(std::unique_ptr<Circuit>& aCircuit, std::pair<std::string, SignalType> anObjective){
    std::string myBacktraceSignal = anObjective.first;
    SignalType myBacktraceValue = anObjective.second;

    while (std::ranges::find(*(aCircuit->theCircuitInputs), myBacktraceSignal) == aCircuit->theCircuitInputs->end()){
        std::unique_ptr<Gate>& myGate = (*aCircuit->theCircuit)[myBacktraceSignal];
        bool myGateBubble = (myGate->gateType == "NAND") || (myGate->gateType == "nand") || (myGate->gateType == "NOR") || (myGate->gateType == "nor") || (myGate->gateType == "XNOR") || (myGate->gateType == "xnor") || (myGate->gateType == "NOT") || (myGate->gateType == "not");

        std::string myBacktraceSignalPrev = myBacktraceSignal; // DEBUG code
        for (auto& myBacktraceGateInput : (*aCircuit->theCircuit)[myBacktraceSignal]->inputs){
            if ((*aCircuit->theCircuitState)[myBacktraceGateInput] == SignalType::X){
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


std::unique_ptr<std::unordered_map<std::string, SignalType>> runPODEMRecursive(std::unique_ptr<Circuit>& aCircuit){

    if (errorAtPO(aCircuit)){
        return aCircuit->getCurrCircuitInputValues();
    }
    if (aCircuit->theDFrontier->empty() && !((*aCircuit->theCircuitState)[aCircuit->theFaultLocation] == SignalType::X)){
        return NULL;
    }
    std::pair<std::string, SignalType> myObjective = getObjective(aCircuit);

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;


    // START OMP implementation
    // std::array<std::unique_ptr<std::unordered_map<std::string, SignalType>>, 2> myPODEMResults;
    // #pragma omp parallel num_threads(2)
    // {
    //     int thread_num = omp_get_thread_num();
    //     if (thread_num == 0) {
    //         myDecision.second = SignalType::ZERO;
    //     } else {
    //         myDecision.second = SignalType::ONE;
    //     }
    //     Circuit myCircuit = *aCircuit;
    //     myCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    //     myPODEMResults[thread_num] = runPODEMRecursive(myCircuit);
    // }

    // if (myPODEMResults[0] != NULL) {
    //     return std::move(myPODEMResults[0]);
    // } else if (myPODEMResults[1] != NULL) {
    //     return std::move(myPODEMResults[1]);
    // } else {
    //     return NULL;
    // }
    // END OMP implementation

    aCircuit->setAndImplyCircuitInput(myDecision.first, myDecision.second);
    std::unique_ptr<std::unordered_map<std::string, SignalType>> myPODEMResult = runPODEMRecursive(aCircuit);
    if(myPODEMResult != NULL){
        return myPODEMResult;
    }

    myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
    aCircuit->setAndImplyCircuitInput(myDecision.first, myDecision.second);
    myPODEMResult = runPODEMRecursive(aCircuit);
    if(myPODEMResult != NULL){
        return myPODEMResult;
    }
    aCircuit->setAndImplyCircuitInput(myDecision.first, SignalType::X);
    return NULL;
}


std::unique_ptr<std::unordered_map<std::string, SignalType>> startPODEM(std::unique_ptr<Circuit>& aCircuit, std::pair<std::string, SignalType> anSSLFault){
    std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = std::make_unique<std::unordered_map<std::string, SignalType>>();

    aCircuit->setCircuitFault(anSSLFault.first, anSSLFault.second);
    aCircuit->resetCircuit();
    return runPODEMRecursive(aCircuit);
}


std::unique_ptr<std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>> runATPG(std::unique_ptr<Circuit>& aCircuit){

    double myTotalComputationTime = 0.0;

    // Unique_ptr to vector of results for each SSL fault ATPG
    // Each result entry consists of: | SSL fault (pair of string and SignalType) | Computation type (double) | Generated Test Vector (Unordered map of input signal names and values) - return empty map if SSL fault undetectable |
    std::unique_ptr<std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>> myATPGData = std::make_unique<std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>>();

    std::unique_ptr<std::vector<std::pair<std::string, SignalType>>> mySSLFaults = std::make_unique<std::vector<std::pair<std::string, SignalType>>>();

    std::unique_ptr<std::vector<std::string>> myTest = std::make_unique<std::vector<std::string>>();
    for (auto& mySignalPair : *(aCircuit->theCircuit)){
        mySSLFaults->push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D));
        mySSLFaults->push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D_b));
    }

    std::size_t myNumFaults = mySSLFaults->size();

    while (!mySSLFaults->empty()){
        std::cout << "\nProgress: " << (myNumFaults - mySSLFaults->size()) << " / " << myNumFaults << " faults complete" << std::endl;
        std::pair<std::string, SignalType> myTargetSSLFault = mySSLFaults->back();

        std::cout << "Info: Running PODEM to detect fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;

        const auto mySingleSSLATPGStartTime = std::chrono::steady_clock::now();
        std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = startPODEM(aCircuit, myTargetSSLFault);
        const auto mySingleSSLATPGTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mySingleSSLATPGStartTime).count();
        myTotalComputationTime += mySingleSSLATPGTime;

        if (myTestVector != NULL){
            myATPGData->push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, *myTestVector));

            std::cout << "\n--- Found test vector for signal " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << " ---" << std::endl;
            for (auto& [myTestVectorInputSignal, myTestVectorInputValue] : *myTestVector){
                std::cout << std::setw(30) << myTestVectorInputSignal << ": " << getSignalStateString(myTestVectorInputValue) << std::endl;
            }

        } else {
            myATPGData->push_back(std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>(myTargetSSLFault, mySingleSSLATPGTime, std::unordered_map<std::string, SignalType>()));
            std::cout << "Info: Unable to generate test vector for fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;
        }

        std::cout << "Computation time for single SSL fault ATPG (sec): " << std::fixed << std::setprecision(10) << mySingleSSLATPGTime << '\n';
        mySSLFaults->pop_back();
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

    std::unique_ptr<std::vector<std::tuple<std::pair<std::string, SignalType>, double, std::unordered_map<std::string, SignalType>>>> myATPGData;

    myATPGData = runATPG(myCircuit);

    for (auto& mySSLTestResult : *myATPGData) {
        std::cout << std::get<0>(mySSLTestResult).first << "," << (std::get<0>(mySSLTestResult).second == SignalType::D ? '0' : '1') << "," << std::get<1>(mySSLTestResult) << "," << (!std::get<2>(mySSLTestResult).empty()) << std::endl;
    }

    return 0;
}