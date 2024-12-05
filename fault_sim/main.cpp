#include <iostream>

#include "cframe.h"
#include "fault_simulation.h"

int main(int argc, char** argv) {

    if (argc != 3) {
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

    int myNumCircuitInputs = myCircuit->theCircuitInputs.size();
    std::shared_ptr<int[]> myCircuitInputs(new int[myNumCircuitInputs]);

    int myCircuitNumOutputs = myCircuit->theCircuitOutputs.size();
    std::shared_ptr<int[]> myCircuitOutputs(new int[myCircuitNumOutputs]);
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
        std::cout << "Input Line Tokens: ";
        std::vector<std::string> myLineTokens = tokenize_line(line);
        for (auto& elem : myLineTokens){
            std::cout << " |" << elem << "| ";
        }
        std::cout << std::endl;

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
            std::cout << "Debug: Populating circuit input array: ";
            for (int myInputIdx = 0; myInputIdx < myNumCircuitInputs; myInputIdx++){

                if (!vectorContains<std::string>(myCircuit->theCircuitInputs, myLineTokens[myInputIdx + 1])){
                    std::cout << "\nError: Invalid input signal in test vector file" << std::endl;
                    return -1;
                }

                myCircuitInputs[myInputIdx] = getSignalMapping(myCircuitMapping, myLineTokens[myInputIdx + 1]);
                std::cout << myCircuitInputs[myInputIdx] << " ";
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

    std::cout << "Debug: Number of test vectors: " << myNumTestVectors << std::endl;

    std::shared_ptr<std::uint8_t[]> myTestVectors(new std::uint8_t[myNumTestVectors * myNumCircuitInputs]);
    for (std::size_t i = 0; i < myBinaryDigits.size(); i++){
        for (std::size_t j = 0; j < myBinaryDigits[i].size(); j++){
            myTestVectors[i*myNumCircuitInputs + j] = myBinaryDigits[i][j];
        }
    }

    for (int i = 0; i < myNumTestVectors; i++){
        std::cout << "Debug: Test Vector " << i << ": ";
        for (int j = 0; j < myNumCircuitInputs; j++){
            std::cout << static_cast<int>(myTestVectors[i*myNumCircuitInputs + j]) << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\nFinished populating CUDA input data structures\n" << std::endl;

    std::shared_ptr<std::uint8_t[]> myDetectedFaults(new std::uint8_t[myCircuitMapping.size()]);

    return 0;
}
