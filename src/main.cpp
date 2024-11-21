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

    // myCircuit.setCircuitFault("66", SignalType::D);
    // for (std::string& inputSignal : *(myCircuit.theCircuitInputs)){
    //     myCircuit.setAndImplyCircuitInput(inputSignal, rand() % 2 ? SignalType::ONE : SignalType::ZERO);
    // }

    myCircuit->printCircuitState();

    return 0;
}