#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


/*
 * This prototype should be in stdio.h, but GCC warns of an implicit
 * declaration without it. This might be a particular problem of the
 * system header files of the developer (Debian Wheezy).
 */
int getline(char **lineptr, int *n, FILE *stream);


/* Size of line buffer for the parsing of rules */
#define SIZ_BUF 4


/* TYPES *********************************************************** */

/* Type for productions matrices */
typedef bool ***P_t;


/* Type and struct definitions for a stack of fixed-size strings */
typedef struct _node {
	char value[SIZ_BUF];
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
	strncpy(new->value, value, SIZ_BUF);
	new->next  = stk->head;
	stk->head  = new;
}

void stk_pop(stack *stk, char *value) {
	node *n   = stk->head;

	value[0] = '\0';
	if (!n) return;

	strncpy(value, n->value, SIZ_BUF);
	stk->head = n->next;
	free(n);
}


/* PRODUCTIONS MATRIX METHODS *************************************** */

/*
 * Create and return a new productions matrix ready to be used for the
 * parsing of a string of length `n'.
 */

P_t P_new(int n) {

	/* Allocation */
	bool ***P = malloc(sizeof(void *) * (n + 1));
	for (int i = 0; i < n + 1; i++) {
		P[i] = malloc(sizeof(void *) * (n + 1));
		for (int j = 0; j < n + 1; j++) {
			P[i][j] = malloc(sizeof(bool) * SIZ_ALPH);
		}
	}

	/* Initialization */
	for (int i=0; i<=n; i++)
		for (int j=0; j<=n; j++)
			for (int k=0; k<SIZ_ALPH; k++)
				P[i][j][k] = false;

	return P;
}


void P_free(P_t P, int n) {
	for (int i = 0; i < n + 1; i++) {
		for (int j = 0; j < n + 1; j++) {
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


void print_matrix(bool ***P, int n) {
	for (int i=1; i<=n; i++)
		for (int j=1; j<=n; j++)
			for (int k=0; k<SIZ_ALPH; k++)
				printf("%d\t%d\t%c\t%s\n",
						i, j, k+65, P[i][j][k] ? "YES" : "");
}



/* SINGLE-STRING PARSING ******************************************** */

void parse(char *input) {

	/* Number of terminals in string */
	int n = strlen(input);

	/* Productions matrix */
	bool ***P = P_new(n);

	/* Handle simple productions */
	for (int i=1; i<=n; i++)
		for (int j=1; j<=r_simp; j++)
			/* R_j -> a_i */
			if (P_simp[j][1] == input[i-1]) {
				int rule = ctoi(P_simp[j][0]);
				P[i][1][rule] = true;
			}

	/* Handle compound productions */
	for (int i=2; i<=n; i++)
		for (int j=1; j<=n-i+1; j++)
			for (int k=1; k<=i-1; k++)
				for (int l=1; l<=r_comp; l++) {
					/* R_A -> R_B R_C */
					int A = ctoi(P_comp[l][0]);
					int B = ctoi(P_comp[l][1]);
					int C = ctoi(P_comp[l][2]);
					if (P[j][k][B] && P[j+k][i-k][C])
						P[j][i][A] = true;
				}

	//print_matrix(P, n);

	if (P[1][n][ctoi(init)])
		puts("YES");
	else
		puts("NO");

	P_free(P, n);
}


/* BUILD PARSER ***************************************************** */

void build() {

	int    lin_siz;
	int    buf_siz    = SIZ_BUF;
	char  *line       = (char *) malloc(buf_siz);
	bool   first_line = true;
	stack *lines_simp = stk_new();
	stack *lines_comp = stk_new();

	/* Read every production rule and keep it in a stack */

	while ((lin_siz = getline(&line, &buf_siz, stdin)) != EOF) {
		bool stop = false;

		if (first_line) {
			init = line[0];
			printf("FIRST IS %c\n", init);
			first_line = false;
		}

		switch (lin_siz) {
			/* Compound rule */
			case 4:
				printf("comp");
				stk_push(lines_comp, line);
				r_comp++;
				break;

			/* Simple rule */
			case 3:
				printf("simp");
				stk_push(lines_simp, line);
				r_simp++;
				break;

			/* Break character */
			case 2: stop = true; break;
		}

		if (stop) break;
		printf("\t%s", line);
	}

	free(line);

	/* Allocate memory for rules arrays */

	P_simp = malloc(sizeof(void *) * (r_simp + 1));
	for (int i = 0; i < r_simp + 1; i++)
		P_simp[i] = malloc(sizeof(char) * SIZ_SIMP);

	P_comp = malloc(sizeof(void *) * (r_comp + 1));
	for (int i = 0; i < r_comp + 1; i++)
		P_comp[i] = malloc(sizeof(char) * SIZ_COMP);

	/* Populate rules arrays from stacks */

	for (int i = 1; i <= r_simp; i++) {
		char line[SIZ_BUF];
		stk_pop(lines_simp, line);
		strncpy(P_simp[i], line, SIZ_SIMP);
	}

	for (int i = 1; i <= r_comp; i++) {
		char line[SIZ_BUF];
		stk_pop(lines_comp, line);
		strncpy(P_comp[i], line, SIZ_COMP);
	}
}

	
/* ****************************************************************** */

int main() {

	build();

	/* INIT FOR STRING PARSING */

	{
		int   buf_siz = SIZ_BUF;
		char *line    = (char *) malloc(buf_siz);

		while (getline(&line, &buf_siz, stdin) != EOF) {
			if (strcmp(line, "#\n") == 0) break;

			int  size = strlen(line) - 1;
			char input[size];
			strncpy(input, line, size);
			input[size] = '\0';

			puts(input);
			parse(input);

		}
		free(line);
	}

	return 0;


/* ****************************************************************** */

}

/* vim: set ft=c: */
