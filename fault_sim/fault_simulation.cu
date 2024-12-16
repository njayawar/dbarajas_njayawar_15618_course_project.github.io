#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

#include "CycleTimer.h"
#include "fframe.h"

// Data structure memory limits
#define LIMIT_NUM_SIGNALS 8192

// CUDA error checking
#define DEBUG
#ifdef DEBUG
#define cudaCheckError(ans) cudaAssert((ans), __FILE__, __LINE__);
inline void cudaAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
if (code != cudaSuccess)
{
fprintf(stderr, "CUDA Error: %s at %s:%d\n",
cudaGetErrorString(code), file, line);
if (abort) exit(code);
}
}
#else
#define cudaCheckError(ans) ans
#endif

// Read-only global constant during kernel invocation
struct GlobalConstants {
    int numCircuitSignals;
    int numCircuitInputs;
    int numCircuitOutputs;
    int numTestVectors;
};

__constant__ GlobalConstants cuConstParams;

// Parallel implementation of fault simulation provided the CUDA-friendly input and output data structures
__global__ void
faultSim_kernel(CudaGate* aCudaCircuitStructure, int* aCudaCircuitTraversalOrder, int* aCudaCircuitInputs,  int* aCudaCircuitOutputs,  uint8_t* aTestVectors, uint8_t* aDetectedFaults) {

    int myTestVectorIdx = blockIdx.x;
    int myThreadIdx = threadIdx.x;

    // Initialize shared golden outputs array and circuit state array
    __shared__ uint8_t myCorrectOutputs[LIMIT_NUM_SIGNALS];
    uint8_t myLocalCircuitState[LIMIT_NUM_SIGNALS];

    // Thread iterates through all responsible faults
    for (int myFaultIdx = myThreadIdx; myFaultIdx < (cuConstParams.numCircuitSignals * 2) + 1; myFaultIdx += blockDim.x) {

        int myCurrTraversalIdx;

        // Traverse through the inputs of the circuit to populate the state
        for (myCurrTraversalIdx = 0; myCurrTraversalIdx < cuConstParams.numCircuitInputs; myCurrTraversalIdx++) {
            uint8_t myNewCircuitVal = aTestVectors[myTestVectorIdx*cuConstParams.numCircuitInputs + myCurrTraversalIdx];

            // Override state if the signal corresponds to the current faultIdx
            if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCudaCircuitTraversalOrder[myCurrTraversalIdx]){
                if ((myFaultIdx-1) % 2 == 0) {
                    myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                } else {
                    myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                }
            } else {
                // Write correct circuit value
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
            }
        }

        // Iterate through all other circuit signals after inputs
        for (; myCurrTraversalIdx < cuConstParams.numCircuitSignals; myCurrTraversalIdx++) {

            int myCurrentGateIdx = aCudaCircuitTraversalOrder[myCurrTraversalIdx];
            CudaGate myCurrGate = aCudaCircuitStructure[myCurrentGateIdx];
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
            if ((myFaultIdx != 0) && ((myFaultIdx-1) / 2) == aCudaCircuitTraversalOrder[myCurrTraversalIdx]){
                if ((myFaultIdx-1) % 2 == 0) {
                    myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 0;
                } else {
                    myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = 1;
                }
            } else {
                // Write correct circuit value
                myLocalCircuitState[aCudaCircuitTraversalOrder[myCurrTraversalIdx]] = myNewCircuitVal;
            }
        }

        // Write shared golden outputs if myFaultIdx == 0
        if (myFaultIdx == 0) {
            for (int i = 0; i < cuConstParams.numCircuitOutputs; i++) {

                int myOutputIdx = aCudaCircuitOutputs[i];
                myCorrectOutputs[i] = myLocalCircuitState[myOutputIdx];
            }
        }

        __syncthreads();

        // All other faultIdx/threadIdx compare local output states against shared golden states and populates global detection data structure
        if (myFaultIdx != 0){
            int myDetectedFaultsIdx = myTestVectorIdx * (cuConstParams.numCircuitSignals * 2) + myFaultIdx - 1;
            aDetectedFaults[myDetectedFaultsIdx] = 0;
            for (int i = 0; i < cuConstParams.numCircuitOutputs; i++) {

                int myOutputIdx = aCudaCircuitOutputs[i];
                if (myCorrectOutputs[i] != myLocalCircuitState[myOutputIdx]) {
                    aDetectedFaults[myDetectedFaultsIdx] = 1;
                }

            }
        }
    }
}


// CUDA entry point - initialize memory and invoke kernel
void cudaFaultSim(int aNumCircuitSignals, CudaGate* aCircuitStructure, int* aCircuitTraversalOrder, int aNumCircuitInputs, int* aCircuitInputs, int aNumCircuitOutputs, int* aCircuitOutputs, int aNumTestVectors, uint8_t* aTestVectors, uint8_t* aDetectedFaults) {

    // Compute number of blocks and threads per block
    const int myThreadsPerBlock = std::min((aNumCircuitSignals * 2) + 1, 768);
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
    cudaMalloc(&myCudaTestVectors, sizeof(uint8_t) * aNumCircuitInputs * aNumTestVectors);
    cudaMalloc(&myCudaDetectedFaults, sizeof(uint8_t) * aNumCircuitSignals * 2 * aNumTestVectors);

    // Perform memcpy of data structures
    cudaMemcpy(myCudaCircuitStructure, aCircuitStructure, sizeof(CudaGate) * aNumCircuitSignals, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitTraversalOrder, aCircuitTraversalOrder, sizeof(int) * aNumCircuitSignals, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitInputs, aCircuitInputs, sizeof(int) * aNumCircuitInputs, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaCircuitOutputs, aCircuitOutputs, sizeof(int) * aNumCircuitOutputs, cudaMemcpyHostToDevice);
    cudaMemcpy(myCudaTestVectors, aTestVectors, sizeof(uint8_t) * aNumCircuitInputs * aNumTestVectors, cudaMemcpyHostToDevice);

    // Populate kernel global constants
    GlobalConstants params;
    params.numCircuitSignals = aNumCircuitSignals;
    params.numCircuitInputs = aNumCircuitInputs;
    params.numCircuitOutputs = aNumCircuitOutputs;
    params.numTestVectors = aNumTestVectors;
    cudaMemcpyToSymbol(cuConstParams, &params, sizeof(GlobalConstants));

    // Run kernel
    faultSim_kernel<<<myNumBlocks, myThreadsPerBlock>>>(myCudaCircuitStructure, myCudaCircuitTraversalOrder, myCudaCircuitInputs, myCudaCircuitOutputs, myCudaTestVectors, myCudaDetectedFaults);
    cudaCheckError(cudaDeviceSynchronize());

    // Perform memcpy of results
    cudaMemcpy(aDetectedFaults, myCudaDetectedFaults, sizeof(uint8_t) * aNumCircuitSignals * 2 * aNumTestVectors, cudaMemcpyDeviceToHost);
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
