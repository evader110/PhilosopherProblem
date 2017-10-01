# PhilosopherProblem
The philosopher problem is well known, and you can read about it here: https://en.wikipedia.org/wiki/Dining_philosophers_problem

My solution involves an arbitrator being the "ForkMaster" and my attempt to solve induced parallelism is whenever a philosopher 
  is done eating check all requests that have been made for people who can eat. The weakness to this problem is that checking every
  member is tedious and time consuming, especially since the number of philosophers is variable.
To run the compiled code use: mpirun -n # program, the # is one plus the total number of philosophers desired and program is the name 
  of your executable.
