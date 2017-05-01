/*
 * mutex.c
 *
 *  Created on: Mar 19, 2016
 *      Author: jiaziyi
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREADS 50
void *increase_counter(void *);

int  counter = 0;

int main()
{

	//create 100 threads of increase_counter, each thread adding 1 to the counter

	printf("\nFinal counter value: %d\n", counter);
}

void *increase_counter(void *arg)
{


	printf("Thread number %ld, working on the counter. The current value is %d\n", (long)pthread_self(), counter);
	int i = counter;
//	usleep (10000); //simulate the data processing
	counter = i+1;

}
