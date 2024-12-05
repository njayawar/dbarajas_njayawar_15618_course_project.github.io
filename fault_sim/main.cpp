#include <iostream>

#include "cframe.h"
#include "fault_simulation.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Need to supply input circuit file and test vectors" << std::endl;
        return -1;
    }

    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(argv[1]);

    std::set<std::string> myCircuitMapping = createSignalsSet(*myCircuit);

    std::cout << "\nDebug: Printing circuit signal mapping" << std::endl;
    for (const auto& myElem : myCircuitMapping) {
        std::cout << std::setw(30) << myElem << ": " << getSignalMapping(myCircuitMapping, myElem) << std::endl;
    }
    std::cout << std::endl;

    std::shared_ptr<CudaGate[]> myCircuitStructure(new CudaGate[myCircuitMapping.size()]);
    createCircuitStructure(myCircuitStructure, *myCircuit, myCircuitMapping);

    int myCircuitNumInputs = myCircuit->theCircuitInputs.size();
    std::shared_ptr<int[]> myCircuitInputs(new int[myCircuitNumInputs]);
    createCircuitInputs(myCircuitInputs, *myCircuit, myCircuitMapping);

    int myCircuitNumOutputs = myCircuit->theCircuitOutputs.size();
    std::shared_ptr<int[]> myCircuitOutputs(new int[myCircuitNumOutputs]);
    createCircuitOutputs(myCircuitOutputs, *myCircuit, myCircuitMapping);

    return 0;
}