#include "cframe.h"
#include "fframe.h"


// Iterate through existing parsed circuit structure and populate CUDA-friendly data structures
void createCircuitStructure(std::shared_ptr<CudaGate[]> aCircuitStructure, Circuit& aCircuit, std::set<std::string> aCircuitMapping) {

    for (auto& [myCircuitSignal, myCircuitGate] : aCircuit.theCircuit){
        #ifdef DEBUG
        std::cout << "Debug: Processing: " << myCircuitSignal << " | Fanin Size: " << myCircuitGate.inputs.size() << " | Fanout Size: " << myCircuitGate.outputs.size() << std::endl;
        #endif

        // Memory allocation checks
        if (myCircuitGate.inputs.size() > MAX_FANIN_SIZE){
            std::cout << "MAX_FANIN_SIZE = " << MAX_FANIN_SIZE << std::endl;
            std::cout << "Fatal Error: Not enough space allocated to support fanin of size " << myCircuitGate.inputs.size() << std::endl;
            return;
        }
        if (myCircuitGate.outputs.size() > MAX_FANOUT_SIZE){
            std::cout << "MAX_FANOUT_SIZE = " << MAX_FANOUT_SIZE << std::endl;
            std::cout << "Fatal Error: Not enough space allocated to support fanout of size " << myCircuitGate.outputs.size() << std::endl;
            return;
        }

        // Perform mapping of signals and gates (string to array index)
        int myMappedSignal = getSignalMapping(aCircuitMapping, myCircuitSignal);

        if (myCircuitGate.gateType == "BUFF" || myCircuitGate.gateType == "buff"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::BUFF;
        } else if (myCircuitGate.gateType == "NOT" || myCircuitGate.gateType == "not"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::NOT;
        } else if (myCircuitGate.gateType == "AND" || myCircuitGate.gateType == "and"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::AND;
        } else if (myCircuitGate.gateType == "NAND" || myCircuitGate.gateType == "nand"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::NAND;
        } else if (myCircuitGate.gateType == "OR" || myCircuitGate.gateType == "or"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::OR;
        } else if (myCircuitGate.gateType == "NOR" || myCircuitGate.gateType == "nor"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::NOR;
        } else if (myCircuitGate.gateType == "XOR" || myCircuitGate.gateType == "xor"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::XOR;
        } else if (myCircuitGate.gateType == "XNOR" || myCircuitGate.gateType == "xnor"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::XNOR;
        } else if (myCircuitGate.gateType == "INPUT" || myCircuitGate.gateType == "input"){
            aCircuitStructure[myMappedSignal].gateType = CudaGateType::INPUT;
        } else {
            std::cout << "Error: Unable to match gate " << myCircuitGate.gateType << std::endl;
        }

        // Fill in array size parameters
        aCircuitStructure[myMappedSignal].faninSize = myCircuitGate.inputs.size();
        aCircuitStructure[myMappedSignal].fanoutSize = myCircuitGate.outputs.size();

        // Populate fanin signal details
        #ifdef DEBUG
        std::cout << "Debug: Fanin Signals: ";
        #endif
        for (std::size_t myFaninSignalIdx = 0; myFaninSignalIdx < myCircuitGate.inputs.size(); myFaninSignalIdx++){
            aCircuitStructure[myMappedSignal].fanin[myFaninSignalIdx] = getSignalMapping(aCircuitMapping, myCircuitGate.inputs[myFaninSignalIdx]);
            #ifdef DEBUG
            std::cout << aCircuitStructure[myMappedSignal].fanin[myFaninSignalIdx] << " ";
            #endif
        }

        // Populate fanout signal details
        #ifdef DEBUG
        std::cout << "\nDebug: Fanout Signals: ";
        #endif
        for (std::size_t myFanoutSignalIdx = 0; myFanoutSignalIdx < myCircuitGate.outputs.size(); myFanoutSignalIdx++){
            aCircuitStructure[myMappedSignal].fanout[myFanoutSignalIdx] = getSignalMapping(aCircuitMapping, myCircuitGate.outputs[myFanoutSignalIdx]);
            #ifdef DEBUG
            std::cout << aCircuitStructure[myMappedSignal].fanout[myFanoutSignalIdx] << " ";
            #endif
        }
        #ifdef DEBUG
        std::cout << std::endl;
        #endif

    }
}


// Helper method to populate CUDA-friendly circuit output data structure
void createCircuitOutputs(std::shared_ptr<int[]> aCircuitOutputs, Circuit& aCircuit, std::set<std::string> aCircuitMapping){
    #ifdef DEBUG
    std::cout << "\nDebug: Populating circuit output array: ";
    #endif
    for (std::size_t myOutputIdx = 0; myOutputIdx < aCircuit.theCircuitOutputs.size(); myOutputIdx++){
        aCircuitOutputs[myOutputIdx] = getSignalMapping(aCircuitMapping, aCircuit.theCircuitOutputs[myOutputIdx]);
        #ifdef DEBUG
        std::cout << aCircuitOutputs[myOutputIdx] << " ";
        #endif
    }
    #ifdef DEBUG
    std::cout << std::endl;
    #endif
}


// Creates set of all existing signals
std::set<std::string> createSignalsSet(Circuit& aCircuit) {
    std::set<std::string> mySignals = std::set<std::string>();
    for (auto& [myCircuitName, myCircuitGate] : aCircuit.theCircuitState) {
        mySignals.insert(myCircuitName);
    }
    return mySignals;
}


// Maps signal name to array index
int getSignalMapping(std::set<std::string> aCircuitMapping, std::string aSignal){
    return std::distance(aCircuitMapping.begin(), aCircuitMapping.find(aSignal));
}
