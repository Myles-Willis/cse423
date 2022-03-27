#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "type.h"
// #include "prodrules.h"
// #include "tree.h"
// #include "defs.h"

typedef struct sym_table {
	char* table_name;
  	int nBuckets;			/* # of buckets */
  	int nEntries;			/* # of symbols in the table */
	struct sym_table *parent;		/* enclosing scope, superclass etc. */
	struct typeinfo *scope;			/* what type do we belong to? class/method? */
  	struct sym_entry **tbl;
  	/* more per-scope/per-symbol-table attributes go here */
} *SymbolTable;

typedef struct sym_entry {
   SymbolTable table;	//what symbol table do we belong to
   char *s;				/* string */
   // more symbol attributes go here for code generation
   struct typeinfo *type;
   struct sym_entry *next;
   } *SymbolTableEntry;


SymbolTable make_sym_table(int size, char *table);

void populate_symbol_tables(struct tree * n);
void dovariabledeclarator(struct tree * n);

extern SymbolTable globals;	       /* global symbols */
extern SymbolTable current;	       /* current */

void print_symbol(char *s, char *symbolname); //ONLY USED IN LAB #6
void printsyms(struct tree *); //ONLY USED IN LAB #6
void printsymbols(SymbolTable st, int level);

void enter_newscope(char *s, int typ);
int insert_symbol(SymbolTable st, char *s, typeptr t);

#define pushscope(stp) do { stp->parent = current; current = stp; } while (0)
#define popscope() do { current = current->parent; } while(0)

#endif