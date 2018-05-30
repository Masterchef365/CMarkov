#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void create_or_append_rank(WordNode* node, unsigned int idx) {
	for (int i = 0; i < node->rank_idx; i++) {
		WordRank* rank = &node->ranks[i];
		if (rank->word_idx == idx) {
			rank->rank++;	
			return;
		}
	}
	node->rank_idx += 1;
	node->ranks[node->rank_idx].word_idx = idx;
	node->ranks[node->rank_idx].rank = 0;
}

//TODO: Create conditional realloc based on whether or not an index is at an end?
//Maybe avoid this by realloc'ing BEFORE any function calls are made

WordNode* create_node(char* word, unsigned int hash) {
	WordNode* ptr = calloc(1, sizeof(WordNode));
	ptr->rank_idx = -1;
	strcpy(ptr->original, word);
	memset(ptr->ranks, 0, sizeof(WordRank) * MAX_RANKS);
	ptr->hash = hash;
	return ptr;
}

unsigned int find_idx(char* word, WordNode* nodes[], unsigned int* len) {
	unsigned int hash = str_hash(word);
	for (int i = 0; i < *len; i++) {
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
		if (ch == ' ') { //TODO: use `is_space()`?
			*out_buf++ = '\0';
			return 1;
		} else {
			*out_buf++ = ch;
		}
	}
	return 0;
}

int main () {
	WordNode* nodes[1000];
	unsigned int node_size = 1000;
	unsigned int node_idx = 0;

	char wordbuf[MAX_WORD_SIZE];

	/* Prime the last(first) word */
	get_word(wordbuf);
	unsigned int last_word = 0; 
	nodes[last_word] = create_node(wordbuf, str_hash(wordbuf));

	while (get_word(wordbuf)) {
		unsigned int current_word = find_idx(wordbuf, nodes, &node_idx);
		create_or_append_rank(nodes[last_word], current_word);
		printf("%s: %zu\n", nodes[last_word]->original, last_word);
		last_word = current_word;
	}
	printf("%s: %zu\n", nodes[last_word]->original, last_word);

	/* Print and free memory */
	for (int i = 0; i <= node_idx; i++) {
		WordNode* node = nodes[i];
		printf("%s: %u, %i\n", node->original, node->hash, node->rank_idx);
		for (int j = 0; j <= node->rank_idx; j++) {
			WordRank rank = node->ranks[j];
			printf("\t%s: %zu\n", nodes[rank.word_idx]->original, rank.rank);
		}
	}
	for (int i = 0; i <= node_idx; i++) free(nodes[i]);
}
