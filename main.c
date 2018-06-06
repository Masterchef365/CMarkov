#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORD_SIZE 500
#define MAX_SUBWORDS 50
#define SHOW_TREE 0

/* A word on the markov tree */
typedef struct Node {
	char text[MAX_WORD_SIZE]; /* The original text of the word */
	unsigned int hash; /* The hash of the word */
	int subword_idx; /* How many subwords have we assigned? */
	int subwords[MAX_SUBWORDS]; /* List of subwords */
} Node;

/* Convert a string into a unique hash */
unsigned int str_hash(char* str) {
	unsigned int hash = 7890;
	int c;
	while (c = *str++) hash += (hash << 5) + c;
	return hash;
}

/* Allocate and construct a new node */
Node* create_node(char* word, unsigned int hash) {
	Node* ptr = calloc(1, sizeof(Node));
	ptr->subword_idx = -1;
	strcpy(ptr->text, word);
	memset(ptr->subwords, 0, sizeof(int) * MAX_SUBWORDS);
	ptr->hash = hash;
	return ptr;
}

/* Append a subword to a node if it doesn't already exist */
void find_or_create_subword(Node* node, int idx) {
	for (int i = 0; i <= node->subword_idx; i++)
		if (node->subwords[i] == idx) return;
	if (node->subword_idx < MAX_SUBWORDS)
		node->subwords[++node->subword_idx] = idx;
}

/* Either find a node, or create it */
unsigned int find_or_create_node(char* word, Node* nodes[], unsigned int* len) {
	unsigned int hash = str_hash(word);
	for (int i = 0; i <= *len; i++)
		if (nodes[i]->hash == hash) return i;
	nodes[++*len] = create_node(word, hash);
	return *len;
}

/* Collect a single ' ' bound word from `src` */
/* TODO: Handle over-sized words, use buffering for higher performance */
int get_word (FILE* src, char* out_buf) {
	int ch;	
	while ((ch = fgetc(src)) != EOF)
	{
		if (ch == ' ') {
			*out_buf++ = '\0';
			return 1;
		} else {
			*out_buf++ = ch;
		}
	}
	return 0;
}

int main (int argc, char** argv) {
	/* Warn user of incorrect usage */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <seed word> <num_words:-1> \n", argv[0]);	
		exit(EXIT_FAILURE);
	}

	/* Max words to output (-1 is infinite) */
	int max_words = -1;
	if (argc == 3) {
		max_words = atoi(argv[2]);
	}

	/* Initialize random seed for markov chain */
	srand(time(NULL));

	int node_size = 1000; /* How many nodes can the tree contain? */
	int node_idx = 0; /* What node is the last on the chain? */
	Node** nodes = calloc(node_size, sizeof(Node*));

	/* Buffer for holding the current word */
	char wordbuf[MAX_WORD_SIZE];

	/* Prime the last(first) word */
	get_word(stdin, wordbuf);
	int last_word = 0; 
	nodes[last_word] = create_node(wordbuf, str_hash(wordbuf));

	/* Add nodes to the graph */
	while (get_word(stdin, wordbuf)) {
		/* Expand the array if we run out of room */
		if (node_idx == node_size - 1) {
			node_size += 2000;
			nodes = realloc(nodes, node_size * sizeof(Node*)); /*TODO: Handle OOM */
			fprintf(stderr, "Realloc: %i\n", node_size);
		}

		/* Add another node to the graph */
		if (strlen(wordbuf) > 0) {
			int current_word = find_or_create_node(wordbuf, nodes, &node_idx);
			if (last_word != current_word) find_or_create_subword(nodes[last_word], current_word);
			last_word = current_word;
		}
	}

	/* Print chains */
#if SHOW_TREE
	for (int i = 0; i <= node_idx; i++) {
		Node* node = nodes[i];
		printf("(%s)\n", node->text);
		for (int j = 0; j <= node->subword_idx; j++) {
			int subword = node->subwords[j];
			printf("\t%s\n", nodes[subword]->text);
		}
	}
#endif

	/* Follow the markov chain! */
#if !SHOW_TREE
	int idx = find_or_create_node(argv[1], nodes, &node_idx);
	int word_count = 0;
	while(1) {
		Node* node = nodes[idx];
		if (nodes[idx]->subword_idx >= 0) {
			int rand_idx = node->subword_idx > 0 ? rand() % node->subword_idx + 1 : 0;
			idx = node->subwords[rand_idx];
			printf("%s ", nodes[idx]->text);
			word_count++;
		} else {
			idx = rand() % node_idx;
		}
		if (word_count > max_words && max_words != -1) {
			break;
		}
	}
#endif

	putchar('\n');

	/* Free allocations */
	for (int i = 0; i <= node_idx; i++) free(nodes[i]);
	free(nodes);
	exit(EXIT_SUCCESS);
}
