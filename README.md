# CPU_scheduling_simulator
A CPU scheduling simulator composed of First Come First Serve, Shortest Jobs First, Round Robin and Priority Queues behavior.


Consisted of a baseline algorithm that is generic First Come First Serve. Also a Compute_stats module to aggregate collected scheduling results for cross algorithm performance comparison.

Fistly, a set of data representing job arrival time and job duration is prepared.

Then each algorithm is fed with the job data and each generate its own scheduling output.

The outputs are time steps in order, and the job that is currently running on the CPU in that respective time frame.

Then, compute_stats is fed with the job files and the output files of the algorithms. It will calculate the waiting time of jobs, number of context switches and longest response time.

The output of compute_stats is used to benchmark the scheduling algorithms.
