# 15-618 Course Project
## Project Proposal
### Title:
Parallelized Acceleration for Automatic Test Pattern Generation

### URL:
https://njayawar.github.io/dbarajas_njayawar_15618_course_project.github.io/

### Summary:
In IC design and manufacturing, one very important step in the process is testing the chips post-manufacturing to identify defects, determine yield, and perform binning - which requires the generation of test patterns tailored to the design. We aim to implement automatic test pattern generation (ATPG) algorithms using parallel programming concepts in order to observe the applicability of parallel programming to this problem and explore how various algorithms can be uniquely parallelized to optimize performance.

### Background:
Automatic test pattern generation (ATPG) is a method of deriving test vectors that could be used when testing chips post-manufacturing for yield analysis and binning. The process of ATPG is first based on a fault model which makes certain assumptions about the behavior of a circuit and guides how test patterns are generated. A common fault model is a single-stuck-at line (SSL) fault, which states that within any type of circuit, any wire can either be stuck at binary 1 or 0. Although not fully comprehensive, implementations have shown that deriving tests for this fault model fortuitously detect other types of faults and ATPG algorithms based on SSL faults can be adapted for other fault models (for example a bridge fault model which tests for two wires incorrectly being fused together during manufacturing). 

An ATPG algorithm needs to derive test vectors to detect every possible SSL fault possible on a circuit. For a test vector to successfully detect an SSL fault, there are two goals: to activate the fault and propagate it to the outputs. Digital circuits have the notion of control (circuit inputs) and observability points (circuit outputs) so ATPG algorithms need to find a suitable test vector to input into the circuit so that the faulty behavior of the circuit is observable on the outputs. There are multiple types of ATPG algorithms we can explore for our project, but we will initially focus on parallelizing the Path-Oriented Decision Making (PODEM) algorithm. 

The PODEM algorithm considers a particular SSL fault and backtraces the signals to the primary circuit inputs to determine input values that will activate the fault - for example if we are considering a stuck-at-1 fault for a signal, the objective would be to find a set of inputs that drive that signal to a 0 otherwise the fault will not be detected with that test vector. Signals in a circuit fan in and out due to gates with multiple inputs and the branching of signals which complicates ATPG as multiple signals may need to be determined to activate a fault. Once activated, PODEM focuses on propagating the fault to a primary output. If a faulty signal is going into an AND gate where the other input is a 0, the fault will not be detected because the output of that gate is controlled by the 0, not the faulty signal. Decisions made along the circuit will determine whether or not a particular test vector can activate and propagate the fault. These decisions can be tracked via a decision tree that is traversed when determining what inputs can detect a particular SSL fault.

<p align="center">
Decision Tree for a Simplified Circuit
  
  ![tmp](https://github.com/user-attachments/assets/1e80b56a-570a-4365-863d-e40993f86c09)
</p>

Another aspect of digital systems testing we may explore is the process of fault simulation. Fault simulation consists of taking the test vectors derived from ATPG and running it back into the circuit. Fault simulation helps determine how many faults are covered by a particular set of test vectors and reduces the test set size if a single test vector can detect multiple SSL faults.


### The Challenge:
ATPG is an np-hard computation problem that takes a significant amount of computation time and resources to complete - especially as ICs get larger and larger. Modern chips need to have millions of faults accounted for and tested, and ATPG processes in industry today take weeks of computation time to complete. Therefore, it is a suitable problem for acceleration.	

There are two particular sources of parallelism when it comes to ATPG algorithms such as PODEM. The first is parallelizing across SSL faults - deriving test vectors for independent SSL faults across multiple threads. However, this approach is not always suitable because a particular test vector may detect multiple SSL faults at once and parallelizing across SSL faults may generate redundant test vectors. This redundancy is to be avoided because larger test set sizes slow down how fast chips can be tested as they come off the line during high volume production. Another source of parallelism that can be explored is the traversal of the decision trees when activating and propagating a fault. Rather than serially exploring different paths in the decision tree, we can have multiple threads explore both paths at the same time which reduces the amount of backtracking that needs to be done when a particular traversal fails to generate a test vector that detects a fault. The challenge in parallelization comes through the inter-thread communication on whether or not a particular decision path has produced a valid result, and whether threads should continue searching down a path, backtrack to a previous decision due to a conflict detected in another thread, or terminate because another thread has found a complete test vector for the fault. 

There is a high amount of communication that is required to communicate decisions and circuit state during the process of ATPG. The state and underlying circuit information needs to be formatted in a way that reduces synchronization and improves locality. Therefore, it may be required of us to develop our own circuit emulator to represent the behavior of an arbitrary circuit in a way that is suitable for our project. Finally, scalability is a big focus in our project because we want to develop an implementation that can scale well to larger and larger circuits while maintaining performance by utilizing more parallel threads. To do so, we will have to minimize communication and synchronization between threads and allocate work to threads in an optimal manner.


### Resources:
For this project, we intend on focusing on OpenMP, MPI, and CUDA as our main forms of parallelization. Currently, we will be starting from scratch when implementing the PODEM algorithm and fault simulation code. There are multiple circuit emulators available that may be used to help represent the circuit during ATPG, but they may not be well suited for our particular task and programming language so we may be starting from scratch in that area too and we have time built in our schedule for developing a circuit emulator that would work best for our project. For OpenMP, MPI, and CUDA, we will use the previous programming assignments from this class when structuring our framework but because our problem is very different from prior assignments, we will have to develop the algorithms and emulators from scratch. 

For computation, we intend on using OpenMP benchmarking to determine performance with up to 8-cores and 128-cores on the GHC and PSC machines respectively. We may also use MPI to further scale the computation on the PSC machines. Finally, one of the goals that we hope to achieve is fault simulation, which can be done with the GPUs available on the GHC machines similar to prior assignments.


### Goals and Deliverables:
The primary goal that we plan to achieve is to develop a parallelized implementation of the PODEM ATPG algorithm that is able to achieve speedup over a single-threaded implementation. This will require the development of a circuit simulator that can represent the behavior of arbitrary circuits. We will have to effectively parallelize the implementation in a way that extracts as much concurrency as possible while minimizing synchronization and communication. With this goal, we hope to evaluate the improvements of using an 8-core and 128-core CPU. Another primary goal we have is to evaluate the scalability of our parallelized implementation which means determining the speedups we achieve with more available processor cores. 

One goal that we hope to achieve if we have the time is a highly-parallelized implementation of fault simulation. This problem is orthogonal to a parallelized ATPG algorithm, but is within the realm of digital systems testing and well suited for parallelization. Another goal that we hope to achieve if we have the time is evaluating a largely scaled implementation of our ATPG algorithm using MPI which may incur additional communication overheads but may be more scalable on very large compute clusters. This exploration is something that we hope to do if the schedule allows.

Performance-wise, we hope to achieve speedups that scale well with the number of threads that are being used for computation. We do expect to see diminishing returns as the thread counts increase due to extra communication and synchronization, but we would hope that our implementation is optimized enough to enable speedups with multiple threads. Similar to other OpenMP and MPI labs that we have completed, we may roughly expect ~6x speedup with an implementation using 8 threads, and ~64x speedup with an implementation using 128 threads, but these numbers are subject to a lot of implementation factors that have not been measured yet. We think that it is possible to achieve these speedup goals because there are multiple sources of parallelism in ATPG algorithms and the communication to computation ratio should be good as every task (input pattern) assigned to a processor requires a single-core evaluation of the results using the full circuit.

The demo that we will have for the poster session will mainly consist of speedup graphs between a single-threaded implementation and a parallelized version. We can also show the resulting test set size as another evaluation metric, and demonstrate the code running in real time. Because the circuits themselves can be visualized, we can show some of the test vectors that were generated by our code and how they activate and propagate certain faults. Lastly, the resulting decision tree can be something that is extracted from a particular run and shown to demonstrate how our implementation effectively parallelized the computation.

With the PODEM algorithm, we are hoping to learn more about how the different methods of parallelization affect the generated test set size, as minimizing redundancy in the resulting test vectors is an interesting aspect that is yet to be determined. We would also want to learn about the scalability of the PODEM algorithm and determine how well it is suited for large compute clusters. 

### Platform Choice:
In order to implement ATPG algorithms, we intend on using OpenMP on the GHC machines for small-scale parallelism and the resources available at the Pittsburgh Supercomputing Center for large-scale parallelism. This allows all threads to have access to the decision tree and circuit state. We can also utilize OpenMP task queues to allocate work to threads and add to the queue whenever new nodes in the decision tree are created. Using OpenMP also lets us evaluate a serial implementation that uses only one thread to effectively determine the benefits of parallelism in ATPG. Once we have created an implementation that works with the 8 cores available on the GHC machines, we can shift to the PSC machines and see how performance scales as the core count increases to 128. 

For further scaling, we may look into using MPI to perform ATPG so that multiple CPUs can be used for parallelization rather than being restricted to one CPU. However, this may introduce additional overheads for communication between CPUs using message passing - and these effects are something that we will have to explore during our implementation and benchmarking phase. OpenMP and MPI are by far the most suitable platforms we have covered in class for ATPG due to the dependencies and communication requirements required while performing the algorithm.

Lastly, the platform that we would choose for our hope to achieve goals of fault simulation is SIMD methods such as ISPC and CUDA. There are various methods of improving fault simulation speed - one of which is utilizing a CPU word’s bitwidth to represent the values of a signal across multiple different SSL faults. For example, a 64-bit processor can represent the behavior of 63 faulty circuits and 1 correct circuit circuit by performing bit-level operations on uint64_t types. This parallelization can be extended far further by utilizing ISPC or GPU threads, which are well suited for the SIMD-style of computation that needs to be done.

### Schedule:

| Week | ToDo |
|:------|------|
| 1: 11/10 - 11/16 | * Project proposal <br> * Designing implementation architecture and how the code will be structured/implemented |
| 2: 11/17 - 11/23 | * Build circuit simulator <br> * Build baseline non-parallel ATPG PODEM algorithm |
| 3: 11/24 - 11/30 | * Developing the parallel OpenMP implementation |
| 4: 12/1 - 12/7 | * Implementation debugging and improvements <br> * Extra: Fault simulation implementation |
| 5: 12/8 - 12/13 | * Performance measurements and performance debugging <br> * Project poster presentation writing |
