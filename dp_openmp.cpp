/*
Works Cited:
(1) Mutex Implementation: https://github.com/jenny-codes/operating-systems/blob/master/06-synchronization/dining_philosophers.c
(2) Semaphore Implementation: https://www.geeksforgeeks.org/dining-philosopher-problem-using-semaphores/
    ** NOTE: For the mutexes and semaphores, I just replaced them with then OpenMP equivalent, in comparison to my original code.
(3) Solution Outline: https://www.youtube.com/watch?v=syMOLWlGjNg
(4) Another Solution Outline: https://legacy.cs.indiana.edu/classes/p415-sjoh/hw/project/dining-philosophers/index.htm
*/

#include <iostream>
#include <omp.h>
#include <unistd.h>
#include <chrono>

using namespace std;

//3 states of Philosopher
#define thinking 0      //Doing nothing
#define hungry 1        //Wants a fork
#define eating 2        //Using Fork

//Defining left and right. Will be used for both philosophers and forks
#define left pid + 1
#define right (pid + philosopher - 1) % philosopher

//Global Variables.
typedef chrono::high_resolution_clock Clock;
int philosopher, pid[100], state[100], i;  //Defining philosophers, their IDs, and their states.
omp_lock_t pickup, putdown;                //2 locks for picking up and putting down
omp_lock_t lock[100];                      //Philosopher locks
int food = 100;

//Checks if fork is being used!
/*If philosopher is hungry and the guys on the left and right
aren't eating, change the state to eating, and eat! */
void check(int pid) {
    if (state[pid] == hungry && state[left] != eating
        && state[right] != eating) {    //If Philosopher is hungry and both left & right aren't eating
        cout << "Philosopher " << pid
        << " eating..." << endl;        //Philosopher declares they're eating
        state[pid] = eating;            //Philosopher is eating now
        usleep(rand()  % 3 + 1);        //Philosopher eats for random amount of time.
        cout << "Philosopher " << pid
        << " done eating..." << endl;   //Philosopher declares they're eating
        omp_unset_lock(&lock[pid]);     //Fork is no longer occupied announcement.
    }
}

//Pick Up Fork Function
void pick_up(int pid) {
    omp_set_lock(&pickup);              //Start picking up fork
    cout << "Philosopher " << pid 
    << " hungry..." << endl;            //Philosopher declares they're hungry.
    state[pid] = hungry;                //Philosopher is hungry now.
    check(pid);                         //Check if left and right philosophers are eating.
    omp_unset_lock(&pickup);            //Finish picking up fork.
    omp_set_lock(&lock[pid]);           //Fork is now occupied announcement.
}

//Put Down Fork Function (Attempt at acyclic setup for 1a extra credit)
void put_down(int pid) {
    omp_set_lock(&putdown);             //Start putting down fork.
    if (pid % 2 == 0) {                 //If its an even-numbered Philosopher (1a implementation)
        check(right);                   //Wake up & ask if right Philosopher needs fork
        check(left);                    //Wake up & ask if left Philosopher needs fork
    }
    else {                              //If its an odd-numbered Philosopher
        check(left);                    //Wake up & ask if left Philosopher needs fork
        check(right);                   //Wake up & ask if right Philosopher needs fork
    }
    state[pid] = thinking;
    omp_unset_lock(&putdown);
}

//Philosopoher Function
void action() {
    while (food > 0) {            //While threads are active and there's food
        int pid = omp_get_thread_num();
        cout << "Philosopher " << pid
             << " thinking..." << endl; //Philosopher thinks
        usleep(rand()  % 3 + 1);        //Philosopher eats for random amount of time.
        pick_up(pid);                   //Philosopher picks up forks.
        put_down(pid);                  //Philosopher puts down forks.
        food--;                         //There's less food on the table now.
    }
}

int main() {
    //[USER-INPUT] Defining # philosophers and forks.
    cout << "Define the # of philosophers: ";
    cin >> philosopher;
    cout << endl;
    omp_set_num_threads(philosopher);    

    //Lock Initialization
    omp_init_lock(&pickup);
    omp_init_lock(&putdown);
    for (i = 0; i < philosopher; i++)
        omp_init_lock(&lock[i]);
    
    auto start = Clock::now();
    //Getting the philosophers to start thinking and eating  
    #pragma omp parallel for private(i)
    for (i = 0; i < philosopher; i++)
    {
        action();
    }
    auto finish = Clock::now();
    
    //Time Output
    cout << endl << "Runtime: "
    << chrono::duration_cast<chrono::microseconds>
    (finish - start).count()
    << " microseconds" << endl;
    
    return 0;
}
