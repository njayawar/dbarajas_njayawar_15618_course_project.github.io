#include <stdio.h>

#include "fframe.h"

#define LIMIT_NUM_SIGNALS 10240

void
faultSim_serial(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {
    for (int myTestVectorIdx = 0; myTestVectorIdx < aNumTestVectors; myTestVectorIdx++) {

        uint8_t myCorrectOutputs[LIMIT_NUM_SIGNALS];

        for (int myFaultIdx = 0; myFaultIdx < (aNumCircuitSignals * 2) + 1; myFaultIdx++) {

            uint8_t myLocalCircuitState[LIMIT_NUM_SIGNALS];
            int myCurrTraversalIdx;

            for (myCurrTraversalIdx = 0; myCurrTraversalIdx < aNumCircuitInputs; myCurrTraversalIdx++) {
                uint8_t myNewCircuitVal = aTestVectors[myTestVectorIdx*aNumCircuitInputs + myCurrTraversalIdx];

                if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCircuitTraversalOrder[myCurrTraversalIdx]){
                    if ((myFaultIdx-1) % 2 == 0) {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                    } else {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                    }
                } else {
                    myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
                }
            }

            for (; myCurrTraversalIdx < aNumCircuitSignals; myCurrTraversalIdx++) {

                int myCurrentGateIdx = aCircuitTraversalOrder[myCurrTraversalIdx];
                CudaGate myCurrGate = aCircuitStructure[myCurrentGateIdx];
                uint8_t myNewCircuitVal = myLocalCircuitState[myCurrGate.fanin[0]];

                switch (myCurrGate.gateType)
                {
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

                if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCircuitTraversalOrder[myCurrTraversalIdx]){
                    if ((myFaultIdx-1) % 2 == 0) {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                    } else {
                        myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                    }
                } else {
                    myLocalCircuitState[aCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
                }
            }

            if (myFaultIdx == 0) {
                for (int i = 0; i < aNumCircuitOutputs; i++) {

                    int myOutputIdx = aCircuitOutputs[i];
                    myCorrectOutputs[i] = myLocalCircuitState[myOutputIdx];

                }
            }

            if (myFaultIdx != 0){
                int myDetectedFaultsIdx = myTestVectorIdx * (aNumCircuitSignals * 2) + myFaultIdx - 1;
                aDetectedFaults[myDetectedFaultsIdx] = 0;
                for (int i = 0; i < aNumCircuitOutputs; i++) {

                    int myOutputIdx = aCircuitOutputs[i];
                    if (myCorrectOutputs[i] != myLocalCircuitState[myOutputIdx]) {
                        aDetectedFaults[myDetectedFaultsIdx] = 1;
                    }

                }
            }
        }
    }
}


void serialFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {
// faultSim_serial(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {
    faultSim_serial(aNumCircuitSignals, aCircuitStructure, aCircuitTraversalOrder, aNumCircuitInputs, aCircuitInputs, aNumCircuitOutputs, aCircuitOutputs, aNumTestVectors, aTestVectors, aDetectedFaults);
}
