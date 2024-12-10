#ifndef FAULT_SIM_H
#define FAULT_SIM_H

#include <cstdint>
#include <set>
#include <string>

#include "cframe.h"

#define MAX_FANIN_SIZE 15
#define MAX_FANOUT_SIZE 15

enum class CudaGateType : uint8_t { AND, OR, NOT, XOR, NAND, NOR, BUFF, XNOR, INPUT};

struct CudaGate {
    CudaGateType gateType;
    int faninSize;
    int fanoutSize;
    int fanin[MAX_FANIN_SIZE];
    int fanout[MAX_FANOUT_SIZE];
};

void createCircuitStructure(std::shared_ptr<CudaGate[]> aCircuitStructure, Circuit& aCircuit, std::set<std::string> aCircuitMapping);
void createCircuitOutputs(std::shared_ptr<int[]> aCircuitOutputs, Circuit& aCircuit, std::set<std::string> aCircuitMapping);

std::set<std::string> createSignalsSet(Circuit& aCircuit);

int getSignalMapping(std::set<std::string> aCircuitMapping, std::string aSignal);

#endif