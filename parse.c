#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#if 0
#include <sys/types.h>
/*
 * This prototype should be in stdio.h, but GCC warns of an implicit
 * declaration without it. This might be a particular problem of the
 * system header files of the developer (Debian Wheezy).
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif


/* Size of line buffer for the parsing of rules */
#define SIZ_BUF_RULE 4

/* Size of line buffer for the parsing of input lines */
#define SIZ_BUF_LINE 1024


/* TYPES *********************************************************** */

/* Type for productions matrices */
typedef bool ***P_t;


/* Type and struct definitions for a stack of fixed-size strings */
typedef struct _node {
	char value[SIZ_BUF_RULE];
	struct _node *next;
} node;

typedef struct {
	node *head;
} stack;


/* GLOBALS ********************************************************** */

/* Constants */
const char SIZ_SIMP = 2;  /* Size for a simple production rule   */
const char SIZ_COMP = 3;  /* Size for a compound production rule */
const char SIZ_ALPH = 26; /* Alphabet size = max number of rules */

/* Number of production rules */
int r_simp = 0; /* Simple rules:   A -> a  */
int r_comp = 0; /* Compound rules: A -> BC */

/* Simple rules array */
char **P_simp;

/* Compound rules array */
char **P_comp;

/* Initial nonterminal */
char init;


/* STACK METHODS **************************************************** */

stack *stk_new() {
	stack *stk = (stack *) malloc(sizeof(stack));
	stk->head = NULL;
	return stk;
}

void stk_push(stack *stk, char *value) {
	node *new  = malloc(sizeof(node));
	strncpy(new->value, value, SIZ_BUF_RULE);
	new->next  = stk->head;
	stk->head  = new;
}

void stk_pop(stack *stk, char *value) {
	node *n   = stk->head;

	value[0] = '\0';
	if (!n) return;

	strncpy(value, n->value, SIZ_BUF_RULE);
	stk->head = n->next;
	free(n);
}


/* PRODUCTIONS MATRIX METHODS *************************************** */

/*
 * Create and return a new productions matrix ready to be used for the
 * parsing of a string of length `n'.
 */

P_t P_new(int n) {
	int i, j, k;

	/* Allocation */
	bool ***P = malloc(sizeof(void *) * (n + 1));
	for (i = 0; i < n + 1; i++) {
		P[i] = malloc(sizeof(void *) * (n + 1));
		for (j = 0; j < n + 1; j++) {
			P[i][j] = malloc(sizeof(bool) * SIZ_ALPH);
		}
	}

	/* Initialization */
	for (i=0; i<=n; i++)
		for (j=0; j<=n; j++)
			for (k=0; k<SIZ_ALPH; k++)
				P[i][j][k] = false;

	return P;
}


void P_free(P_t P, int n) {
	int i, j;

	for (i = 0; i < n + 1; i++) {
		for (j = 0; j < n + 1; j++) {
			free(P[i][j]);
		}
		free(P[i]);
	}
	free(P);
}


/* MISC ************************************************************* */

/* Convert chars A..Z to ints 0..26 */
int ctoi(char c) {
	return c - 65;
}


#if DEBUG
void print_matrix(bool ***P, int n) {
	int i, j, k;

	for (i = 1; i <= n; i++)
		for (int j = 1; j <= n; j++)
			for (int k = 0; k < SIZ_ALPH; k++)
				printf("%d\t%d\t%c\t%s\n",
						i, j, k+65, P[i][j][k] ? "YES" : "");
}
#endif



/* PARSER CONSTRUCTION ********************************************** */

void build() {

	int    i;
	int    lin_siz;
	size_t buf_siz    = SIZ_BUF_RULE;
	char  *line       = (char *) malloc(buf_siz);
	bool   first_line = true;
	stack *lines_simp = stk_new();
	stack *lines_comp = stk_new();

	/* Read every production rule and keep it in a stack */

	while ((lin_siz = getline(&line, &buf_siz, stdin)) != EOF) {
		bool stop = false;

		if (first_line) {
			init = line[0];
#if DEBUG
			printf("FIRST IS %c\n", init);
#endif
			first_line = false;
		}

		switch (lin_siz) {
			/* Compound rule */
			case 4:
#if DEBUG
				printf("comp");
#endif
				stk_push(lines_comp, line);
				r_comp++;
				break;

			/* Simple rule */
			case 3:
#if DEBUG
				printf("simp");
#endif
				stk_push(lines_simp, line);
				r_simp++;
				break;

			/* Break character */
			case 2: stop = true; break;
		}

		if (stop) break;
#if DEBUG
		printf("\t%s", line);
#endif
	}

	free(line);

	/* Allocate memory for rules arrays */

	P_simp = malloc(sizeof(void *) * (r_simp + 1));
	for (i = 0; i < r_simp + 1; i++)
		P_simp[i] = malloc(sizeof(char) * SIZ_SIMP);

	P_comp = malloc(sizeof(void *) * (r_comp + 1));
	for (i = 0; i < r_comp + 1; i++)
		P_comp[i] = malloc(sizeof(char) * SIZ_COMP);

	/* Populate rules arrays from stacks */

	for (i = 1; i <= r_simp; i++) {
		char line[SIZ_BUF_RULE];
		stk_pop(lines_simp, line);
		strncpy(P_simp[i], line, SIZ_SIMP);
	}

	for (i = 1; i <= r_comp; i++) {
		char line[SIZ_BUF_RULE];
		stk_pop(lines_comp, line);
		strncpy(P_comp[i], line, SIZ_COMP);
	}
}


/* PARSING ********************************************************** */

void parse(char *input) {
	int i, j, k, l;

	/* Number of terminals in string */
	int n = strlen(input);

	/* Productions matrix */
	bool ***P = P_new(n);

	/* Handle simple productions */
	for (i = 1; i <= n; i++)
		for (j = 1; j <= r_simp; j++)
			/* R_j -> a_i */
			if (P_simp[j][1] == input[i-1]) {
				int rule = ctoi(P_simp[j][0]);
				P[i][1][rule] = true;
			}

	/* Handle compound productions */
	for (i = 2; i <= n; i++)
		for (j = 1; j <= n - i + 1; j++)
			for (k = 1; k <= i - 1; k++)
				for (l = 1; l <= r_comp; l++) {
					/* R_A -> R_B R_C */
					int A = ctoi(P_comp[l][0]);
					int B = ctoi(P_comp[l][1]);
					int C = ctoi(P_comp[l][2]);
					if (P[j][k][B] && P[j+k][i-k][C])
						P[j][i][A] = true;
				}

#if DEBUG
	print_matrix(P, n);
#endif

	if (P[1][n][ctoi(init)])
		puts("yes");
	else
		puts("no");

	P_free(P, n);
}


void parse_lines() {
	size_t  buf_siz = SIZ_BUF_LINE;
	char   *line    = (char *) malloc(buf_siz);
	char    input[SIZ_BUF_LINE];

	while (getline(&line, &buf_siz, stdin) != EOF) {
		int size = strlen(line) - 1;

		if (strcmp(line, "#\n") == 0) break;

		strncpy(input, line, size);
		input[size] = '\0';

#if DEBUG
		puts(input);
#endif
		parse(input);
	}
	free(line);
}
	

/* LET'S DO THIS **************************************************** */

int main() {

	build();
	parse_lines();

	return 0;
}

/* vim: set ft=c ts=4 tw=78 fdm=marker fdl=0 fen :*/
