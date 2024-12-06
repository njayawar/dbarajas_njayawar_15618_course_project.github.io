#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

#include "CycleTimer.h"
#include "fframe.h"

#define LIMIT_NUM_SIGNALS 1024

struct GlobalConstants {
    int numCircuitSignals;
    int numCircuitInputs;
    int numCircuitOutputs;
    int numTestVectors;
};

__constant__ GlobalConstants cuConstParams;

__global__ void
faultSim_kernel(CudaGate* aCudaCircuitStructure, int* aCudaCircuitTraversalOrder, int* aCudaCircuitInputs,  int* aCudaCircuitOutputs,  uint8_t* aTestVectors, uint8_t* aDetectedFaults) {

    int myTestVectorIdx = blockIdx.x;
    int myFaultIdx = threadIdx.x;

    __shared__ uint8_t myCorrectOutputs[LIMIT_NUM_SIGNALS];

    uint8_t myLocalCircuitState[LIMIT_NUM_SIGNALS];
    int myCurrTraversalIdx;

    for (myCurrTraversalIdx = 0; myCurrTraversalIdx < cuConstParams.numCircuitInputs; myCurrTraversalIdx++) {
        uint8_t myNewCircuitVal = aTestVectors[myTestVectorIdx*cuConstParams.numCircuitInputs + myCurrTraversalIdx];

        if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCudaCircuitTraversalOrder[myCurrTraversalIdx]){
            if ((myFaultIdx-1) % 2 == 0) {
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
            } else {
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
            }
        } else {
            myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
        }
    }

    for (; myCurrTraversalIdx < cuConstParams.numCircuitSignals; myCurrTraversalIdx++) {

        int myCurrentGateIdx = aCudaCircuitTraversalOrder[myCurrTraversalIdx];
        CudaGate myCurrGate = aCudaCircuitStructure[myCurrentGateIdx];
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

        if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCudaCircuitTraversalOrder[myCurrTraversalIdx]){
            if ((myFaultIdx-1) % 2 == 0) {
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
            } else {
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
            }
        } else {
            myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
        }
    }

    __syncthreads();

    if (myFaultIdx == 0) {
        for (int i = 0; i < cuConstParams.numCircuitOutputs; i++) {

            int myOutputIdx = aCudaCircuitOutputs[i];
            myCorrectOutputs[i] = myLocalCircuitState[myOutputIdx];

        }
    }

    __syncthreads();

    if (myFaultIdx != 0){
        int myDetectedFaultsIdx = myTestVectorIdx * (cuConstParams.numCircuitSignals * 2) + myFaultIdx - 1;
        aDetectedFaults[myDetectedFaultsIdx] = 0;
        for (int i = 0; i < cuConstParams.numCircuitOutputs; i++) {

            int myOutputIdx = aCudaCircuitOutputs[i];
            if (myCorrectOutputs[i] != myLocalCircuitState[myOutputIdx]) {
                aDetectedFaults[myDetectedFaultsIdx] = 1;
                printf("Found detected fault on output %d | Good: %d | Bad: %d\n", myOutputIdx, myCorrectOutputs[i], myLocalCircuitState[myOutputIdx]);
            }

        }
    }
}


void cudaFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {

    // Compute number of blocks and threads per block
    const int myThreadsPerBlock = (aNumCircuitSignals * 2) + 1;
    const int myNumBlocks = aNumTestVectors;

    if (aNumCircuitSignals >= LIMIT_NUM_SIGNALS){
        printf("Error: Too many signals within circuit - need to increase LIMIT_NUM_SIGNALS\n");
    }

    // Allocate buffers on GPU
    CudaGate* myCudaCircuitStructure;
    int* myCudaCircuitTraversalOrder;
    int* myCudaCircuitInputs;
    int* myCudaCircuitOutputs;
    uint8_t* myCudaTestVectors;
    uint8_t* myCudaDetectedFaults;
    cudaMalloc(&myCudaCircuitStructure, sizeof(CudaGate) * aNumCircuitSignals);
    cudaMalloc(&myCudaCircuitTraversalOrder, sizeof(int) * aNumCircuitSignals);
    cudaMalloc(&myCudaCircuitInputs, sizeof(int) * aNumCircuitInputs);
    cudaMalloc(&myCudaCircuitOutputs, sizeof(int) * aNumCircuitOutputs);
    cudaMalloc(&myCudaTestVectors, sizeof(uint8_t) * aNumCircuitInputs * aNumCircuitSignals);
    cudaMalloc(&myCudaDetectedFaults, sizeof(uint8_t) * aNumCircuitSignals * 2 * aNumTestVectors);

    // start timing after allocation of device memory
    double startTime = CycleTimer::currentSeconds();

    cudaMemcpy(myCudaCircuitStructure, aCircuitStructure, sizeof(CudaGate) * aNumCircuitSignals, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitTraversalOrder, aCircuitTraversalOrder, sizeof(int) * aNumCircuitSignals, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitInputs, aCircuitInputs, sizeof(int) * aNumCircuitInputs, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitOutputs, aCircuitOutputs, sizeof(int) * aNumCircuitOutputs, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaTestVectors, aTestVectors, sizeof(uint8_t) * aNumCircuitInputs * aNumCircuitSignals, cudaMemcpyHostToDevice);

    GlobalConstants params;
    params.numCircuitSignals = aNumCircuitSignals;
    params.numCircuitInputs = aNumCircuitInputs;
    params.numCircuitOutputs = aNumCircuitOutputs;
    params.numTestVectors = aNumTestVectors;
    cudaMemcpyToSymbol(cuConstParams, &params, sizeof(GlobalConstants));

    // Run kernel
    faultSim_kernel<<<myNumBlocks, myThreadsPerBlock>>>(myCudaCircuitStructure, myCudaCircuitTraversalOrder, myCudaCircuitInputs, myCudaCircuitOutputs, myCudaTestVectors, myCudaDetectedFaults);
    cudaDeviceSynchronize();

    cudaMemcpy(aDetectedFaults, myCudaDetectedFaults, sizeof(uint8_t) * aNumCircuitSignals * 2 * aNumTestVectors, cudaMemcpyDeviceToHost);

    double endTime = CycleTimer::currentSeconds();
}

void
printCudaInfo() {

    // for fun, just print out some stats on the machine

    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    printf("---------------------------------------------------------\n");
    printf("Found %d CUDA devices\n", deviceCount);

    for (int i=0; i<deviceCount; i++) {
        cudaDeviceProp deviceProps;
        cudaGetDeviceProperties(&deviceProps, i);
        printf("Device %d: %s\n", i, deviceProps.name);
        printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
        printf("   Global mem: %.0f MB\n",
               static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
        printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
    }
    printf("---------------------------------------------------------\n");
}
