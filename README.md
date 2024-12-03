# 15-618 Course Project
## Project Proposal
### URL:
https://docs.google.com/document/d/1DsREZ87A6Q04XDlbyS9fuON7GX2PO-IlJ7pOVObdvxE/edit?usp=sharing

## Project Milestone Report:

| Half-Week | ToDo |
|:------|------|
| 11/27 - 11/30 | * Finish debugging parallel OpenMP implementation (David)  <br> * Initial performance measurements and correctness checking (Navod) |
| 12/1 - 12/3 | * Develop improvements and debug them (Navod & David) |
| 12/4 - 12/7 | * Implement SIMD-parallel Fault Simulation (Navod & David) |
| 12/8 - 12/10 | * Performance Measurements and Performance Debugging (Navod & David) |
| 12/11 - 12/13 | * Project Poster Presentation Writing (Navod & David) |

In the first week, we fully implemented our circuit simulator with some low-level operations abstracted away to make our PODEM algorithm implementation easier to develop. We added debug features that would enable inspection of the underlying certain to observe how the circuit is being tested. We then followed this up the following week with a sequential PODEM implementation that can perform ATPG for all possible stuck-at signal faults. We were able to validate the algorithm’s operation against an existing ATPG tool called Atalanta developed by Virginia Polytechnic Institute & State University. Our implementation was able to derive valid test vectors for the detectable stuck-at faults while correctly identifying the undetectable faults. We also implemented timing code in our algorithm to measure and report the computation time required for each stuck-at fault to begin evaluating the various implementations.

Currently, we have implemented an OpenMP-parallel implementation from the original sequential version that produces a valid output. We have tried two primary techniques of parallelization: ‘parallel for’ and ‘tasks’ to search the decision space across workers. We experimented with global completion flags and global thread spawning limits to optimize the performance. At this point, there is still work to be done to improve the performance of our parallel implementation.

So far, we’ve been able to stay on track with our original proposed schedule. We are confident in our ability to achieve all of our deliverables. We are also expected to finish the extra SIMD-parallel fault simulation that would complement our ATPG algorithm.
List of goals for the poster session:
* Sequential ATPG algorithm implementation (finished)
* Evaluation and results for a sequential ATPG algorithm (finished)
* Parallel ATPG algorithm implementation (finished/in-progress)
* Evaluation and results for a parallel ATPG algorithm (in-progress)
* Evaluation and results for sequential fault simulation (todo)
* Evaluation and results for parallel fault simulation (todo)

At the poster session we intend on having a graph displaying results on our benchmarks on both the GHC and PSC machines. We will also be able to demo the algorithm running and possibly a visualization of the decision trees that were traversed during ATPG.

The results that we presently have are the execution times of all of our benchmarks using the sequential implementation (displayed in scatter plot format below). Points in blue represent stuck-at faults which were successfully detected, while red points represent undetectable faults.

<img width="708" alt="Screenshot 2024-11-27 at 10 02 20 PM" src="https://github.com/user-attachments/assets/484f62e8-e4c7-482b-bb0e-c63218869e66">

Our largest concerns are with the ability to which we can schedule work to cores given the overhead of spawning multiple threads. At the core of our algorithm is a decision tree that we take advantage of by spawning a thread on both decisions that it can take (i.e. when it splits to both 1 and 0 branches). This will result in an unpredictably large set of threads to be spawned, depending on how many decisions are made by the algorithm on the particular circuit workload. We have attempted to throttle the total amount of spawned threads and default to sequential ATPG if we have hit the thread limit. We are also experimenting with using OpenMP tasks to allow dynamic scheduling of the decision tree traversal. Finally, we will have to consider other sources of parallelism in the algorithm (ex. rather than traversing the 0 and 1 side of the decision tree for one particular signal in parallel, explore decisions across multiple signals in parallel).

