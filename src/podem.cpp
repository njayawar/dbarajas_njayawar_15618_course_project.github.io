#include "cframe.h"
#include "podem.h"


// Helper function to determine success of PODEM
bool errorAtPO(Circuit& aCircuit){
    for (auto& myOutput : aCircuit.theCircuitOutputs) {
        if ((aCircuit.theCircuitState[myOutput] == SignalType::D) || (aCircuit.theCircuitState[myOutput] == SignalType::D_b)){
            return true;
        }
    }
    return false;
}


// Determine noncontrolling value of an input gate type
SignalType getNonControllingValue(std::string aGate){
    if (aGate == "AND" || aGate == "and" || aGate == "NAND" || aGate == "nand"){
        return SignalType::ONE;
    }
    if (aGate == "OR" || aGate == "or" || aGate == "NOR" || aGate == "nor"){
        return SignalType::ZERO;
    }
    if (aGate == "XOR" || aGate == "xor" || aGate == "XNOR" || aGate == "xnor"){
        return SignalType::ZERO;
    }
    std::cout << "Error: Gate type " << aGate << " should not be on the DFrontier" << std::endl;
    return SignalType::ZERO;
}


// Return a set of current available objectives for Across-Signals parallelism
std::vector<std::pair<std::string, SignalType>> getMultipleObjectives(Circuit& aCircuit){
    std::vector<std::pair<std::string, SignalType>> myObjectives = std::vector<std::pair<std::string, SignalType>>();
    // Objective is activation
    if (aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X){
        SignalType mySAObjective = (aCircuit.theFaultValue == SignalType::D) ? SignalType::ONE : SignalType::ZERO;
        myObjectives.push_back(std::pair<std::string, SignalType>(aCircuit.theFaultLocation, mySAObjective));
        return myObjectives;
    }
    // Objective is propogation
    for (std::string myDFrontierGate : aCircuit.theDFrontier) {
        if (myObjectives.size() >= static_cast<std::size_t>(MAX_PARALLEL_OBJECTIVES)) {
            break;
        }
        for (auto& myDFrontierGateInput : aCircuit.theCircuit[myDFrontierGate].inputs) {
            if ((aCircuit.theCircuitState[myDFrontierGateInput] == SignalType::X)) {
                if (myObjectives.size() >= static_cast<std::size_t>(MAX_PARALLEL_OBJECTIVES)) {
                    break;
                }
                myObjectives.push_back(std::pair<std::string, SignalType>(myDFrontierGateInput, getNonControllingValue(aCircuit.theCircuit[myDFrontierGate].gateType)));
            }
        }
    }

    if (myObjectives.size() == 0){
        std::cout << "Error: Unable to create objective when it should have been possible" << std::endl;
    }

    return myObjectives;
}


// Return a single available objective from the circuit
std::pair<std::string, SignalType> getObjective(Circuit& aCircuit){
    // Objective is activation
    if (aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X){
        SignalType mySAObjective = (aCircuit.theFaultValue == SignalType::D) ? SignalType::ONE : SignalType::ZERO;
        return std::pair<std::string, SignalType>(aCircuit.theFaultLocation, mySAObjective);
    }
    // Objective is propogation
    std::string myDFrontierGate = *(aCircuit.theDFrontier.begin());
    for (auto& myDFrontierGateInput : aCircuit.theCircuit[myDFrontierGate].inputs) {
        if (aCircuit.theCircuitState[myDFrontierGateInput] == SignalType::X){
            return std::pair<std::string, SignalType>(myDFrontierGateInput, getNonControllingValue(aCircuit.theCircuit[myDFrontierGate].gateType));
        }
    }
    std::cout << "Error: Unable to create objective when it should have been possible" << std::endl;
    return std::pair<std::string, SignalType>("", SignalType::X);
}


// Given an objective, backtrace to a primary input to determine signal input and value based on circuit heuristics
std::pair<std::string, SignalType> doBacktrace(Circuit& aCircuit, std::pair<std::string, SignalType> anObjective){
    std::string myBacktraceSignal = anObjective.first;
    SignalType myBacktraceValue = anObjective.second;

    while (std::ranges::find(aCircuit.theCircuitInputs, myBacktraceSignal) == aCircuit.theCircuitInputs.end()){
        Gate myGate = aCircuit.theCircuit[myBacktraceSignal];
        bool myGateBubble = (myGate.gateType == "NAND") || (myGate.gateType == "nand") || (myGate.gateType == "NOR") || (myGate.gateType == "nor") || (myGate.gateType == "XNOR") || (myGate.gateType == "xnor") || (myGate.gateType == "NOT") || (myGate.gateType == "not");

        std::string myBacktraceSignalPrev = myBacktraceSignal; // DEBUG code
        for (auto& myBacktraceGateInput : aCircuit.theCircuit[myBacktraceSignal].inputs){
            if (aCircuit.theCircuitState[myBacktraceGateInput] == SignalType::X){
                myBacktraceSignal = myBacktraceGateInput;
                break;
            }
        }

        if (myBacktraceSignal == myBacktraceSignalPrev){ // DEBUG code
            std::cout << "Error: unable to perform backtrace (find input value == X)" << std::endl;
        }

        if (myGateBubble){
            myBacktraceValue = (myBacktraceValue == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        }
    }

    return std::pair<std::string, SignalType>(myBacktraceSignal, myBacktraceValue);
}


// PODEM with tasks parallelized Across-Decisions
std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelDecisions(Circuit& aCircuit){

    // aCircuit.printCircuitState();
    if (theSolutionFound) {
        return std::unordered_map<std::string, SignalType>();
    }

    if (errorAtPO(aCircuit)){
        theSolutionFound = true;
        return aCircuit.getCurrCircuitInputValues();
    }
    if (aCircuit.theDFrontier.empty() && !(aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X)){
        return std::unordered_map<std::string, SignalType>();
    }

    // Find an objective within the circuit
    std::pair<std::string, SignalType> myObjective = getObjective(aCircuit);

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    // Backtrce to primary input to make a decision
    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;

    const int myNumTasks = 2;
    std::unordered_map<std::string, SignalType> myPODEMResults[myNumTasks];
    std::vector<Circuit> myCircuits = std::vector<Circuit>(myNumTasks);

    // std::cout << "Number of active tasks: " << theTaskCnt << std::endl;

    // Under the parallel-throttling threshold, spawn new tasks
    if (theTaskCnt < MAX_ACTIVE_TASKS){

        // Update number of active tasks
        #pragma omp critical
        {
            theTaskCnt += 2;
            if (theTaskCnt > theMaxTaskCnt) {
                theMaxTaskCnt = theTaskCnt;
            }
        }

        // Spawn concurrent tasks for the decisions
        #pragma taskgroup
        {
            // std::cout << "Spawning tasks from thread " << omp_get_thread_num() << std::endl;
            #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
            {
                // std::cout << "Executing task 0 in thread " << omp_get_thread_num() << " at nested level " << omp_get_level() << std::endl;
                myCircuits[0] = aCircuit;
                myCircuits[0].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                myPODEMResults[0] = runPODEMRecursiveParallelDecisions(myCircuits[0]);
                // finishedTasks0 = true;
                #pragma omp critical
                {
                    theTaskCnt--;
                }
            }

            #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
            {
                // std::cout << "Executing task 1 in thread " << omp_get_thread_num() << " at nested level " << omp_get_level() << std::endl;
                myCircuits[1] = aCircuit;
                myCircuits[1].setAndImplyCircuitInput(myDecision.first, (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE);
                myPODEMResults[1] = runPODEMRecursiveParallelDecisions(myCircuits[1]);
                // finishedTasks1 = true;
                #pragma omp critical
                {
                    theTaskCnt--;
                }
            }
            // std::cout << "Thread waiting at taskwait " << omp_get_thread_num() << std::endl;
            #pragma omp taskwait
        }
        // }
        // std::cout << "Thread proceeding after taskwait " << omp_get_thread_num() << std::endl;

        // while (!finishedTasks0 || !finishedTasks1) {
        //     #pragma omp taskyield
        // }

        // Check results of each decision
        if (!myPODEMResults[0].empty()) {
            aCircuit = myCircuits[0];
            return myPODEMResults[0];
        } else if (!myPODEMResults[1].empty()) {
            aCircuit = myCircuits[1];
            return myPODEMResults[1];
        } else {
            aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
            return std::unordered_map<std::string, SignalType>();
        }

    // Default to serial computation
    } else {

        // Set decision and recursively run PODEM
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        // Previous decision failed, backtrack and try opposite decision
        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        myPODEMResult = runPODEMRecursiveParallelDecisions(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        // Failed, reset decision and return NULL
        aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
        return std::unordered_map<std::string, SignalType>();

    }
}


// Serial implementation of the PODEM algorithm
std::unordered_map<std::string, SignalType> runPODEMRecursiveSerial(Circuit& aCircuit){

    // aCircuit.printCircuitState();
    if (errorAtPO(aCircuit)){
        theSolutionFound = true;
        return aCircuit.getCurrCircuitInputValues();
    }
    if (aCircuit.theDFrontier.empty() && !(aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X)){
        return std::unordered_map<std::string, SignalType>();
    }

    // Backtrce to primary input to make a decision
    std::pair<std::string, SignalType> myObjective = getObjective(aCircuit);

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    // Backtrce to primary input to make a decision
    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;

    // Set decision and recursively run PODEM
    aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveSerial(aCircuit);
    if(!myPODEMResult.empty()){
        return myPODEMResult;
    }

    // Previous decision failed, backtrack and try opposite decision
    myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
    aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
    myPODEMResult = runPODEMRecursiveSerial(aCircuit);
    if(!myPODEMResult.empty()){
        return myPODEMResult;
    }

    // Failed, reset decision and return NULL
    aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
    return std::unordered_map<std::string, SignalType>();
}


// PODEM with tasks parallelized Across-Signals
std::unordered_map<std::string, SignalType> runPODEMRecursiveParallelSignals(Circuit& aCircuit){

    // aCircuit.printCircuitState();
    if (theSolutionFound) {
        return std::unordered_map<std::string, SignalType>();
    }

    if (errorAtPO(aCircuit)){
        theSolutionFound = true;
        return aCircuit.getCurrCircuitInputValues();
    }
    if (aCircuit.theDFrontier.empty() && !(aCircuit.theCircuitState[aCircuit.theFaultLocation] == SignalType::X)){
        return std::unordered_map<std::string, SignalType>();
    }

    // Generate a set of concurrent objectives
    std::vector<std::pair<std::string, SignalType>> myObjectives = getMultipleObjectives(aCircuit);
    int myObjectivesSize = myObjectives.size();

    // std:: cout << "Info: My current objective: " << myObjective.first << " | " << myObjective.second << std::endl;

    // std:: cout << "Info: My current decision: " << myDecision.first << " | " << myDecision.second << std::endl;


    const int myNumTasks = MAX_PARALLEL_OBJECTIVES;
    std::unordered_map<std::string, SignalType> myPODEMResults[myNumTasks];
    std::vector<Circuit> myCircuits = std::vector<Circuit>(myNumTasks);

    // std::cout << "Number of active tasks: " << theTaskCnt << std::endl;

    if (theTaskCnt < MAX_ACTIVE_TASKS){

        #pragma omp critical
        {
            theTaskCnt += myObjectives.size();
            if (theTaskCnt > theMaxTaskCnt) {
                theMaxTaskCnt = theTaskCnt;
            }
        }

        #pragma taskgroup
        {
            // std::cout << "Spawning tasks from thread " << omp_get_thread_num() << std::endl;
            for (int i = 0; i < myObjectivesSize; i++) {
                std::pair<std::string, SignalType> myObjective = myObjectives[i];

                // Spawn a task for each possible propogation strategy
                #pragma omp task untied shared(myCircuits) shared(myPODEMResults)
                {
                    // Make a custom decision for each objective
                    std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjective);

                    myCircuits[i] = aCircuit;
                    myCircuits[i].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                    myPODEMResults[i] = runPODEMRecursiveParallelSignals(myCircuits[i]);

                    if(myPODEMResults[i].empty()){
                        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
                        myCircuits[i].setAndImplyCircuitInput(myDecision.first, myDecision.second);
                        myPODEMResults[i] = runPODEMRecursiveParallelSignals(myCircuits[i]);
                    }

                    #pragma omp critical
                    {
                        theTaskCnt--;
                    }
                }
            }
            // std::cout << "Thread waiting at taskwait " << omp_get_thread_num() << std::endl;
            #pragma omp taskwait
        }
        // }
        // std::cout << "Thread proceeding after taskwait " << omp_get_thread_num() << std::endl;

        // while (!finishedTasks0 || !finishedTasks1) {
        //     #pragma omp taskyield
        // }

        for (int i = 0; i < myObjectivesSize; i++) {
            if (!myPODEMResults[i].empty()) {
                aCircuit = myCircuits[i];
                return myPODEMResults[i];
            }
        }

        return std::unordered_map<std::string, SignalType>();

    } else {
        std::pair<std::string, SignalType> myDecision = doBacktrace(aCircuit, myObjectives[0]);

        // Set decision and recursively run PODEM
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        std::unordered_map<std::string, SignalType> myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        // Previous decision failed, backtrack and try opposite decision
        myDecision.second = (myDecision.second == SignalType::ONE) ? SignalType::ZERO : SignalType::ONE;
        aCircuit.setAndImplyCircuitInput(myDecision.first, myDecision.second);
        myPODEMResult = runPODEMRecursiveParallelSignals(aCircuit);
        if(!myPODEMResult.empty()){
            return myPODEMResult;
        }

        // Failed, reset decision and return NULL
        aCircuit.setAndImplyCircuitInput(myDecision.first, SignalType::X);
        return std::unordered_map<std::string, SignalType>();

    }
}
