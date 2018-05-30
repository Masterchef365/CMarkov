#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

//TODO: Create conditional realloc based on whether or not an index is at an end?
//Maybe avoid this by realloc'ing BEFORE any function calls are made

#define MAX_WORD_SIZE 100
#define MAX_RANKS 50

//https://stackoverflow.com/questions/7666509/hash-function-for-string#7666577
unsigned int str_hash(char* str) {
	unsigned int hash = 7890;
	int c;
	while (c = *str++) hash += (hash << 5) + c; /* hash * 33 + c */
	return hash;
}

typedef struct WordRank {
	int word_idx;
	unsigned int rank;
} WordRank;

typedef struct WordNode {
	char original[MAX_WORD_SIZE];	
	unsigned int hash;
	int rank_idx;
	WordRank ranks[MAX_RANKS];
} WordNode;

/* Search for a rank entry matching our input index and accumulate it. If it does not exist, create it */
void create_or_accumulate_rank(WordNode* node, int idx) {
	/* Search an accumulate */
	for (int i = 0; i <= node->rank_idx; i++) {
		WordRank* rank = &node->ranks[i];
		if (rank->word_idx == idx) {
			rank->rank++;	
			return;
		}
	}

	/* Not found, creat it */
	if (node->rank_idx < MAX_RANKS) {
		node->rank_idx += 1;
		node->ranks[node->rank_idx].word_idx = idx;
		node->ranks[node->rank_idx].rank = 0;
	}
}

WordNode* create_node(char* word, unsigned int hash) {
	WordNode* ptr = calloc(1, sizeof(WordNode));
	ptr->rank_idx = -1;
	strcpy(ptr->original, word);
	memset(ptr->ranks, 0, sizeof(WordRank) * MAX_RANKS);
	ptr->hash = hash;
	return ptr;
}

unsigned int find_or_create(char* word, WordNode* nodes[], unsigned int* len) {
	unsigned int hash = str_hash(word);
	for (int i = 0; i <= *len; i++) {
		if (nodes[i]->hash == hash)	{
			return i;
		}
	}
	*len += 1;
	nodes[*len] = create_node(word, hash);
	return *len;
}

int get_word (char* out_buf) {
	int ch;	
	while ((ch = getchar()) != EOF) {
		if (isspace(ch)) {
			*out_buf++ = '\0';
			return 1;
		} else {
			if (isalnum(ch)) {
				*out_buf++ = ch;
			}
		}
	}
	return 0;
}

int main (int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s \'starting word\'\n", argv[0]);	
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	WordNode* nodes[10000]; //TODO: MAKE IT EXPANDABLE (man gcc doesn't fit)
	int node_size = 10000;
	int node_idx = 0;

	char wordbuf[MAX_WORD_SIZE];

	/* Prime the last(first) word */
	get_word(wordbuf);
	int last_word = 0; 
	nodes[last_word] = create_node(wordbuf, str_hash(wordbuf));

	/* Add nodes to the graph */
	while (get_word(wordbuf)) {
		if (strlen(wordbuf) > 0) {
			int current_word = find_or_create(wordbuf, nodes, &node_idx);
			if (last_word != current_word) create_or_accumulate_rank(nodes[last_word], current_word);
			last_word = current_word;
		}
	}

	/* Print chains */
	for (int i = 0; i <= node_idx; i++) {
		WordNode* node = nodes[i];
		printf("(%s)\n", node->original);
		for (int j = 0; j <= node->rank_idx; j++) {
			WordRank rank = node->ranks[j];
			printf("\t%s: %zu\n", nodes[rank.word_idx]->original, rank.rank);
		}
	}

	int idx = find_or_create(argv[1], nodes, &node_idx);
	while(1) {
		WordNode* node = nodes[idx];
		if (nodes[idx]->rank_idx >= 0) {
			int rand_idx = node->rank_idx > 0 ? rand() % node->rank_idx + 1 : 0;
			idx = node->ranks[rand_idx].word_idx;
			//fprintf(stderr, "(%i) %s ", rand_idx, nodes[idx]->original);
			fprintf(stderr, "%s ", nodes[idx]->original);
		} else {
			//idx = rand() % node_idx;
			break;
		}
	}

	putchar('\n');

	/* Free allocations */
	for (int i = 0; i <= node_idx; i++) free(nodes[i]);
	exit(EXIT_SUCCESS);
}
