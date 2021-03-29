/*
Works Cited:
(1) Mutex Implementation: https://github.com/jenny-codes/operating-systems/blob/master/06-synchronization/dining_philosophers.c
(2) Semaphore Implementation: https://www.geeksforgeeks.org/dining-philosopher-problem-using-semaphores/
(3) Solution Outline: https://www.youtube.com/watch?v=syMOLWlGjNg
(4) Another Solution Outline: https://legacy.cs.indiana.edu/classes/p415-sjoh/hw/project/dining-philosophers/index.htm
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
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
int philosopher, pid[100], state[100];  //Defining philosophers, their IDs, and their states.
pthread_mutex_t pickup, putdown;        //2 mutex locks for picking up and putting down
sem_t sem[100];                         //Semaphore initialization
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
        usleep(rand()  % 3 + 1);         //Philosopher eats for random amount of time.
    /*  sleep(3);                       //Philosopher eats for fixed time (1c implementation) */
        cout << "Philosopher " << pid
        << " done eating..." << endl;   //Philosopher declares they're eating
        sem_post(&sem[pid]);            //Fork is no longer occupied announcement.
    }
}

//Pick Up Fork Function
void pick_up(int pid) {
    pthread_mutex_lock(&pickup);        //Start picking up fork
    cout << "Philosopher " << pid 
    << " hungry..." << endl;            //Philosopher declares they're hungry.
    state[pid] = hungry;                //Philosopher is hungry now.
    check(pid);                         //Check if left and right philosophers are eating.
    pthread_mutex_unlock(&pickup);      //Finish picking up fork.
    sem_wait(&sem[pid]);                //Fork is now occupied announcement.
}

//Put Down Fork Function (Attempt at acyclic setup for 1a extra credit)
void put_down(int pid) {
    pthread_mutex_lock(&putdown);       //Start putting down fork.
    if (pid % 2 == 0) {                 //If its an even-numbered Philosopher (1a implementation)
        check(right);                   //Wake up & ask if right Philosopher needs fork
        check(left);                    //Wake up & ask if left Philosopher needs fork
    }
    else {                              //If its an odd-numbered Philosopher
        check(left);                    //Wake up & ask if left Philosopher needs fork
        check(right);                   //Wake up & ask if right Philosopher needs fork
    }
    state[pid] = thinking;
    pthread_mutex_unlock(&putdown);
}

//Philosopoher Function
void *action(void* tids) {
    while (food > 0) {                   //While threads are active
        int *pid = (int*) tids;
        cout << "Philosopher " << *pid
             << " thinking..." << endl;  //Philosopher thinks
        usleep(rand() % 3 + 1);          //Philosopher thinks for random amount of time.
    /*  sleep(3);                        //Philosopher thinks for fixed tmi (1c implementation)*/
        pick_up(*pid);                   //Philosopher picks up forks.
        put_down(*pid);                  //Philosopher puts down forks.
        food--;
    }
}

int main() {
    //[USER-INPUT] Defining # philosophers and forks.
    cout << "Define the # of philosophers: ";
    cin >> philosopher;
    cout << endl;
    
    //Initialize Philosopher IDs
    for (int i = 0; i < philosopher; i++)
        pid[i] = i + 1;
        
    //Initialize the semaphores (always initiate before Pthreads!)
    for (int i = 0; i < philosopher; i++)
        sem_init(&sem[i], 0, 0);
        
    //Initialize Pthread IDs to be used
    pthread_t tids[philosopher];
    
    auto start = Clock::now();
    //Generating Philosophers as Pthreads
    for (long i = 0; i < philosopher; i++)
        pthread_create(&tids[i], NULL, action, &pid[i]);
    
    //Making Philosophers go away when done eating
    for (long i = 0; i < philosopher; i++)
        pthread_join(tids[i], NULL);
    auto finish = Clock::now();
    
    //Time Output
    cout << endl << "Runtime: "
    << chrono::duration_cast<chrono::microseconds>
    (finish - start).count()
    << " microseconds" << endl;
        
    //Exit line in case finish condition is decided upon.
    pthread_exit(NULL);
    
    return 0;
}
