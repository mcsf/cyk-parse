#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const char SIZ_SIMP =  2;
const char SIZ_COMP =  3;
const char SIZ_ALPH = 26;
const char SIZ_BUF  =  4;

/**
 * RULES:
 *
 * RAB
 * AAZ
 * A0
 * BBO
 * B1
 * Z0
 * O1
 *
 */


/* A..Z -> 0..26 */
int ctoi(char c) {
	return c - 65;
}

void print_matrix(bool P[6][6][SIZ_ALPH], int n) {
	for (int i=1; i<=n; i++) {
		for (int j=1; j<=n; j++) {
			for (int k=0; k<SIZ_ALPH; k++) {
				printf("%d\t%d\t%c\t%s\n",
						i, j, k+65, P[i][j][k] ? "YES" : "");
			}
		}
	}
}

int main() {

/* ****************************************************************** */

	/* BUILD PARSER */

	{
		int   lin_siz;
		int   buf_siz = SIZ_BUF;
		char *line    = (char *) malloc(buf_siz);

		while ((lin_siz = getline(&line, &buf_siz, stdin)) != EOF) {
			bool stop = false;

			switch (lin_siz) {
				/* Compound rule */
				case 4: printf("comp"); break;

						/* Simple rule */
				case 3: printf("simp"); break;

						/* Break character */
				case 2: stop = true; break;
			}
			if (stop) break;
			printf("\t%s", line);
		}
		free(line);
	}

	/* Number of production rules */
	int r_simp; /* Simple rules:   A -> a  */
	int r_comp; /* Compound rules: A -> BC */

	/* Initial rule */
	char init = 'R';

	/* TODO */
	r_simp = 4; r_comp = 3;

	/* Simples rules array */
	char P_simp[SIZ_SIMP][r_simp+1];

	/* Compound rules array */
	char P_comp[SIZ_COMP][r_comp+1];

	/* TODO */
	P_simp[1][0] = 'A'; P_simp[1][1] = '0';
	P_simp[2][0] = 'B'; P_simp[2][1] = '1';
	P_simp[3][0] = 'Z'; P_simp[3][1] = '0';
	P_simp[4][0] = 'O'; P_simp[4][1] = '1';

	P_comp[1][0] = 'R'; P_comp[1][1] = 'A'; P_comp[1][2] = 'B'; 
	P_comp[2][0] = 'A'; P_comp[2][1] = 'A'; P_comp[2][2] = 'Z'; 
	P_comp[3][0] = 'B'; P_comp[3][1] = 'B'; P_comp[3][2] = 'O'; 
	
/* ****************************************************************** */

	/* INIT FOR STRING PARSING */

	{
		int   buf_siz = SIZ_BUF;
		char *line    = (char *) malloc(buf_siz);

		while (getline(&line, &buf_siz, stdin) != EOF) {
			if (strcmp(line, "#") == 0) break;

			printf(line);

		}
		free(line);
	}

	return 0;

	/* Input string */
	const char input[] = "00111";

	/* Number of terminals in string */
	int n = strlen(input);

	/* Productions matrix */
	bool P[n+1][n+1][SIZ_ALPH];

	/* Initialization */
	for (int i=0; i<=n; i++)
		for (int j=0; j<=n; j++)
			for (int k=0; k<SIZ_ALPH; k++)
				P[i][j][k] = false;

/* ****************************************************************** */

	/* PARSE STRING */

	/* Handle simple productions */
	printf("SIMPLE: %s\n", input);
	for (int i=1; i<=n; i++) {
		printf("\ti = %d\n", i);
		for (int j=1; j<=r_simp; j++) {
			/* R_j -> a_i */
			printf("\t\ti, j = %d, %d", i, j);
			printf(" (%c -> %d)", P_simp[j][1], P_simp[j][1]-48);
			printf(" == %c", input[i-1]);
			if (P_simp[j][1] == input[i-1]) {
				int rule = ctoi(P_simp[j][0]);
				P[i][1][rule] = true;
				printf(" true for %c", rule+65);
			}
			printf("\n");
		}
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


/* ****************************************************************** */

	/* FINISH */

	print_matrix(P, n);

	if (P[1][n][ctoi(init)])
		puts("YES");
	else
		puts("NO");

	return 0;
}

/* vim: set ft=c: */
