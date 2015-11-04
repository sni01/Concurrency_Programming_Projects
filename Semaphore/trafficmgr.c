/*
* Traffic Management Main File
* Take input string and create four threads
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "monitor.h"
#include "q.h"
#include "cart.h"

#define THREAD_NUM 4
pthread_barrier_t barrier;

int finishes[THREAD_NUM];

struct queue_info{
	char dir;
	int thread_index;
};

int allFinished(){
  int res = 1;
  int i = 0;
  while(i < THREAD_NUM){
    if(finishes[i] == 0){
      res = 0;
      break;
    }
    i++;
  }
  return res;
}

void *threadFunc(void *arg){
	struct cart_t *cart;
	struct queue_info *info = (struct queue_info *)arg;
	fprintf(stderr, "thread for direction %c starts\n", info->dir);

  pthread_barrier_wait(&barrier);

	cart = q_getCart(info->dir);
	while(1){
    if(allFinished() == 1) break;
    if(cart == NULL){
      finishes[info->thread_index] = 1;
    }
    else{
      fprintf(stderr, "thread for direction %c gets cart %i\n", info->dir, cart->num);
      monitor_arrive(cart);
      monitor_cross(cart);
      monitor_leave(cart);
      cart = q_getCart(info->dir);
    }
	}

	fprintf(stderr, "thread for direction %c exits\n", info->dir);
	pthread_barrier_wait(&barrier);

	return (void *) 1;
}

int isValidChar(char a){
	if(a == Q_NORTH || a == Q_WEST || a == Q_EAST || a == Q_SOUTH) return 1;
	return 1;
}

int main(int argc, char* argv[]){
 	char *carts;
 	int index;
 	pthread_t tids[4];
 	struct queue_info *queue_infos[4];

 	if(argc != 2){
 		fprintf(stderr, "wrong argument number");
 		exit(1);
 	}

 	carts = argv[1];
 	index = 0;
 	q_init();
 	pthread_barrier_init(&barrier, NULL, THREAD_NUM);
  monitor_init();

 	/*
 	 * check input string is valid
 	 */
 	if(strlen(carts) <= 0){
		fprintf(stderr, "input is empty");
		exit(1);
	}

	index = 0;
	while(index < strlen(carts)){
		if(isValidChar(carts[index]) == 0){
			fprintf(stderr, "input direction is invalid");
			exit(1);
		}
		q_putCart(carts[index]);
    thread_index = getIndexByDir(carts[index]);
		index++;
	}

  index = 0;
  while(index < THREAD_NUM){
    finishes[index] = 0;
    index++;
  }

	/*
	 * 0 -> North
	 * 1 -> West
	 * 2 -> South
	 * 3 -> East
	 */
	queue_infos[0] = malloc(sizeof(struct queue_info));
	queue_infos[1] = malloc(sizeof(struct queue_info));
	queue_infos[2] = malloc(sizeof(struct queue_info));
	queue_infos[3] = malloc(sizeof(struct queue_info));
	queue_infos[0]->dir = 'n';
	queue_infos[0]->thread_index = 0;
	queue_infos[1]->dir = 'w';
	queue_infos[1]->thread_index = 1;
	queue_infos[2]->dir = 's';
	queue_infos[2]->thread_index = 2;
	queue_infos[3]->dir = 'e';
	queue_infos[3]->thread_index = 3;

	/*
	 * Create four threads
	 */
	index = 0;
	while(index < 4){
		if(pthread_create(&tids[index], NULL, threadFunc, queue_infos[index]) != 0)
			perror("pthread create error"), exit(EXIT_FAILURE);
		index++;
	}

	index = 0;
	while(index < 4){
		pthread_join(tids[index], NULL);
		index++;
	}

	monitor_shutdown();
  q_shutdown();

	return 1;
}