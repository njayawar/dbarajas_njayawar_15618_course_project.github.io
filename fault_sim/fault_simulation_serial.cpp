#include <stdio.h>

#include "fframe.h"

// Data structure memory limits
#define LIMIT_NUM_SIGNALS 10240

// Serial implementation of fault simulation provided the CUDA-friendly input and output data structures
void
faultSim_serial(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {
    // Iterate through all provided test vectors
    for (int myTestVectorIdx = 0; myTestVectorIdx < aNumTestVectors; myTestVectorIdx++) {

        // Initialize golden circuit output structures
        uint8_t myCorrectOutputs[LIMIT_NUM_SIGNALS];

        // Iterate through all possible faults
        for (int myFaultIdx = 0; myFaultIdx < (aNumCircuitSignals * 2) + 1; myFaultIdx++) {

            // Initialize circuit state array
            uint8_t myLocalCircuitState[LIMIT_NUM_SIGNALS];
            int myCurrTraversalIdx;

            // Traverse through the inputs of the circuit to populate the state
            for (myCurrTraversalIdx = 0; myCurrTraversalIdx < aNumCircuitInputs; myCurrTraversalIdx++) {
                uint8_t myNewCircuitVal = aTestVectors[myTestVectorIdx*aNumCircuitInputs + myCurrTraversalIdx];

                // Override state if the signal corresponds to the current faultIdx
                if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCircuitTraversalOrder[myCurrTraversalIdx]){
                    if ((myFaultIdx-1) % 2 == 0) {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                    } else {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                    }
                } else {
                    // Write correct circuit value
                    myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
                }
            }

            // Iterate through all other circuit signals after inputs
            for (; myCurrTraversalIdx < aNumCircuitSignals; myCurrTraversalIdx++) {

                int myCurrentGateIdx = aCircuitTraversalOrder[myCurrTraversalIdx];
                CudaGate myCurrGate = aCircuitStructure[myCurrentGateIdx];
                uint8_t myNewCircuitVal = myLocalCircuitState[myCurrGate.fanin[0]];

                // Update circuit signal state based on signal type
                switch (myCurrGate.gateType)
                {
                // Determine output value given all current inputs (guaranteed to be determinable due to predetermined traversal order)
                case CudaGateType::AND:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal &= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    break;
                case CudaGateType::OR:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal |= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    break;
                case CudaGateType::NOT:
                    myNewCircuitVal = !myCurrGate.fanin[0];
                    break;
                case CudaGateType::XOR:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal ^= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    break;
                case CudaGateType::NAND:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal &= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    myNewCircuitVal = !myNewCircuitVal;
                    break;
                case CudaGateType::NOR:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal |= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    myNewCircuitVal = !myNewCircuitVal;
                    break;
                case CudaGateType::XNOR:
                    for (int myInputIdx = 1; myInputIdx < myCurrGate.faninSize; myInputIdx++) {
                        myNewCircuitVal ^= myLocalCircuitState[myCurrGate.fanin[myInputIdx]];
                    }
                    myNewCircuitVal = !myNewCircuitVal;
                    break;
                case CudaGateType::BUFF: // LEAVE EMPTY
                    /* code */
                    break;
                case CudaGateType::INPUT: // LEAVE EMPTY
                    /* code */
                    printf("Error: Should not see input at this phase of traversal: %d\n", myCurrTraversalIdx);
                    break;
                default:
                    break;
                }

                // Override state if the signal corresponds to the current faultIdx
                if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCircuitTraversalOrder[myCurrTraversalIdx]){
                    if ((myFaultIdx-1) % 2 == 0) {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                    } else {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                    }
                } else {
                    // Write correct circuit value
                    myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
                }
            }

            // Write golden outputs if no fault is being considered
            if (myFaultIdx == 0) {
                for (int i = 0; i < aNumCircuitOutputs; i++) {

                    int myOutputIdx = aCircuitOutputs[i];
                    myCorrectOutputs[i] = myLocalCircuitState[myOutputIdx];

                }
            }

            // All other faultIdx values compare its own output against golden outputs to determine fault detection
            if (myFaultIdx != 0){
                int myDetectedFaultsIdx = myTestVectorIdx * (aNumCircuitSignals * 2) + myFaultIdx - 1;
                aDetectedFaults[myDetectedFaultsIdx] = 0;
                for (int i = 0; i < aNumCircuitOutputs; i++) {

                    int myOutputIdx = aCircuitOutputs[i];
                    if (myCorrectOutputs[i] != myLocalCircuitState[myOutputIdx]) {
                        // Fault detected, update detected faults data structure
                        aDetectedFaults[myDetectedFaultsIdx] = 1;
                    }

                }
            }
        }
    }
}


void serialFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {
    faultSim_serial(aNumCircuitSignals, aCircuitStructure, aCircuitTraversalOrder, aNumCircuitInputs, aCircuitInputs, aNumCircuitOutputs, aCircuitOutputs, aNumTestVectors, aTestVectors, aDetectedFaults);
}
