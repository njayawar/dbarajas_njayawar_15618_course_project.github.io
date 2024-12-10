#include <iostream>

#include "cframe.h"
#include "fframe.h"
#include "CycleTimer.h"

void cudaFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults);
void printCudaInfo();

void serialFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults);

std::vector<std::string> tokenize_file_name(std::string s) {
    std::ranges::replace(s, '/', ' ');
    std::ranges::replace(s, '.', ' ');

    std::istringstream stream(s);

    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "Need to supply input circuit file and test vectors" << std::endl;
        return -1;
    }

    std::unique_ptr<Circuit> myCircuit = std::make_unique<Circuit>(argv[1]);

    std::set<std::string> myCircuitMapping = createSignalsSet(*myCircuit);

    #ifdef DEBUG
    std::cout << "\nDebug: Printing circuit signal mapping" << std::endl;
    for (const auto& myElem : myCircuitMapping) {
        std::cout << std::setw(30) << myElem << ": " << getSignalMapping(myCircuitMapping, myElem) << std::endl;
    }
    std::cout << std::endl;
    #endif

    std::shared_ptr<CudaGate[]> myCircuitStructure(new CudaGate[myCircuitMapping.size()]);
    createCircuitStructure(myCircuitStructure, *myCircuit, myCircuitMapping);

    #ifdef DEBUG
    std::cout << "Finished structure" << std::endl;
    #endif

    int myNumCircuitInputs = myCircuit->theCircuitInputs.size();
    std::shared_ptr<int[]> myCircuitInputs(new int[myNumCircuitInputs]);

    int myNumCircuitOutputs = myCircuit->theCircuitOutputs.size();
    std::shared_ptr<int[]> myCircuitOutputs(new int[myNumCircuitOutputs]);
    createCircuitOutputs(myCircuitOutputs, *myCircuit, myCircuitMapping);

    std::ifstream myTestVectorsFile;
    myTestVectorsFile.open(argv[2]);

    if (!myTestVectorsFile.is_open()) {
        std::cout << "Error opening file " << argv[2] << std::endl;
        return -1;
    }

    int myNumTestVectors = 0;

    std::vector<std::vector<std::uint8_t>> myBinaryDigits = std::vector<std::vector<std::uint8_t>>();
    std::string line;
    while (std::getline(myTestVectorsFile, line)) {
        if (line.starts_with('#') || line.starts_with('$') || string_is_whitespace(line)) {
            continue;
        }
        std::vector<std::string> myLineTokens = tokenize_line(line);
        #ifdef DEBUG
        std::cout << "Input Line Tokens: ";
        for (auto& elem : myLineTokens){
            std::cout << " |" << elem << "| ";
        }
        std::cout << std::endl;
        #endif

        if (myLineTokens[0] == "VECTORS" || myLineTokens[0] == "vectors"){
            if (myLineTokens.size() != 2){
                std::cout << "Error: Invalid number of tokens in test vector file" << std::endl;
                return -1;
            }
            myNumTestVectors = std::stoi(myLineTokens[1]);
        } else if (myLineTokens[0] == "INPUTS" || myLineTokens[0] == "inputs"){
            if (myLineTokens.size() != static_cast<std::size_t>(1 + myNumCircuitInputs)){
                std::cout << "Error: Invalid number of tokens in test vector file" << std::endl;
                return -1;
            }
            #ifdef DEBUG
            std::cout << "Debug: Populating circuit input array: ";
            #endif
            for (int myInputIdx = 0; myInputIdx < myNumCircuitInputs; myInputIdx++){

                if (!vectorContains<std::string>(myCircuit->theCircuitInputs, myLineTokens[myInputIdx + 1])){
                    std::cout << "\nError: Invalid input signal in test vector file" << std::endl;
                    return -1;
                }

                myCircuitInputs[myInputIdx] = getSignalMapping(myCircuitMapping, myLineTokens[myInputIdx + 1]);
                #ifdef DEBUG
                std::cout << myCircuitInputs[myInputIdx] << " ";
                #endif
            }
            std::cout << std::endl;
        } else if (myLineTokens[0].find(':') != std::string::npos){
            if (myLineTokens.size() != 2){
                std::cout << "Error: Invalid number of tokens in test vector file" << std::endl;
                return -1;
            }

            std::vector<std::uint8_t> myTestBinaryDigits = std::vector<std::uint8_t>();
            for (char digit : myLineTokens[1]) {
                if ((digit - '0' != 0) && (digit - '0' != 1)){
                    std::cout << "Error: Test vector contains invalid digits" << std::endl;
                    return -1;
                }
                myTestBinaryDigits.push_back(digit - '0');
            }

            if (myTestBinaryDigits.size() != static_cast<std::size_t>(myNumCircuitInputs)){
                std::cout << "Error: Invalid number inputs in test vector" << std::endl;
                return -1;
            }

            myBinaryDigits.push_back(myTestBinaryDigits);

        }

    }

    if (myBinaryDigits.size() != static_cast<std::size_t>(myNumTestVectors)){
        std::cout << "Error: Invalid number of test vectors" << std::endl;
        return -1;
    }

    #ifdef DEBUG
    std::cout << "Debug: Number of test vectors: " << myNumTestVectors << std::endl;
    #endif

    std::shared_ptr<std::uint8_t[]> myTestVectors(new std::uint8_t[myNumTestVectors * myNumCircuitInputs]);
    for (std::size_t i = 0; i < myBinaryDigits.size(); i++){
        for (std::size_t j = 0; j < myBinaryDigits[i].size(); j++){
            myTestVectors[i*myNumCircuitInputs + j] = myBinaryDigits[i][j];
        }
    }

    #ifdef DEBUG
    for (int i = 0; i < myNumTestVectors; i++){
        std::cout << "Debug: Test Vector " << i << ": ";
        for (int j = 0; j < myNumCircuitInputs; j++){
            std::cout << static_cast<int>(myTestVectors[i*myNumCircuitInputs + j]) << " ";
        }
        std::cout << std::endl;
    }
    #endif

    std::vector<int> myTraversalOrderVector = std::vector<int>();
    for (int i = 0; i < myNumCircuitInputs; i++){
        myTraversalOrderVector.push_back(myCircuitInputs[i]);
    }
    while (myTraversalOrderVector.size() < myCircuitMapping.size()){
        for (auto& mySignal : myCircuitMapping){
            if (!vectorContains<int>(myTraversalOrderVector, getSignalMapping(myCircuitMapping, mySignal))) {
                bool myInputsReady = true;
                for (auto& myFanin : myCircuit->theCircuit[mySignal].inputs) {
                    if (!vectorContains<int>(myTraversalOrderVector, getSignalMapping(myCircuitMapping, myFanin))) {
                        myInputsReady = false;
                    }
                }
                if (myInputsReady) {
                    myTraversalOrderVector.push_back(getSignalMapping(myCircuitMapping, mySignal));
                }
            }
        }
    }

    #ifdef DEBUG
    std::cout << "\nFinished populating CUDA input data structures\n" << std::endl;

    printCudaInfo();
    #endif

    std::shared_ptr<std::uint8_t[]> myDetectedFaults(new std::uint8_t[myCircuitMapping.size() * 2 * myNumTestVectors]);
    #ifdef DEBUG
    std::cout << "\nStarting Parallel Fault Simulation Timer" << std::endl;
    #endif
    double myParallelStartTime = CycleTimer::currentSeconds();
    cudaFaultSim(myCircuitMapping.size(), myCircuitStructure.get(), myTraversalOrderVector.data(), myNumCircuitInputs, myCircuitInputs.get(), myNumCircuitOutputs, myCircuitOutputs.get(), myNumTestVectors, myTestVectors.get(), myDetectedFaults.get());
    double myParallelEndTime = CycleTimer::currentSeconds();
    #ifdef DEBUG
    std::cout << "Ending Parallel Fault Simulation Timer" << std::endl;
    #endif

    #ifdef DEBUG
    std::cout << "\n--------------------- Parallel Fault Simulation Results ---------------------" << std::endl;
    for (int myVectorIdx = 0; myVectorIdx < myNumTestVectors; myVectorIdx++) {
        std::cout << "Test Vector: " << myVectorIdx << std::endl;
        int myFaultCnt = 0;
        for (std::size_t myFaultIdx = 0; myFaultIdx < myCircuitMapping.size() * 2; myFaultIdx+=2) {
            int mySA0Idx = (myVectorIdx * myCircuitMapping.size() * 2) + myFaultIdx;
            int mySA1Idx = (myVectorIdx * myCircuitMapping.size() * 2) + myFaultIdx + 1;
            // std::cout << std::setw(30) << (*std::next(myCircuitMapping.begin(), (myFaultIdx/2))) << " / 0 fault detected: " << static_cast<int>(myDetectedFaults[mySA0Idx]) << std::endl;
            // std::cout << std::setw(30) << (*std::next(myCircuitMapping.begin(), (myFaultIdx/2))) << " / 1 fault detected: " << static_cast<int>(myDetectedFaults[mySA1Idx]) << std::endl;
            myFaultCnt += static_cast<int>(myDetectedFaults[mySA0Idx]) + static_cast<int>(myDetectedFaults[mySA1Idx]);
        }
        std::cout << "Total faults detected: " << myFaultCnt << " / " << (myCircuitMapping.size() * 2) << std::endl;
        std::cout << std::endl;
    }
    #endif

    #ifdef DEBUG
    std::cout << "\nStarting Serial Fault Simulation Timer" << std::endl;
    #endif
    double mySerialStartTime = CycleTimer::currentSeconds();
    serialFaultSim(myCircuitMapping.size(), myCircuitStructure.get(), myTraversalOrderVector.data(), myNumCircuitInputs, myCircuitInputs.get(), myNumCircuitOutputs, myCircuitOutputs.get(), myNumTestVectors, myTestVectors.get(), myDetectedFaults.get());
    double mySerialEndTime = CycleTimer::currentSeconds();
    #ifdef DEBUG
    std::cout << "Ending Serial Fault Simulation Timer" << std::endl;
    #endif

    #ifdef DEBUG
    std::cout << "\n--------------------- Serial Fault Simulation Results ---------------------" << std::endl;
    for (int myVectorIdx = 0; myVectorIdx < myNumTestVectors; myVectorIdx++) {
        std::cout << "Test Vector: " << myVectorIdx << std::endl;
        int myFaultCnt = 0;
        for (std::size_t myFaultIdx = 0; myFaultIdx < myCircuitMapping.size() * 2; myFaultIdx+=2) {
            int mySA0Idx = (myVectorIdx * myCircuitMapping.size() * 2) + myFaultIdx;
            int mySA1Idx = (myVectorIdx * myCircuitMapping.size() * 2) + myFaultIdx + 1;
            // std::cout << std::setw(30) << (*std::next(myCircuitMapping.begin(), (myFaultIdx/2))) << " / 0 fault detected: " << static_cast<int>(myDetectedFaults[mySA0Idx]) << std::endl;
            // std::cout << std::setw(30) << (*std::next(myCircuitMapping.begin(), (myFaultIdx/2))) << " / 1 fault detected: " << static_cast<int>(myDetectedFaults[mySA1Idx]) << std::endl;
            myFaultCnt += static_cast<int>(myDetectedFaults[mySA0Idx]) + static_cast<int>(myDetectedFaults[mySA1Idx]);
        }
        std::cout << "Total faults detected: " << myFaultCnt << " / " << (myCircuitMapping.size() * 2) << std::endl;
        std::cout << std::endl;
    }
    #endif

    std::vector<std::string> myTokenizedCircuitFileName = tokenize_file_name(argv[1]);

    std::string myBenchName = myTokenizedCircuitFileName[myTokenizedCircuitFileName.size()-2];

    std::cout << "Serial Time (s)   " << std::setw(6) << myBenchName << " " << std::setw(3) << myNumTestVectors << " : " << (mySerialEndTime - mySerialStartTime) << std::endl;
    std::cout << "Parallel Time (s) " << std::setw(6) << myBenchName << " " << std::setw(3) << myNumTestVectors << " : " << (myParallelEndTime - myParallelStartTime) << std::endl << std::endl;

    std::ofstream myOutputFile("benchmarks_results.txt", std::ios::app);
    if (!myOutputFile) {
        std::cout << "Error: Could not open the file" << std::endl;
    }

    myOutputFile << "Serial   " << std::setw(6) << myBenchName << " " << std::setw(3) << myNumTestVectors << " : " << (mySerialEndTime - mySerialStartTime) << std::endl;
    myOutputFile << "Parallel " << std::setw(6) << myBenchName << " " << std::setw(3) << myNumTestVectors << " : " << (myParallelEndTime - myParallelStartTime) << std::endl << std::endl;

    myOutputFile.close();

    return 0;
}
