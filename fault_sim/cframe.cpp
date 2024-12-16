#include <sstream>

#include "cframe.h"


bool string_is_whitespace(std::string& s) {
    bool is_spaces = std::all_of(s.begin(), s.end(), [](char c) {
        return std::isspace(c);
    });
    return is_spaces;
}


// Replaces the brackets in a string with whitespace
void remove_brackets(std::string& s) {
    std::ranges::replace(s, '(', ' ');
    std::ranges::replace(s, ')', ' ');
}


// Replaces the commas in a string with whitespace
void remove_commas(std::string& s) {
    std::ranges::replace(s, ',', ' ');
}


// Replaces the equals signs in a string with whitespace
void remove_equals_sign(std::string& s) {
    std::ranges::replace(s, '=', ' ');
}


// Splits an ISCAS line and returns vector of tokens consisting
// of wire names and the type of gate
std::vector<std::string> tokenize_line(std::string s) {
    remove_brackets(s);
    remove_commas(s);
    remove_equals_sign(s);

    std::istringstream stream(s);

    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}


// Gets the wire name of a string
static std::vector<std::string> get_wire_names(std::string line) {
    std::vector<std::string> tokens = tokenize_line(line);

    if (tokens[0] == "INPUT" ||
        tokens[0] == "input" ||
        tokens[0] == "OUTPUT" ||
        tokens[0] == "output") {
        return std::vector<std::string>({tokens[1]});
    } else {
        return std::vector<std::string>(tokens.begin() + 2, tokens.end());
    }
}


// Scans input file and takes the count of each wire
std::unordered_map<std::string, std::vector<std::string>> Circuit::get_wire_cnt(std::ifstream& file) {
    // Initializes map
    std::unordered_map<std::string, std::vector<std::string>> wire_cnt;

    // Parses file line by line
    std::string line;
    while (std::getline(file, line)) {
        // Skips line if its a comment or end-of-file or empty
        if (line.starts_with('#') || line.starts_with('$') || string_is_whitespace(line)) {
            continue;
        }

        // Gets the name of each wire
        std::vector<std::string> wire_names = get_wire_names(line);
        std::string myOutputWireName = tokenize_line(line)[0];

        // Sets value of each wire
        for (auto wire_name : wire_names) {
            if (line.starts_with("INPUT") || line.starts_with("input")) {
                theCircuitInputs.push_back(wire_name);
            } else if (line.starts_with("OUTPUT") || line.starts_with("output")) {
                theCircuitOutputs.push_back(wire_name);
            } else {
                if (wire_cnt.contains(wire_name)) {
                    wire_cnt[wire_name].push_back(myOutputWireName);
                } else {
                    wire_cnt[wire_name] = std::vector<std::string>({ myOutputWireName });
                }
            }
        }
    }

    return wire_cnt;
}


std::string getSignalStateString(SignalType aSignal){
    return signalTypeNames.find(aSignal)->second;
}


std::string getReturnCodeString(ImplyReturnType aReturnType){
    return returnCodeNames.find(aReturnType)->second;
}


// Parses circuit from input file and populates data structures
void Circuit::populate_circuit(std::ifstream& aCircuitFile, std::unordered_map<std::string, std::vector<std::string>> aWireCnt) {
    std::string myLine;
    while (std::getline(aCircuitFile, myLine)) {
        // Skips line if its a comment or end-of-file or empty
        if (myLine.starts_with('#') || myLine.starts_with('$') || string_is_whitespace(myLine)) {
            continue;
        }

        auto myInputWireNames = get_wire_names(myLine);

        if (!myLine.starts_with("OUTPUT") && !myLine.starts_with("output")) {

            std::vector<std::string> myTokens = tokenize_line(myLine);
            std::string myGateOutputWire;
            std::string myGateType;

            // Process circuit input
            if (myLine.starts_with("INPUT") || myLine.starts_with("input")) {
                myGateType = "INPUT";
                myGateOutputWire = myTokens[1];
            } else {
                myGateType = myTokens[1];
                myGateOutputWire = myTokens[0];
            }

            // std::cout << "Processing: " << myLine << " | " << myGateOutputWire << std::endl;

            Gate myGate = {myGateType, std::vector<std::string>(), std::vector<std::string>()};

            // Process regular gate
            if (!myLine.starts_with("INPUT") && !myLine.starts_with("input")) {
                for (auto& myInputWire : myInputWireNames) {
                    if (aWireCnt[myInputWire].size() > 1) {
                        // Input is a branch
                        std::size_t myBranchIter = 0;
                        std::string myInputWireBranch = myInputWire + "_BRANCH" + std::to_string(myBranchIter) + "_" + myGateOutputWire;
                        while (std::ranges::find(myGate.inputs, myInputWireBranch) != myGate.inputs.end()){
                            myBranchIter++;
                            myInputWireBranch = myInputWire + "_BRANCH" + std::to_string(myBranchIter) + "_" + myGateOutputWire;
                        }
                        myGate.inputs.push_back(myInputWireBranch);
                    } else {
                        myGate.inputs.push_back(myInputWire);
                    }
                }
            }

            // Add additional gates if output is a stem to a branch
            if (aWireCnt[myGateOutputWire].size() > 1) {
                for (auto& myOutputBranch : aWireCnt[myGateOutputWire]) {
                    std::size_t myBranchIter = 0;
                    std::string myOutputBranchName = myGateOutputWire + "_BRANCH" + std::to_string(myBranchIter) + "_" + myOutputBranch;
                    while (std::ranges::find(myGate.outputs, myOutputBranchName) != myGate.outputs.end()){
                        myBranchIter++;
                        myOutputBranchName = myGateOutputWire + "_BRANCH" + std::to_string(myBranchIter) + "_" + myOutputBranch;
                    }
                    myGate.outputs.push_back(myOutputBranchName);
                    Gate myBranchGate = {"BUFF", std::vector<std::string>({myGateOutputWire}), std::vector<std::string>({myOutputBranch})};
                    theCircuit[myOutputBranchName] = myBranchGate;
                    theCircuitState[myOutputBranchName] = SignalType::X;
                    theCircuitSignals.push_back(myOutputBranchName);
                }
            } else {
                for (auto& myOutputBranch : aWireCnt[myGateOutputWire]) {
                    myGate.outputs.push_back(myOutputBranch);
                }
            }

            theCircuit[myGateOutputWire] = myGate;
            theCircuitState[myGateOutputWire] = SignalType::X;
            theCircuitSignals.push_back(myGateOutputWire);
        }
    }
}


Circuit::Circuit() {};


// Upon construction, begin parsing and populate data structures
Circuit::Circuit(const std::string aCircuitFileString) :
        theFaultLocation(""),
        theFaultValue(SignalType::X),
        theCircuitFileString(aCircuitFileString) {

    theCircuit = std::unordered_map<std::string, Gate>();
    theCircuitState = std::unordered_map<std::string, SignalType>();

    theCircuitInputs = std::vector<std::string>();
    theCircuitOutputs = std::vector<std::string>();
    theCircuitSignals = std::vector<std::string>();

    theDFrontier = std::unordered_set<std::string>();

    theCircuitFile.open(theCircuitFileString);

    if (!theCircuitFile.is_open()) {
        std::cout << "Error opening file " << theCircuitFileString << std::endl;
        return;
    }

    // First pass - determine any stem and branches for future reference
    auto myWireCnt = get_wire_cnt(theCircuitFile);

    theCircuitFile.clear();
    theCircuitFile.seekg(0);

    // Second pass - populate data structures
    populate_circuit(theCircuitFile, myWireCnt);

    #ifdef DEBUG
    std::cout << "\n\n----- Printing Populated Circuit -----" << std::endl;

    for (auto& [myWireName, myGate] : theCircuit){
        std::cout << "\n--- Gate: " << myWireName << " | Type: " << myGate.gateType << " ---" << std::endl;
        std::cout << "Inputs: ";
        for (auto& myGateInput : myGate.inputs) {
            std::cout << myGateInput << " ";
        }
        std::cout << std::endl << "Outputs: ";
        for (auto& myGateOutput : myGate.outputs) {
            std::cout << myGateOutput + " ";
        }
        std::cout << std::endl;
    }

    printCircuitState();
    #endif

}


Circuit& Circuit::operator=(const Circuit& other) {
    theCircuit = other.theCircuit;
    theCircuitState = other.theCircuitState;
    theCircuitInputs = other.theCircuitInputs;
    theCircuitOutputs = other.theCircuitOutputs;
    theCircuitSignals = other.theCircuitSignals;
    theFaultLocation = other.theFaultLocation;
    theFaultValue = other.theFaultValue;
    theDFrontier = other.theDFrontier;
    theCircuitFileString = other.theCircuitFileString;
    return *this;
}


Circuit::Circuit(const Circuit& other) :
    theCircuit(other.theCircuit),
    theCircuitState(other.theCircuitState),
    theCircuitInputs(other.theCircuitInputs),
    theCircuitOutputs(other.theCircuitOutputs),
    theCircuitSignals(other.theCircuitSignals),
    theFaultLocation(other.theFaultLocation),
    theFaultValue(other.theFaultValue),
    theDFrontier(other.theDFrontier),
    theCircuitFileString(other.theCircuitFileString)
{}


void printGate(Gate aGate) {
    std::cout << aGate.gateType << std::endl;
    for (auto& anInput : aGate.inputs) {
        std::cout << anInput << ' ';
    }
    std::cout << std::endl;
    for (auto& anOutput : aGate.outputs) {
        std::cout << anOutput << ' ';
    }
    std::cout << std::endl;
}

void Circuit::printCircuit() {}


void Circuit::printCircuitState(){
    std::cout << "\n\n----- Printing Circuit State -----" << std::endl;
    for (auto& [myWireName, myWireState] : theCircuitState){
        std::cout << std::setw(30) << myWireName << ": " << getSignalStateString(myWireState) << std::endl;
    }
    std::cout << std::endl;
}


void Circuit::printDFrontierGates(){
    std::cout << "\n\n----- Printing DFrontier Gates -----" << std::endl;
    for (auto& dFrontierGate : theDFrontier){
        std::cout << dFrontierGate << std::endl;
    }
    std::cout << std::endl;
}


Circuit::~Circuit() {
    if (theCircuitFile.is_open()) {
        theCircuitFile.close();
    }
}


bool Circuit::setCircuitFault(std::string aFaultLocation, SignalType aFaultValue){

    if (aFaultValue != SignalType::D && aFaultValue != SignalType::D_b) {
        std::cout << "Error: Fault value must of type D or D_b" << std::endl;
        return false;
    }

    if (vectorContains<std::string>(theCircuitSignals, aFaultLocation)){
        theFaultLocation = aFaultLocation;
        theFaultValue = aFaultValue;
        #ifdef DEBUG
        std::cout << "Info: Set fault value " << getSignalStateString(aFaultValue) << " to signal " << aFaultLocation<< std::endl;
        #endif
        return true;
    } else {
        std::cout << "Error: Unable to set fault value " << getSignalStateString(aFaultValue) << " to signal " << aFaultLocation<< std::endl;
        return false;
    }
}


// Sets a circuit input and propogates the effect of the assignment all the way down the circuit
ImplyReturnType Circuit::setAndImplyCircuitInput(std::string anInput, SignalType aValue){
    if (aValue != SignalType::ONE && aValue != SignalType::ZERO && aValue != SignalType::X) {
        std::cout << "Error: setAndImply(" << anInput << ", " << getSignalStateString(aValue) << ") | Input value must of type 1 or 0" << std::endl;
        return ImplyReturnType::ERROR;
    }
    if (!vectorContains<std::string>(theCircuitInputs, anInput)){
        std::cout << "Error: setAndImply(" << anInput << ", " << getSignalStateString(aValue) << ") | Signal " << anInput << " is not a valid circuit input" << std::endl;
        return ImplyReturnType::ERROR;
    }

    // std::cout << "Info: Setting and implying value " << getSignalStateString(aValue) << " to signal " << anInput << std::endl;

    ImplyReturnType myReturnCode = ImplyReturnType::NORMAL;

    bool mySignalIsOutputFlag = vectorContains<std::string>(theCircuitOutputs, anInput);

    // If a fault location is seen, override the correct assignment
    if ((anInput == theFaultLocation) && (aValue != SignalType::X)){
        if (theFaultValue == SignalType::D){
            if (aValue == SignalType::ZERO){
                theCircuitState[anInput] = SignalType::ZERO;
                myReturnCode = ImplyReturnType::MASKED;
            } else {
                theCircuitState[anInput] = SignalType::D;
                myReturnCode = (mySignalIsOutputFlag) ? ImplyReturnType::DETECTED : ImplyReturnType::ACTIVATED;
            }
        }
        if (theFaultValue == SignalType::D_b){
            if (aValue == SignalType::ONE){
                theCircuitState[anInput] = SignalType::ONE;
                myReturnCode = ImplyReturnType::MASKED;
            } else {
                theCircuitState[anInput] = SignalType::D_b;
                myReturnCode = (mySignalIsOutputFlag) ? ImplyReturnType::DETECTED : ImplyReturnType::ACTIVATED;
            }
        }
    } else {
        theCircuitState[anInput] = aValue;
    }

    // Recursively perform signal assignment down the circuit
    for (auto& fanoutSignal : theCircuit[anInput].outputs) {
        // std::cout << "Debug: Checking gate " << fanoutSignal << " for update" << std::endl;
        ImplyReturnType myNewReturnCode = evaluateGateRecursive(fanoutSignal);

        if (myReturnCode == ImplyReturnType::MASKED && (myNewReturnCode == ImplyReturnType::DETECTED || myNewReturnCode == ImplyReturnType::ACTIVATED)){
            myReturnCode = ImplyReturnType::ERROR;
        } else {
            if (myReturnCode == ImplyReturnType::ERROR || myNewReturnCode == ImplyReturnType::ERROR){
                myReturnCode = ImplyReturnType::ERROR;
            } else if (myNewReturnCode == ImplyReturnType::DETECTED) {
                myReturnCode = myNewReturnCode;
            } else if (myNewReturnCode == ImplyReturnType::ACTIVATED && myReturnCode != ImplyReturnType::DETECTED) {
                myReturnCode = myNewReturnCode;
            } else if (myNewReturnCode == ImplyReturnType::MASKED){
                if (myReturnCode == ImplyReturnType::DETECTED || myReturnCode == ImplyReturnType::ACTIVATED) {
                    myReturnCode = ImplyReturnType::ERROR;
                } else {
                    myReturnCode = myNewReturnCode;
                }
            }
        }
    }

    // std::cout << "Info: Imply (" << anInput << ", " << getSignalStateString(aValue) << ") returning with code: " << getReturnCodeString(myReturnCode) << std::endl;
    // printCircuitState();

    return myReturnCode;
}


// Recursive call to evaluate the output of a gate and propogate assignemnt through the circuit
ImplyReturnType Circuit::evaluateGateRecursive(std::string aGateName){

    ImplyReturnType myReturnCode = ImplyReturnType::NORMAL;

    Gate aGate = theCircuit[aGateName];
    SignalType myNewSignalValue = theCircuitState[aGate.inputs[0]];
    SignalType myOldSignalValue = theCircuitState[aGateName];

    bool myDInputFlag = theCircuitState[aGate.inputs[0]] == SignalType::D || theCircuitState[aGate.inputs[0]] == SignalType::D_b;

    if (aGate.gateType == "BUFF" || aGate.gateType == "buff"){
        myNewSignalValue = theCircuitState[aGate.inputs[0]];
    } else if (aGate.gateType == "NOT" || aGate.gateType == "not"){
        myNewSignalValue = opNOT[theCircuitState[aGate.inputs[0]]];
    } else if (aGate.gateType == "AND" || aGate.gateType == "and"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opAND[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
    } else if (aGate.gateType == "NAND" || aGate.gateType == "nand"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opAND[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
        myNewSignalValue = opNOT[myNewSignalValue];
    } else if (aGate.gateType == "OR" || aGate.gateType == "or"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opOR[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
    } else if (aGate.gateType == "NOR" || aGate.gateType == "nor"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opOR[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
        myNewSignalValue = opNOT[myNewSignalValue];
    } else if (aGate.gateType == "XOR" || aGate.gateType == "xor"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opXOR[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
    } else if (aGate.gateType == "XNOR" || aGate.gateType == "xnor"){
        for (std::size_t myFaninIter = 1; myFaninIter < aGate.inputs.size(); myFaninIter++){
            myNewSignalValue = opXOR[myNewSignalValue][theCircuitState[aGate.inputs[myFaninIter]]];
            myDInputFlag = myDInputFlag || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D || theCircuitState[aGate.inputs[myFaninIter]] == SignalType::D_b;
        }
        myNewSignalValue = opNOT[myNewSignalValue];
    } else {
        std::cout << "Error: Unable to match gate " << aGate.gateType << std::endl;
        myReturnCode = ImplyReturnType::ERROR;
    }

    if (myNewSignalValue == SignalType::X && myDInputFlag) {
        theDFrontier.insert(aGateName);
    } else {
        theDFrontier.erase(aGateName);
    }

    bool mySignalIsOutputFlag = vectorContains<std::string>(theCircuitOutputs, aGateName);

    // Override assignment if a fault location is seen
    if ((aGateName == theFaultLocation) && (myNewSignalValue != SignalType::X)){
        if (theFaultValue == SignalType::D){
            if (myNewSignalValue == SignalType::ZERO){
                theCircuitState[aGateName] = myNewSignalValue;
                myReturnCode = ImplyReturnType::MASKED;
            } else {
                theCircuitState[aGateName] = SignalType::D;
                myReturnCode = ImplyReturnType::ACTIVATED;
            }
        }
        if (theFaultValue == SignalType::D_b){
            if (myNewSignalValue == SignalType::ONE){
                theCircuitState[aGateName] = myNewSignalValue;
                myReturnCode = ImplyReturnType::MASKED;
            } else {
                theCircuitState[aGateName] = SignalType::D_b;
                myReturnCode = ImplyReturnType::ACTIVATED;
            }
        }
    } else {
        theCircuitState[aGateName] = myNewSignalValue;
    }

    if (mySignalIsOutputFlag && (myNewSignalValue == SignalType::D || myNewSignalValue == SignalType::D_b)) {
        myReturnCode = ImplyReturnType::DETECTED;
    }

    // Return code priority determination
    if (myNewSignalValue != myOldSignalValue){
        for (auto& fanoutSignal : aGate.outputs) {
            // std::cout << "Debug: Checking gate " << fanoutSignal << " for update" << std::endl;
            ImplyReturnType myNewReturnCode = evaluateGateRecursive(fanoutSignal);

            if (myReturnCode == ImplyReturnType::MASKED && (myNewReturnCode == ImplyReturnType::DETECTED || myNewReturnCode == ImplyReturnType::ACTIVATED)){
                myReturnCode = ImplyReturnType::ERROR;
            } else {
                if (myReturnCode == ImplyReturnType::ERROR || myNewReturnCode == ImplyReturnType::ERROR){
                    myReturnCode = ImplyReturnType::ERROR;
                } else if (myNewReturnCode == ImplyReturnType::DETECTED) {
                    myReturnCode = myNewReturnCode;
                } else if (myNewReturnCode == ImplyReturnType::ACTIVATED && myReturnCode != ImplyReturnType::DETECTED) {
                    myReturnCode = myNewReturnCode;
                } else if (myNewReturnCode == ImplyReturnType::MASKED){
                    if (myReturnCode == ImplyReturnType::DETECTED || myReturnCode == ImplyReturnType::ACTIVATED) {
                        myReturnCode = ImplyReturnType::ERROR;
                    } else {
                        myReturnCode = myNewReturnCode;
                    }
                }
            }
        }
    }

    theCircuit[aGateName] = aGate;
    return myReturnCode;

}


// Initialize circuit to all Xs
void Circuit::resetCircuit(){
    for(auto& aCircuitInput: theCircuitInputs){
        setAndImplyCircuitInput(aCircuitInput, SignalType::X);
    }
}


// Return map of all current circuit inputs to values
std::unordered_map<std::string, SignalType> Circuit::getCurrCircuitInputValues(){
    std::unordered_map<std::string, SignalType> myCurrCircuitInputValues = std::unordered_map<std::string, SignalType>();
    for (auto& myInput : theCircuitInputs) {
        if (theCircuitState[myInput] == SignalType::D){
            myCurrCircuitInputValues[myInput] = SignalType::ONE;
        } else if (theCircuitState[myInput] == SignalType::D_b){
            myCurrCircuitInputValues[myInput] = SignalType::ZERO;
        } else {
            myCurrCircuitInputValues[myInput] = theCircuitState[myInput];
        }
    }
    return myCurrCircuitInputValues;
}
