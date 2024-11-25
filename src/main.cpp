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

    std::cout << "\nInfo: Running PODEM to detect fault: " << anSSLFault.first << " | SA: " << (anSSLFault.second == SignalType::D ? '0' : '1') << std::endl;

    aCircuit->setCircuitFault(anSSLFault.first, anSSLFault.second);
    aCircuit->resetCircuit();
    return runPODEMRecursive(aCircuit);
}


std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> runATPG(std::unique_ptr<Circuit>& aCircuit){
    std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> myTestVectors = std::make_unique<std::vector<std::unordered_map<std::string, SignalType>>>();
    std::unique_ptr<std::vector<std::pair<std::string, SignalType>>> mySSLFaults = std::make_unique<std::vector<std::pair<std::string, SignalType>>>();

    std::unique_ptr<std::vector<std::string>> myTest = std::make_unique<std::vector<std::string>>();
    for (auto& mySignalPair : *(aCircuit->theCircuit)){
        mySSLFaults->push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D));
        mySSLFaults->push_back(std::pair<std::string, SignalType>(mySignalPair.first, SignalType::D_b));
        // break; // Temp: only test on one fault
    }

    while (mySSLFaults->size() > 0){
        std::pair<std::string, SignalType> myTargetSSLFault = mySSLFaults->back();
        std::unique_ptr<std::unordered_map<std::string, SignalType>> myTestVector = startPODEM(aCircuit, myTargetSSLFault);
        if (myTestVector != NULL){
            myTestVectors->push_back(*myTestVector);

            std::cout << "\n--- Found test vector for signal " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << " ---" << std::endl;
            for (auto& [myTestVectorInputSignal, myTestVectorInputValue] : *myTestVector){
                std::cout << std::setw(30) << myTestVectorInputSignal << ": " << getSignalStateString(myTestVectorInputValue) << std::endl;
            }

        } else {
            std::cout << "Info: Unable to generate test vector for fault: " << myTargetSSLFault.first << " | SA: " << (myTargetSSLFault.second == SignalType::D ? '0' : '1') << std::endl;
        }
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

    std::unique_ptr<std::vector<std::unordered_map<std::string, SignalType>>> myTestVectors;

    myTestVectors = runATPG(myCircuit);

    return 0;
}