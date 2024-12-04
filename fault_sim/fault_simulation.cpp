#include "cframe.h"
#include "fault_simulation.h"

void createCircuitStructure(std::shared_ptr<CudaCircuit[]> aCircuitStructure, Circuit& aCircuit, std::set<std::string> aCircuitMapping) {

}

std::set<std::string> createSignalsSet(Circuit& aCircuit) {
    std::set<std::string> mySignals = std::set<std::string>();
    for (auto& [myCircuitName, myCircuitGate] : aCircuit.theCircuitState) {
        mySignals.insert(myCircuitName);
    }
    return mySignals;
}