/*
* Read and Write function
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <semaphore.h>
#include <unistd.h>

int max_lines = 10;
int lines_in_buffer = 0;
char **buffer;
size_t bufsize = 1000;
#define QUIT "quit"
sem_t mutex;
sem_t spaces; /* spare buffer to read from file */
sem_t items; /* lines number in buffer */
int next_out;
int next_in;

struct thread_info
{
	/* data */
	FILE *file;
	int sleep;
};

int nextPos(int pos){
	return (pos + 1) % max_lines;
}

void *readThreadFunc(void *arg) {
	struct thread_info *ti = (struct thread_info *) arg;
	int line_accum = 1;
    while(1){
    	/*usleep(ti->sleep);*/
		sem_wait(&spaces);
    	sem_wait(&mutex);
    	next_in = nextPos(next_in);
    	if(getline(&buffer[next_in], &bufsize, ti->file) != -1){
			printf("fill thread: wrote line into buffer (nwritten=%d)\n", line_accum);
			line_accum = line_accum + 1;
		}
		else{
			sprintf(buffer[next_in], QUIT);
			printf("%s\n", "read finishes");
			sem_post(&mutex);
			sem_post(&items);
			return (void *) 1;
		}
		sem_post(&mutex);
    	sem_post(&items);
    }
    return (void *) 1;
}

void *writeThreadFunc(void *arg){
	struct thread_info *ti = (struct thread_info *) arg;
	int line_accum = 1;
	while(1){
		usleep(ti->sleep);
		sem_wait(&items);
		sem_wait(&mutex);
		next_out = nextPos(next_out);
    	if(strcmp(buffer[next_out], QUIT) == 0){
			printf("%s\n", "write finishes.");
			sem_post(&mutex);
			sem_post(&spaces);
			return (void *) 1;
		}
		else{
			int char_pos = 0;
			while(buffer[next_out][char_pos] != '\0'){
				char_pos = char_pos + 1;
			}
			printf("drain thread: read line from buffer (nread=%d)\n", line_accum);
			fwrite(buffer[next_out], sizeof(char), char_pos, ti -> file);
			line_accum = line_accum + 1;
		}
    	sem_post(&mutex);
    	sem_post(&spaces);
    }
    return (void *) 1;
}


int main(int argc, char* argv[]){
	/* declaration */
	int i;
	pthread_t tids[2];
	FILE *input;
	FILE *output;
	int sleep;
	struct thread_info *inputArg;
	struct thread_info *outputArg;
	void *ret1;
	void *ret2;

	if(argc != 4){
		fprintf(stderr, "wrong args number"), exit(0);
		exit(1);
	}

	/*
	 * allocate buffer
	 */
	buffer = (char**)malloc(max_lines * sizeof(char*));
    i = 0;
    while(i < max_lines){
        buffer[i] = (char*)malloc(bufsize * sizeof(char));
        i++;
    }

	/*
	 * initial variables
	 */
    inputArg = malloc(sizeof(struct thread_info));
    outputArg = malloc(sizeof(struct thread_info));

    /*
	 * open files
	 */
	input = fopen(argv[1], "r");
	output = fopen(argv[2], "wr");
	if(input == NULL || output == NULL){
		fprintf(stderr, "open files error"), exit(0);
	}

    sleep = atoi(argv[3]);
	inputArg->file = input;
	inputArg->sleep = sleep;
	outputArg->file = output;
	outputArg->sleep = sleep + 10;
	next_in = -1;
	next_out = -1;

	/*
	 * initial semaphores
	 */
	if (sem_init(&mutex, 0, 1) < 0)
        perror("semaphore initialization"), exit(EXIT_FAILURE);
    if (sem_init(&spaces, 0, max_lines) < 0)
    	perror("semaphore initialization"), exit(EXIT_FAILURE);
    if (sem_init(&items, 0, 0) < 0)
    	perror("semaphore initialization"), exit(EXIT_FAILURE);

	/*
	 * start two threads to read and write
	 */
	if (pthread_create(&tids[0], NULL, readThreadFunc, inputArg) != 0)
        perror("pthread_create"), exit(EXIT_FAILURE);
    if (pthread_create(&tids[1], NULL, writeThreadFunc, outputArg) != 0)
        perror("pthread_create"), exit(EXIT_FAILURE);
	
	/*
	 * main thread waits for other threads return.
	 */
	if (pthread_join(tids[0], &ret1) != 0)
        perror("pthread_join"), exit(EXIT_FAILURE);
    if (pthread_join(tids[1], &ret2) != 0)
        perror("pthread_join"), exit(EXIT_FAILURE);

	/*
	 * de-allocated resources and close files
	 */
	fclose(input);
	fclose(output);
	if (sem_destroy(&mutex) < 0)
        perror("sem_destroy"), exit(EXIT_FAILURE);
    free(buffer);

    return 1;
}


