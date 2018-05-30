#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_SIZE 100
#define MAX_RANKS 50

unsigned int str_hash(char* str) {
	unsigned int hash = 7890;
	int c;
	while (c = *str++) hash += (hash << 5) + c; /* hash * 33 + c */

	return hash;
}

typedef struct WordRank {
	size_t word_idx;
	unsigned int rank;
} WordRank;

typedef struct WordNode {
	char original[MAX_WORD_SIZE];	
	unsigned int hash;
	size_t rank_idx;
	WordRank ranks[MAX_RANKS];
} WordNode;

WordNode* create_node(char* word) {
	WordNode* ptr = calloc(1, sizeof(WordNode));
	strcpy(ptr->original, word);
	ptr->hash = str_hash(word);
	return ptr;
}

int main () {
	WordNode* nodes[1000];
	size_t node_size = 1000;
	size_t node_idx = 0;

	int ch;	
	char wordbuf[MAX_WORD_SIZE];
	char* wordbuf_ptr = wordbuf;
	//TODO: use sscanf instead?
	while ((ch = getchar()) != EOF) {
		if (ch == ' ') {
			*wordbuf_ptr++ = '\0';
			nodes[node_idx++] = create_node(wordbuf);
			wordbuf_ptr = wordbuf;
		} else {
			*wordbuf_ptr++ = ch;
		}
	}

	for (int i = 0; i < node_idx; i++) {
		WordNode* node = nodes[i];
		printf("%s: %u\n", node->original, node->hash);
		for (int j = 0; j < node->rank_idx; j++) {
			WordRank rank = node->ranks[j];
			printf("\t%s: %zu", nodes[rank.word_idx]->original, rank.rank);
		}
	}
}
