//Ethan O'Dell

#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include <queue>

//run compiled code (for 5 philosophers) with mpirun -n 6 program

using namespace std;

int main ( int argc, char *argv[] ) 
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  int tag = 1;
  int forkMaster = 0;
  int amHungry = 1;
  int canEat = 2;
  int doneEating = 3;
  int ateCount = 0; //Number of Philosophers that have ate
  std::queue<int> weAreHungry; //Philosophers waiting on forks
  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of processes.
  p = MPI::COMM_WORLD.Get_size ( );
  int numPhilosophers = p - 1;

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank ( );
  
  //Safety check - need at least 2 philosophers to make sense
  if (p < 3) {
      MPI::Finalize ( );
      std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
      return 1; //non-normal exit
  }

  srand(id + time(NULL)); //Ensure random seed
  
  //Set the table
  int* forks = new int[numPhilosophers]; //Forks are set values from 1 to the number of philosophers
  for(int i = 1; i < p; i++) //Forks are set to 0 if it is taken
    forks[i-1] = i;

  while(ateCount < numPhilosophers)//only let each philosopher eat once
  {
    //  Setup Fork Master (Ombudsman) and Philosophers
    if ( id == forkMaster ) //Master
    {
      int msgIn;
      MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status);
      int thisPhilosopher = status.Get_source();
      if(msgIn == amHungry)//Someone wants to eat, lets see if they can
      {
        std::cout << "Recieved Request to eat from Philosopher " << thisPhilosopher << std::endl;
        if(forks[thisPhilosopher-1] != 0 && thisPhilosopher != numPhilosophers && forks[thisPhilosopher] != 0)
        { //if his left fork is available and he is not last guy and right fork available
          msgIn = canEat;
          std::cout << "Assigning forks " << forks[thisPhilosopher-1] << " and " << forks[thisPhilosopher] << " to Philosopher " << thisPhilosopher << std::endl;
          forks[thisPhilosopher-1] = 0; //this forks are taken
          forks[thisPhilosopher] = 0;

          MPI::COMM_WORLD.Send(&msgIn, 1, MPI::INT, thisPhilosopher, tag);
        }
        else if (forks[thisPhilosopher-1] != 0 && thisPhilosopher == numPhilosophers && forks[0] != 0)
        {
          msgIn = canEat;
          forks[thisPhilosopher-1] = 0;
          forks[0] = 0;
          MPI::COMM_WORLD.Send(&msgIn, 1, MPI::INT, thisPhilosopher, tag);
          std::cout << "Assigning forks " << thisPhilosopher << " and 1 to Philosopher " 
                    << thisPhilosopher << std::endl;
        }
        else
        {
          weAreHungry.push(thisPhilosopher);
          std::cout << "Could not process Philosopher " << thisPhilosopher 
                    << "'s request, adding to the wait queue\n";
        }
      }
      else if(msgIn == doneEating) //The philosopher is done eating
      {
        std::cout << "Recieved Done Eating from Philosopher " << thisPhilosopher << std::endl;
        forks[thisPhilosopher-1] = thisPhilosopher;
        std::cout << "Forks " << forks[thisPhilosopher-1] << " and ";
        if (thisPhilosopher == numPhilosophers) //Put the forks back but must check for last guy
        {
          forks[0] = 1;
          std::cout << "1 returned from Philosopher " << thisPhilosopher << std::endl;
        }
        else//he is not the last guy
        {
          forks[thisPhilosopher] = thisPhilosopher + 1;
          std::cout << forks[thisPhilosopher] << " returned from Philosopher " << thisPhilosopher << std::endl;
        }
        ateCount++; //Another philosopher has eaten
        //Check if someone else can eat
        if(!weAreHungry.empty())//Check if we can help the poor man in the queue
        {
          int first = weAreHungry.front();
          thisPhilosopher = weAreHungry.front(); 
          do
          {
            weAreHungry.pop();
            if(forks[thisPhilosopher-1] != 0 && thisPhilosopher != numPhilosophers && forks[thisPhilosopher] != 0)
            {
              msgIn = 2;
              forks[thisPhilosopher-1] = 0; //this forks are taken
              forks[thisPhilosopher] = 0;

              MPI::COMM_WORLD.Send(&msgIn, 1, MPI::INT, thisPhilosopher, tag);
              std::cout << "Removing Request from the queue\n"
                        << "Assigning forks " << thisPhilosopher << " and " << thisPhilosopher + 1
                        << " to Philosopher " << thisPhilosopher << std::endl;
              if(!weAreHungry.empty() && thisPhilosopher == first)
                first = weAreHungry.front();
            }
            else if (forks[thisPhilosopher-1] != 0 && thisPhilosopher == numPhilosophers && forks[0] != 0)
            {
              msgIn = 2;
              forks[thisPhilosopher-1] = 0;
              forks[0] = 0;
              MPI::COMM_WORLD.Send(&msgIn, 1, MPI::INT, thisPhilosopher, tag);
              std::cout << "Removing Request from the queue\n"
                        << "Assigning forks " << thisPhilosopher << " and 1 to Philosopher " 
                        << thisPhilosopher << std::endl;
              if(!weAreHungry.empty() && thisPhilosopher == first)
                first = weAreHungry.front();
            }
            else
            {
               std::cout << "Unable to process queued request from Philosopher " << thisPhilosopher << std::endl;
               weAreHungry.push(thisPhilosopher);
            }
            if(!weAreHungry.empty())
              thisPhilosopher = weAreHungry.front();
          }while(thisPhilosopher != first);
        }
      }
    }
    else //I'm a philosopher
    {
      int msgOut = amHungry;//I am hungry
      MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, forkMaster, tag);
      MPI::COMM_WORLD.Recv(&msgOut, 1, MPI::INT, forkMaster, tag);//I can eat
      if(msgOut == canEat)
      {
        msgOut = doneEating;
        MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, forkMaster, tag);//im done eating
      }
      break;//This implementation has each philosopher only eat once
    }
  }
  delete [] forks;
  MPI::Finalize ( );
  return 0;
}