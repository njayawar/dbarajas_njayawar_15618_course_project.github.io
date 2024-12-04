#ifndef FAULT_SIM_H
#define FAULT_SIM_H

#include <cstdint>
#include <set>
#include <string>

#include "cframe.h"

#define MAX_FANIN_SIZE 10
#define MAX_FANOUT_SIZE 10

enum class CudaGateType : uint8_t { AND, OR, NOT, XOR, NAND, NOR, BUFF, XNOR };

struct CudaCircuit {
    CudaGateType t;
    int fanin_size;
    int fanout_size;
    int fanin[MAX_FANIN_SIZE];
    int fanout[MAX_FANOUT_SIZE];
};

void createCircuitStructure(std::shared_ptr<CudaCircuit[]> aCircuitStructure, Circuit& aCircuit, std::set<std::string> aCircuitMapping);

std::set<std::string> createSignalsSet(Circuit& aCircuit);

#endif