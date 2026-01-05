#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"
#define QUEUE_SIZE 1000
#define SLEEP_TIME (long) 1e5


int durata = 0; //Durata della simulazione in termini di job da creare
int counter = 0; //Quanti job ci sono in coda
int consumati = 0; //Quanti job abbiamo consumato nella simulazione
long cum_turnaround = 0;
int creati = 0; //Quanti job ho creato

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void* produttore(void* arg);
void* consumatore(void* arg);

void* produttore(void* arg) {
	Queue *q = (Queue*)arg;
	while (1) {
		pthread_mutex_lock(&lock);
		if (creati >= durata) {
			pthread_mutex_unlock(&lock);
			break;
		}
		while (counter >= QUEUE_SIZE) {
			pthread_mutex_unlock(&lock);
			sched_yield();
			pthread_mutex_lock(&lock);
		}
		Process *p = malloc(sizeof(Process));
		p->id = creati;
		creati++;
		p->exec_time = random() % (int)1e6;
		gettimeofday(&(p->arrival), NULL);
		enqueue(q, p);
		printf("[PRODUTTORE] Ho creato il job %ld con execTime %ld\n", p->id, p->exec_time);
		counter++;
		pthread_mutex_unlock(&lock);
		usleep(SLEEP_TIME);
	}
	return NULL;
}


void* consumatore(void* arg) {
	Queue *q = (Queue*)arg;
	while (1) {
		pthread_mutex_lock(&lock);
		while (counter == 0 && creati<durata) {
			pthread_mutex_unlock(&lock);
			sched_yield();
			pthread_mutex_lock(&lock);
		}
		if (counter==0 && creati == durata) {
			pthread_mutex_unlock(&lock);
			break;
		}
		Process *p = dequeue(q);
		counter--;
		pthread_mutex_unlock(&lock);
		gettimeofday(&(p->start), NULL);
		usleep(p->exec_time);
		gettimeofday(&(p->end), NULL);
		// T_end - T_arrival
		long ts_arrival = p->arrival.tv_sec * 1e6 + p->arrival.tv_usec;
		long ts_end = p->end.tv_sec * 1e6 + p->end.tv_usec;
		pthread_mutex_lock(&lock);
		consumati++;
		printf("[CONSUMATORE] Ho consumato il job %ld con execTime %ld\n", p->id, p->exec_time);
		cum_turnaround+=(ts_end-ts_arrival);
		pthread_mutex_unlock(&lock);
	}
	return NULL;

}

int main(int argc, char** argv) {

	durata = atoi(argv[1]);
	int ncpu = atoi(argv[2]);

	Queue *q = malloc(sizeof(Queue));
	initializeQueue(q);

	pthread_t p;
	pthread_t c[ncpu];

	pthread_create(&p, NULL, produttore, q);
	for (int i=0; i<ncpu; i++) {
		pthread_create(&c[i], NULL, consumatore, q);
	}

	pthread_join(p, NULL);
	for (int i=0; i<ncpu; i++) {
		pthread_join(c[i], NULL);
	}

	float avg_turnaround = cum_turnaround/(float)durata;
	printf("Turnaround medio: %f secondi\n", avg_turnaround/1e6);

}
