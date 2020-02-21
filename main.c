#include <stdio.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <stdbool.h>


    #define MAX_CAPACITY 4
    #define ROOM_NUMBER 10
    #define STUDENTS 100

    void *student(void *arg);
    void *roomkeeper(void *arg);

    sem_t roomFull; // eğer oda dolarsa biraz bekleyip oda boşaltılacak
    sem_t incomingStudents;
    sem_t roomController; //odanın dolmasıın kontrol ediyor
    sem_t studentIncomingSemaphor; // binary semaphor
    sem_t roomkeeper_semaphor; // roomkeeperi uyandıran semaphor
    sem_t roomkeeperCleaning; // oda dolunca roomkeeper odayı boşaltıp temizlik yapsın diye


    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int full = 0; // oda dolduğunda 1 olur
    int roomNumberCounter = 0;
    int allDone = 0;
    int roomArray[4] = {0,0,0,0};
    int freePlaceinRoom = 4;

    int main(int argc, char* argv[]){
        //semaphors
        sem_init(&studentIncomingSemaphor, 0, 1);
        sem_init(&roomController,0,4);
        sem_init(&roomkeeper_semaphor,0,0);
        sem_init(&roomkeeperCleaning,0,0);

        //threads
        pthread_t studentsTid[STUDENTS];
        pthread_t roomkeepersTid[ROOM_NUMBER];

        int roomkeepers[ROOM_NUMBER];
        int students[STUDENTS];

        for(int i=0; i< ROOM_NUMBER; i++){
            roomkeepers[i] = i+1;
        }
        for(int i=0; i<STUDENTS; i++){
            students[i] = i+1;
        }
        for(int i=0;i<ROOM_NUMBER;i++){
            pthread_create(&roomkeepersTid[i], NULL, roomkeeper, (void*)&roomkeepers[i]);
        }

        for(int i=0;i<STUDENTS;i++){
                    pthread_create(&studentsTid[i], NULL, student, (void*)&students[i]);
        }
        // join threads to wait for finish.
        for(int i=0;i<STUDENTS;i++){
            pthread_join(studentsTid[i], NULL);
        }

        allDone = 1;

        for(int i=0;i<ROOM_NUMBER;i++){
            pthread_join(roomkeepersTid[i], NULL);
        }

    }
    void *student(void *arg){
        sleep(rand()%2);
        int remaining_place;
        int id = *(int *) arg;
        printf("Student %d is waiting at the library\n", id);
        sem_wait(&studentIncomingSemaphor);
        sleep(1);
          sem_post(&studentIncomingSemaphor);
        if (freePlaceinRoom == 4) {
            sem_post(&roomkeeper_semaphor);
        }
        sem_wait(&roomController);
        pthread_mutex_lock(&mutex);
        roomArray[freePlaceinRoom-1] = id;
        pthread_mutex_unlock(&mutex);
        freePlaceinRoom--;
        if (freePlaceinRoom > 0) {
            printf("ANNOUNCE: I have %d more free places. \n", freePlaceinRoom);
        }
        if (freePlaceinRoom == 0) {
            printf("ANNOUNCE: I have no places left at the room. \n", freePlaceinRoom);
            printf("ANNOUNCE: The students in my room are: %d,%d,%d,%d \n", roomArray[0],roomArray[1],roomArray[2],roomArray[3]);
              sem_post(&roomkeeperCleaning);
            freePlaceinRoom = 4;
        }
        // if(remaining_place==3){
        //     sem_post(&roomkeeper_semaphor); // ilk öğrenci girdikten sonra roomkeeper uyandırıldı. ROOMKEEPER burada duyuru yapacak.
        // }


    }

    void *roomkeeper(void *arg){
        int id = *(int *) arg;
        while(true){
            sem_wait(&roomkeeper_semaphor);
            printf("Room number #%d is open\n",id);
            //odayı boşaltıyoruz tamamen
            for (int i = 0; i < 4; i++) {
                sem_post(&roomController);
            }
            sem_wait(&roomkeeperCleaning);
            printf("ANNOUNCE: Room #%d is full and will be cleaned.\n",id);
        }
    }
