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
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include <unistd.h>

typedef enum SignalType {
    ZERO = 0,
    ONE = 1,
    D = 2,
    D_b = 3,
    X = 4
} SignalType;

const std::unordered_map<SignalType, std::string> signalTypeNames = {
    {SignalType::ZERO, "ZERO"},
    {SignalType::ONE, "ONE"},
    {SignalType::D, "D"},
    {SignalType::D_b, "D_b"},
    {SignalType::X, "X"}
};

typedef enum ImplyReturnType {
    ERROR,
    MASKED,
    NORMAL,
    ACTIVATED,
    DETECTED
} ImplyReturnType;

const std::unordered_map<ImplyReturnType, std::string> returnCodeNames = {
    {ImplyReturnType::ERROR, "ERROR"},
    {ImplyReturnType::MASKED, "MASKED"},
    {ImplyReturnType::NORMAL, "NORMAL"},
    {ImplyReturnType::ACTIVATED, "ACTIVATED"},
    {ImplyReturnType::DETECTED, "DETECTED"}
};

typedef enum GateType {
    AND,
    OR,
    NOT,
    XOR,
    NAND,
    NOR,
    BUFF,
    XNOR
} GateType;

std::string getSignalStateString(SignalType aSignal);

struct Gate {
    std::string gateType;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    Gate(std::string gateType, std::vector<std::string> inputs, std::vector<std::string> outputs) : gateType(gateType), inputs(inputs), outputs(outputs) {}
    Gate() : gateType(""), inputs(), outputs() {}
};

void printGate(Gate aGate);

const SignalType opAND [5][5] = {
    {SignalType::ZERO, SignalType::ZERO, SignalType::ZERO, SignalType::ZERO, SignalType::ZERO},
    {SignalType::ZERO, SignalType::ONE,  SignalType::D,    SignalType::D_b,  SignalType::X},
    {SignalType::ZERO, SignalType::D,    SignalType::D,    SignalType::ZERO, SignalType::X},
    {SignalType::ZERO, SignalType::D_b,  SignalType::ZERO, SignalType::D_b,  SignalType::X},
    {SignalType::ZERO, SignalType::X,    SignalType::X,    SignalType::X,    SignalType::X}
};

const SignalType opOR [5][5] = {
    {SignalType::ZERO, SignalType::ONE,  SignalType::D,    SignalType::D_b,  SignalType::X},
    {SignalType::ONE,  SignalType::ONE,  SignalType::ONE,  SignalType::ONE,  SignalType::ONE},
    {SignalType::D,    SignalType::ONE,  SignalType::D,    SignalType::ONE,  SignalType::X},
    {SignalType::D_b,  SignalType::ONE,  SignalType::ONE,  SignalType::D_b,  SignalType::X},
    {SignalType::X,    SignalType::ONE,  SignalType::X,    SignalType::X,    SignalType::X}
};

const SignalType opXOR [5][5] = {
    {SignalType::ZERO, SignalType::ONE,  SignalType::D,    SignalType::D_b,  SignalType::X},
    {SignalType::ONE,  SignalType::ZERO, SignalType::D_b,  SignalType::D,    SignalType::X},
    {SignalType::D,    SignalType::D_b,  SignalType::ZERO, SignalType::ONE,  SignalType::X},
    {SignalType::D_b,  SignalType::D,    SignalType::ONE,  SignalType::ZERO, SignalType::X},
    {SignalType::X,    SignalType::X,    SignalType::X,    SignalType::X,    SignalType::X}
};

const SignalType opNOT [5] = {SignalType::ONE, SignalType::ZERO, SignalType::D_b, SignalType::D, SignalType::X};

template <typename T>
bool vectorContains(std::vector<T> aVector, T aValue){
    return (std::ranges::find(aVector, aValue) != aVector.end());
}

class Circuit {
public:
    Circuit();
    Circuit(const std::string aCircuitFileString);
    Circuit(const Circuit& other);
    Circuit& operator=(const Circuit& other);
    ~Circuit();

    bool setCircuitFault(std::string aFaultLocation, SignalType aFaultValue);
    ImplyReturnType setAndImplyCircuitInput(std::string anInput, SignalType aValue);
    void resetCircuit();
    std::unordered_map<std::string, SignalType> getCurrCircuitInputValues();

    std::unordered_map<std::string, Gate> theCircuit;
    std::unordered_map<std::string, SignalType> theCircuitState;

    std::vector<std::string> theCircuitInputs;
    std::vector<std::string> theCircuitOutputs;
    std::vector<std::string> theCircuitSignals;

    std::string theFaultLocation;
    SignalType theFaultValue;

    std::unordered_set<std::string> theDFrontier;

    void printCircuitState();
    void printDFrontierGates();
    void printCircuit();

private:
    std::string theCircuitFileString;
    std::ifstream theCircuitFile;

    std::unordered_map<std::string, std::vector<std::string>> get_wire_cnt(std::ifstream& file);
    void populate_circuit(std::ifstream& aCircuitFile, std::unordered_map<std::string, std::vector<std::string>> aWireCnt);

    ImplyReturnType evaluateGateRecursive(std::string aGate);
};
