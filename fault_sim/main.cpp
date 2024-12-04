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

    std::shared_ptr<CudaCircuit[]> myCircuitStructure(new CudaCircuit[myCircuitMapping.size()]);
    createCircuitStructure(myCircuitStructure, *myCircuit, myCircuitMapping);

    return 0;
}