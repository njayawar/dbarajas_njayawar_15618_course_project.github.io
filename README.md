# 15-618 Course Project
## Project Proposal
### URL:
https://docs.google.com/document/d/1DsREZ87A6Q04XDlbyS9fuON7GX2PO-IlJ7pOVObdvxE/edit?usp=sharing

# 15-618 Course Project
## Project Proposal
### URL:
https://docs.google.com/document/d/1DsREZ87A6Q04XDlbyS9fuON7GX2PO-IlJ7pOVObdvxE/edit?usp=sharing

### Project Milestone Report:

| Half-Week | ToDo |
|:------|------|
| 11/27 - 11/30 | * Finish debugging parallel OpenMP implementation (David)  * Initial performance measurements and correctness checking (Navod) |
| 12/1 - 12/3 | * Develop improvements and debug them (Navod & David) |
| 12/4 - 12/7 | * Implement SIMD-parallel Fault Simulation (Navod & David) |
| 12/8 - 12/10 | * Performance Measurements and Performance Debugging (Navod & David) |
| 12/11 - 12/13 | * Project Poster Presentation Writing (Navod & David) |

By the first week we were able to finish deriving both all implementation details of the sequential algorithm as well as build the testing infrastructure needed. By the second week we were able to finish the sequential implementation, developing a C++-based ATPG PODEM algorithm that can generate test vectors that we are able to cross-check with online implementations. Currently we have implemented an OpenMP-parallel implementation from the original sequential version that is still being debugged for correctness.

So far for the project we’ve been able to stay slightly ahead of our original proposed schedule. We are confident in our ability to achieve all of our deliverables. We are also expected to finish the extra SIMD-parallel fault simulation that would complement our ATPG algorithm.

At the poster session we intend on having a graph displaying results on our benchmarks on both the GHC and PSC machines.

The results that we presently have are the execution times of all of our benchmarks using the sequential implementation (displayed in scatter plot format below):

<img width="708" alt="Screenshot 2024-11-27 at 10 02 20 PM" src="https://github.com/user-attachments/assets/484f62e8-e4c7-482b-bb0e-c63218869e66">

Our largest concerns are with the ability to which we can schedule work to cores given the overhead of spawning multiple threads. At the core of our algorithm is a decision tree that we take advantage of by spawning a thread on both decisions that it can take (i.e. when it splits to both 1 and 0 branches). This will result in an unpredictably large set of threads to be spawned, depending on how many decisions are made by the algorithm on the particular circuit workload. Given that the parallel implementation is not finished yet we are not sure how many threads will be spawned for our benchmarks and if we’d have to throttle the total amount and make our final threads more coarse so that the overhead of spawning them won’t dominate the total execution time. This parameter is something that we must tune via experimentation which has yet to be done.

