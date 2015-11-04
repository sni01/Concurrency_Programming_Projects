/*
 * Implementation of monitor
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include "monitor.h"
 #include "cart.h"
 #include "q.h"

 #define MONITOR_LEN 4
 #define CROSS_TIME 10000000

 pthread_mutex_t pass_lock;
 pthread_cond_t pass_conds[MONITOR_LEN];

 int thread_index;

 char getDirByIndex(int i){
 	switch(i){
 		case 0 : return 'n';
 		case 1 : return 'w';
 		case 2 : return 's';
 		case 3 : return 'e';
 		default : return ' ';
 	}
 }

 int getIndexByDir(char a){
 	switch(a){
 		case 'n' : return 0;
 		case 'w' : return 1;
 		case 's' : return 2;
 		case 'e' : return 3;
 		default : return -1;
 	}
 }

 int nextInt(int a){
 	int tmp = a;
 	a = (a + 1) % MONITOR_LEN;
 	while(q_cartIsWaiting(getDirByIndex(a)) == 0 && tmp != a){
 		a = (a + 1) % MONITOR_LEN;
 	}
 	return a;
 }

 void monitor_init(){
 	thread_index = 0;
 	pthread_mutex_init(&pass_lock, NULL);

 	while(thread_index < MONITOR_LEN){
 		pthread_cond_init(&pass_conds[thread_index], NULL);
 		thread_index++;
 	}

 	thread_index = 0;
 }

 /*
  * If there is a cart in intersection, semaphore blocked
  */
 void monitor_arrive(struct cart_t *cart){
 	int thread_index_cur = getIndexByDir(cart->dir);
 	fprintf(stderr, "cart %i from %c arrived\n", cart->num, cart->dir);

 	pthread_mutex_lock(&pass_lock);
 	while(thread_index != thread_index_cur){
 		fprintf(stderr, "cart %i from %c must be waiting before entering intersection\n", cart->num, cart->dir);
 		pthread_cond_wait(&pass_conds[thread_index_cur], &pass_lock);
 	}
 	fprintf(stderr, "cart %i from %c allowed to proceed into intersection\n", cart->num, cart->dir);
 }

 /*
  * Cross intersection
  */
 void monitor_cross(struct cart_t *cart){ 	
 	fprintf(stderr, "cart %i from %c passing through intersection\n", cart->num, cart->dir);
 	q_cartHasEntered(cart->dir);
 	usleep(CROSS_TIME);
 	thread_index = nextInt(thread_index);
 }

 /*
  * signal semaphore
  */
 void monitor_leave(struct cart_t *cart){
 	fprintf(stderr, "cart %i from %c leaving intersect\n", cart->num, cart->dir);
 	pthread_cond_broadcast(&pass_conds[thread_index]);
 	pthread_mutex_unlock(&pass_lock);
 	free(cart);
 }

 void monitor_shutdown(){
 	int a = 0;
 	pthread_mutex_destroy(&pass_lock);

 	while(a < MONITOR_LEN){
 		pthread_cond_destroy(&pass_conds[a]);
 		a++;
 	}
 }