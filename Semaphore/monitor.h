/*
 * Monitor header file
 */
 #ifndef _MONITOR_
 #define _MONITOR_

 #include "cart.h"

 extern int thread_index;
 void monitor_init();
 void monitor_arrive(struct cart_t *cart);
 void monitor_cross(struct cart_t *cart);
 void monitor_leave(struct cart_t *cart);
 void monitor_shutdown();
 extern int getIndexByDir(char c);

 #endif