#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "tac.h"
#include "symboltable.h"

struct icn_string {
	struct tree *node;
	struct addr *address;
	int str_bytes;
	int total_bytes;

	struct icn_string *next;
};

struct instr *gen_arglist(struct tree *arglist);
struct addr *newtemp(int num_bytes);
void gen_intermediate_code (struct tree *n);
void gentoken(struct tree *n);
void gen_qualified_addr(struct tree *name_head);
void genfirst(struct tree *t);
void genfollow(struct tree *t);
void gentargets(struct tree *t);
int set_identifier_addr(struct tree *n);
int print_intermediate_tree(struct tree* tree, int depth);
void print_icn_strings(struct icn_string *head, FILE *icn_out);




#endif
