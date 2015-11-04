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
sem_t lock;
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
    	usleep(ti->sleep);
    	sem_wait(&lock);
    	if(lines_in_buffer < max_lines){
    		next_in = nextPos(next_in);
    		if(getline(&buffer[next_in], &bufsize, ti->file) != -1){
				printf("fill thread: wrote line into buffer (nwritten=%d)\n", line_accum);
				lines_in_buffer = lines_in_buffer + 1;
				line_accum = line_accum + 1;
			}
			else{
				sprintf(buffer[next_in], QUIT);
				printf("%s\n", "read finishes");
				lines_in_buffer = lines_in_buffer + 1;
				sem_post(&lock);
				return (void *) 1;
			}
    	}
    	else{
    		printf("%s\n", "fill thread: could not write -- not enough space");
    	}
    	sem_post(&lock);
    }
    return (void *) 1;
}

void *writeThreadFunc(void *arg){
	struct thread_info *ti = (struct thread_info *) arg;
	int line_accum = 1;
	while(1){
		/*usleep(ti->sleep);*/
		sem_wait(&lock);
    	if(lines_in_buffer > 0){
    		int char_pos = 0;
    		next_out = nextPos(next_out);
			if(strcmp(buffer[next_out], QUIT) == 0){
				printf("%s\n", "write finishes.");
				sem_post(&lock);
				return (void *) 1;
			}
			while(buffer[next_out][char_pos] != '\0'){
				char_pos = char_pos + 1;
			}
			printf("drain thread: read line from buffer (nread=%d)\n", line_accum);
			fwrite(buffer[next_out], sizeof(char), char_pos, ti -> file);
			line_accum = line_accum + 1;
			lines_in_buffer = lines_in_buffer - 1;
    	}
    	else{
    		printf("%s\n", "no new string in buffer");
    	}
    	sem_post(&lock);
    }
    return (void *) 1;
}


int main(int argc, char* argv[]){
	int i;
	pthread_t tids[2];
	FILE *input;
	FILE *output;
	int sleep;
	void *ret1;
	void *ret2;
	struct thread_info *inputArg;
	struct thread_info *outputArg;


	if(argc != 4){
		fprintf(stderr, "wrong args number"), exit(0);
		exit(1);
	}

	/*
	 * allocate buffer
	 */
	i = 0;
	buffer = (char**)malloc(max_lines * sizeof(char*));
    while(i < max_lines){
        buffer[i] = (char*)malloc(bufsize * sizeof(char));
        i++;
    }

	/*
	 * initial variables
	 */
	inputArg = malloc(sizeof(struct thread_info));
	outputArg = malloc(sizeof(struct thread_info));

	input = fopen(argv[1], "r");
	output = fopen(argv[2], "wr");
	sleep = atoi(argv[3]);
	inputArg->file = input;
	inputArg->sleep = sleep;
	outputArg->file = output;
	outputArg->sleep = sleep + 10;
	next_in = -1;
	next_out = -1;

	if (sem_init(&lock, 0, 1) < 0)
        perror("semaphore initialization"), exit(EXIT_FAILURE);

	if(input == NULL || output == NULL){
		fprintf(stderr, "open files error"), exit(0);
	}

	if (pthread_create(&tids[0], NULL, readThreadFunc, inputArg) != 0)
        perror("pthread_create"), exit(EXIT_FAILURE);
    if (pthread_create(&tids[1], NULL, writeThreadFunc, outputArg) != 0)
        perror("pthread_create"), exit(EXIT_FAILURE);
	
	if (pthread_join(tids[0], &ret1) != 0)
        perror("pthread_join"), exit(EXIT_FAILURE);
    if (pthread_join(tids[1], &ret2) != 0)
        perror("pthread_join"), exit(EXIT_FAILURE);

	fclose(input);
	fclose(output);
	if (sem_destroy(&lock) < 0)
        perror("sem_destroy"), exit(EXIT_FAILURE);
    free(buffer);

    return 1;
}


