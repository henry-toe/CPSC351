#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "msg.h"    /* For the message struct */

using namespace std;

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr = NULL;


/**
 * The function for receiving the name of the file
 * @return - the name of the file received from the sender
 */
string recvFileName()
{
    fileNameMsg msg;
    msgrcv(msqid, &msg, sizeof(msg.fileName),
           FILE_NAME_TRANSFER_TYPE, 0);

    return string(msg.fileName);
}

 /**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
    FILE* fp = fopen("keyfile.txt", "w");
    fprintf(fp, "Hello world");
    fclose(fp);

    key_t key = ftok("keyfile.txt", 'a');

    shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    sharedMemPtr = shmat(shmid, NULL, 0);
    if (sharedMemPtr == (void*)-1) { perror("shmat"); exit(1); }

    msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid < 0) { perror("msgget"); exit(1); }
}
 

/**
 * The main loop
 * @param fileName - the name of the file received from the sender.
 * @return - the number of bytes received
 */
unsigned long mainLoop(const char* fileName)
{
	/* The size of the message received from the sender */
	int msgSize = -1;
	
	/* The number of bytes received */
	int numBytesRecv = 0;
	
	/* The string representing the file name received from the sender */
	string recvFileNameStr = fileName;
	
	/* TODO: append __recv to the end of file name */
	
	/* Open the file for writing */
	FILE* fp = fopen(recvFileNameStr.c_str(), "w");
			
	/* Error checks */
	if(!fp)
	{
		perror("fopen");	
		exit(-1);
	}
		
    message rcvMsg;
    ackMessage ack;
	
	/* Keep receiving until the sender sets the size to 0, indicating that
 	 * there is no more data to send.
 	 */	
	while(msgSize != 0)
	{	
        msgrcv(msqid, &rcvMsg, sizeof(int),
               SENDER_DATA_TYPE, 0);

        msgSize = rcvMsg.size;

        if (msgSize != 0)
        {
            numBytesRecv += msgSize;

            fwrite(sharedMemPtr, sizeof(char), msgSize, fp);

            ack.mtype = RECV_DONE_TYPE;
            msgsnd(msqid, &ack, 0, 0);
        }
        else
        {
            fclose(fp);
        }
	}
	
	return numBytesRecv;
}



/**
 * Performs cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    shmdt(sharedMemPtr);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msqid, IPC_RMID, NULL);
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */
void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
	
	/* TODO: Install a signal handler (see signaldemo.cpp sample file).
 	 * If user presses Ctrl-c, your program should delete the message
 	 * queue and the shared memory segment before exiting. You may add 
	 * the cleaning functionality in ctrlCSignal().
 	 */
    signal(SIGINT, ctrlCSignal);

	/* Initialize */
	init(shmid, msqid, sharedMemPtr);
	
	/* Receive the file name from the sender */
	string fileName = recvFileName();
	
	/* Go to the main loop */
	fprintf(stderr, "The number of bytes received is: %lu\n", mainLoop(fileName.c_str()));

	/* TODO: Detach from shared memory segment, and deallocate shared memory 
	 * and message queue (i.e. call cleanup) 
	 */
	cleanUp(shmid, msqid, sharedMemPtr);
		
	return 0;
}
