#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

typedef struct node {
	char free;
	size_t size;
	struct node *next;
} node;

node* first;


node* search_free_node(size_t size) {
	//Scorro la lista fino ad individuare l'ultimo nodo
        node* ret = NULL;
	node *cur = first;
        while (cur != NULL && (!cur->free || cur->size<size+sizeof(node))) {
		cur = cur->next;
        }

        //Se l'ultimo nodo è nullo, non ho spazio
        if (cur == NULL) {  
                ret = NULL;
        }
	else {
		ret = (node*) cur;
	}
	return ret;
}

void* alloc_in_free_node(node* free_node, size_t size) {
	size_t remaining = free_node->size - size;
	if (remaining > sizeof(node)) {
		node* new_node = (node*) ((void*)free_node+sizeof(node)+size);
		new_node->free = 1;
		new_node->size = remaining - sizeof(node);
		new_node->next = free_node->next;
		free_node->next = new_node;
		free_node->size = size;
	}
	free_node->free=0;
	return (void*)free_node+sizeof(node);
}


void* my_malloc(size_t size) {
	void* ret;
	node* free_node = search_free_node(size);
	if (free_node == NULL) {
		ret = NULL;
	} else {
		ret = alloc_in_free_node(free_node, size);
	}
	return ret;
}

void my_free(void* ptr) {
	if (ptr == NULL) return;
	node* to_free = (node*) (ptr-sizeof(node));
	to_free->free = 1;

}

void print_memory() {
	node* cur = first;
	int i = 0;
	printf("--------Inizio stampa memoria-------\n");
	while (cur != NULL) {
		printf("Area: %d\t Free: %d\t Size: %lu\n", i, cur->free, cur->size);
		i++;
		cur = cur->next;
	}
	printf("--------Fine stampa memoria-------\n\n");

}

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Sintassi: my_malloc <dimensione>");
	}
	size_t size = atoi(argv[1]);
	void* mem = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	first = (node*) mem;


	first->free = 1;
	first->size = size-sizeof(node);
	first->next = NULL;

	print_memory();


	void* ptr1 = my_malloc(10);
	void* ptr2 = my_malloc(20);
	void* ptr3 = my_malloc(30);

	print_memory();


	my_free(ptr3);

	print_memory();

	void* ptr4 = my_malloc(1);

	print_memory();

}
