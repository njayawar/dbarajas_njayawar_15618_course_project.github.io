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

#include "cframe.h"


std::unique_ptr<std::unordered_map<std::string, SignalType>> runPODEM(std::unique_ptr<Circuit>& aCircuit, std::pair<std::string, SignalType> anSSLFault){
    std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = std::make_unique<std::unordered_map<std::string, SignalType>>();

    std::cout << "Running PODEM to detect fault: " << anSSLFault.first << " | SA: " << anSSLFault.second << std::endl;

    // PODEM algorithm given SSL fault and circuit


    return myTestVector;
}


std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> runATPG(std::unique_ptr<Circuit>& aCircuit){
    std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> myTestVectors = std::make_unique<std::vector<std::unordered_map<std::string, SignalType>>>();
    std::unique_ptr<std::vector<std::pair<std::string, SignalType>>> mySSLFaults = std::make_unique<std::vector<std::pair<std::string, SignalType>>>();

    std::unique_ptr<std::vector<std::string>> myTest = std::make_unique<std::vector<std::string>>();
    for (auto& mySignalPair : *(aCircuit->theCircuit)){
        mySSLFaults->push_back(std::make_pair<std::string, SignalType>(std::string(mySignalPair.first), SignalType::ONE));
        mySSLFaults->push_back(std::make_pair<std::string, SignalType>(std::string(mySignalPair.first), SignalType::ZERO));
    }

    while (mySSLFaults->size() > 0){
        std::pair<std::string, SignalType> myTargetSSLFault = mySSLFaults->back();
        myTestVectors->push_back(*runPODEM(aCircuit, myTargetSSLFault));
        mySSLFaults->pop_back();
    }

    return myTestVectors;
}


int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Need to supply 1 input circuit file" << std::endl;
        return -1;
    }

    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(argv[1]);

    myCircuit->setCircuitFault("d_BRANCH0_m", SignalType::D_b);
    myCircuit->setAndImplyCircuitInput("A", SignalType::ZERO);
    myCircuit->setAndImplyCircuitInput("B", SignalType::ONE);
    myCircuit->setAndImplyCircuitInput("C", SignalType::ONE);

    myCircuit->printCircuitState();

    std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> myTestVectors;
    myTestVectors = runATPG(myCircuit);

    myCircuit->resetCircuit();
    myCircuit->printCircuitState();

    return 0;
}