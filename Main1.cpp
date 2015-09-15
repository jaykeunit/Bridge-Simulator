//Larry Martinez
//Cosc 4330 OS Summer 2015 
#define NTIMES 7
#define MEMSIZE 4

#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[])
{
   long key_mem;
   long key_memTicket;
   int sid_mem;
   int sid_ticket;
   int *pmem;
   int *ticket;
   int ticket_number_index = -1;
   int pid;
   int value = 1;
   sem_t *notfull;
   char sem_notfull[]= "ljm_notfull";
   int semvalue;
   string license_plate;
   int arrival_delay;
   int weight;
   int max_weight;
   int seconds_on_bridge;

   if(argc !=2) {
      cout<<"wrong number of arguments"<<endl;
      _exit(0);
   }

   max_weight = atoi(argv[1]);

   // Create notfull semaphore with initial value set to one
   notfull = sem_open(sem_notfull,  O_CREAT, 0600, value);

   if (notfull == SEM_FAILED) {
      perror("unable to create ljm_notfull semaphore");
      sem_unlink(sem_notfull);
      exit(1);
     }

   //create a shared memory segment
   key_mem = 7451268;
   sid_mem = shmget(key_mem, MEMSIZE, 0600 | IPC_CREAT);

   if (sid_mem == -1) {
      perror("Cannot get shared segment");
      exit(3);
   }

   pmem = (int *) shmat(sid_mem, NULL, 0);

   if (pmem == NULL) {
      perror("Cannot get address of shared segment");
      exit(24);
   }

   //create a shared memory segment
   key_mem = 7745892;
   sid_ticket = shmget(key_memTicket, MEMSIZE, 0600 | IPC_CREAT);

   if (sid_ticket == -1) {
      perror("Cannot get shared segment");
      exit(3);
   }

   ticket = (int *) shmat(sid_ticket, NULL, 0);

   if (ticket == NULL) {
      perror("Cannot get address of shared segment");
      exit(24);
   }

   //while loop to read input file
   while(!cin.eof()) {

      cin>> license_plate;
      cin>> arrival_delay;
      cin>> weight;
      cin>> seconds_on_bridge;

      sleep(arrival_delay);
      cout <<">>>" << license_plate <<" Arrives at the bridge: Car weights " <<weight<<" tons" << endl;
      
      if(weight > max_weight) {
         cout<<license_plate<<": exceeds max weight of the bridge"<<endl;
      }

      else {
         ticket_number_index++;

         //create a fork process for each vehicle trying to enter the bridge
         pid = fork();
         if (pid == 0) {

            for( ; ; ) {
               sem_wait(notfull);
               int transfer_weight = *pmem + weight;
               
               if(transfer_weight <= max_weight && ticket_number_index == *ticket) {
                  *ticket += 1;
                  *pmem += weight;
                  cout << ">>>" << license_plate << " Vehicle enters the bridge:  " << *pmem << " tons on the bridge." << endl;
                  sem_post(notfull);
                  break;
               }
               sem_post(notfull);
            }
            sleep(seconds_on_bridge);
            sem_wait(notfull);
            *pmem -= weight;
            cout << "<<<" << license_plate << " Vehicle leaves the bridge: " << *pmem << " tons now on the bridge." << endl;
            sem_post(notfull);
            _exit(0);
         }
      }
   }
   while(wait(0)!=-1);

   //detaching and destroying shared memory segments
   shmdt(pmem);
   shmctl(sid_mem, IPC_RMID, NULL);

   //detaching and destroying shared memory segments
   shmdt(ticket);
   shmctl(sid_ticket, IPC_RMID, NULL);

   //closing the semaphore
   sem_close(notfull);

   if(sem_unlink(sem_notfull) != 0) {
     cout << "WARNING: semaphore " << sem_notfull << " was not deleted" << endl;
   }
   return 0;
} // main 
