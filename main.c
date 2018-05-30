#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORD_SIZE 500
#define MAX_SUBWORDS 50

/*https://stackoverflow.com/questions/7666509/hash-function-for-string#7666577 */
unsigned int str_hash(char* str) {
	unsigned int hash = 7890;
	int c;
	while (c = *str++) hash += (hash << 5) + c; /* hash * 33 + c */
	return hash;
}

typedef struct WordNode {
	char original[MAX_WORD_SIZE];	/*The input word */
	unsigned int hash; /*The hash of the word */
	int subword_idx; /*How many subwords have we assigned? */
	int subwords[MAX_SUBWORDS]; /*List of subwords */
} WordNode;

/* Search for a subword entry matching our input index and accumulate it. If it does not exist, create it */
void create_or_accumulate_subword(WordNode* node, int idx) {
	/* Search and accumulate */
	for (int i = 0; i <= node->subword_idx; i++) {
		int* subword = &node->subwords[i];
		if (*subword == idx) return;
	}

	/* Not found, create it */
	if (node->subword_idx < MAX_SUBWORDS) { /*TODO: Expandable? */
		node->subwords[++node->subword_idx] = idx;
	}
}

/* Allocate and construct a new node */
WordNode* create_node(char* word, unsigned int hash) {
	WordNode* ptr = calloc(1, sizeof(WordNode));
	ptr->subword_idx = -1;
	strcpy(ptr->original, word);
	memset(ptr->subwords, 0, sizeof(int) * MAX_SUBWORDS);
	ptr->hash = hash;
	return ptr;
}

/* Find or create a node in a wordtree */
unsigned int find_or_create(char* word, WordNode* nodes[], unsigned int* len) {
	unsigned int hash = str_hash(word);
	for (int i = 0; i <= *len; i++) {
		if (nodes[i]->hash == hash)	{
			return i;
		}
	}
	nodes[++*len] = create_node(word, hash);
	return *len;
}

/*TODO: Handle over-length words */
int get_word (char* out_buf) {
	int ch;	
	while ((ch = fgetc(stdin)) != EOF) {
		if (isspace(ch)) {
			/* Finish the word */
			*out_buf++ = '\0';
			return 1;
		} else {
			/*TODO: Check if the letter is non-text */
			*out_buf++ = ch;
		}
	}
	return 0;
}

int main (int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <starting word> <num_words:-1> \n", argv[0]);	
		exit(EXIT_FAILURE);
	}
	
	int max_words = -1;
	if (argc == 3) {
		max_words = atoi(argv[2]);
	}

	srand(time(NULL));

	int node_size = 1000;
	WordNode** nodes = calloc(node_size, sizeof(WordNode*));
	int node_idx = 0;

	char wordbuf[MAX_WORD_SIZE];

	/* Prime the last(first) word */
	get_word(wordbuf);
	int last_word = 0; 
	nodes[last_word] = create_node(wordbuf, str_hash(wordbuf));

	/* Add nodes to the graph */
	while (get_word(wordbuf)) {
		/* Expand the array if we run out of room */
		if (node_idx == node_size - 1) {
			node_size += 2000;
			nodes = realloc(nodes, node_size * sizeof(WordNode*)); /*TODO: Handle OOM */
			fprintf(stderr, "Realloc: %i\n", node_size);
		}

		/* Add another node to the graph */
		if (strlen(wordbuf) > 0) {
			int current_word = find_or_create(wordbuf, nodes, &node_idx);
			if (last_word != current_word) create_or_accumulate_subword(nodes[last_word], current_word);
			last_word = current_word;
		}
	}

	/* Print chains */
	/*
	for (int i = 0; i <= node_idx; i++) {
		WordNode* node = nodes[i];
		printf("(%s)\n", node->original);
		for (int j = 0; j <= node->subword_idx; j++) {
			int subword = node->subwords[j];
			printf("\t%s: %zu\n", nodes[subword]->original, subword);
		}
	}
	*/

	/* Follow the markov chain! */
	int idx = find_or_create(argv[1], nodes, &node_idx);
	int word_count = 0;
	while(1) {
		WordNode* node = nodes[idx];
		if (nodes[idx]->subword_idx >= 0) {
			int rand_idx = node->subword_idx > 0 ? rand() % node->subword_idx + 1 : 0;
			idx = node->subwords[rand_idx];
			fprintf(stderr, "%s ", nodes[idx]->original);
			word_count++;
		} else {
			idx = rand() % node_idx;
			/*break; */
		}
		if (word_count > max_words && max_words != -1) {
			break;
		}
	}

	putchar('\n');

	/* Free allocations */
	for (int i = 0; i <= node_idx; i++) free(nodes[i]);
	free(nodes);
	exit(EXIT_SUCCESS);
}
