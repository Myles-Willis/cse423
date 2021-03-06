#include "symboltable.h"

#define SBufSize 1024               /* initial size of the string buffer */

/*
 * str_buf references a string buffer. Strings are built a
 *  character at a time. When a buffer "fragment" is filled,
 *  another is allocated and the the current string copied to it.
 */
struct str_buf_frag {
   struct str_buf_frag *next;     /* next buffer fragment */
   char s[1];                     /* variable size buffer */
};

struct str_buf {
   unsigned int size;             /* total size of current buffer */
   char *p;                       /* next free character in buffer */
   char *end;                     /* end of current buffer */
   struct str_buf_frag *frag_lst; /* list of buffer fragments */
};

static struct str_buf buf;
SymbolTable stringpool;

SymbolTable globals;
SymbolTable current;


void init_sbuf(struct str_buf *);   /* initialize an sbuf struct */
void clear_sbuf(struct str_buf *);  /* free struct buffer storage */
void new_sbuf(struct str_buf *);    /* allocate add'l buffer */
char *checked_alloc(int size);

/*
* init_sbuf - initialize a new sbuf struct, allocating an initial buffer.
*/
void init_sbuf(struct str_buf *sbuf) {
	sbuf->size = SBufSize;
	sbuf->frag_lst = (struct str_buf_frag *)checked_alloc((unsigned int)
	(sizeof(struct str_buf_frag) + (SBufSize - 1)));
	sbuf->frag_lst->next = NULL;
	sbuf->p = sbuf->frag_lst->s;
	sbuf->end = sbuf->p + SBufSize;
}

/*
* clear_sbuf - free string buffer storage.
*/
void clear_sbuf(struct str_buf *sbuf) {
	struct str_buf_frag *sbf, *sbf1;

	for (sbf = sbuf->frag_lst; sbf != NULL; sbf = sbf1) {
		sbf1 = sbf->next;
		free((char *)sbf);
	}
	sbuf->frag_lst = NULL;
	sbuf->p = NULL;
	sbuf->end = NULL;
}

/*
* new_sbuf - allocate a new buffer for a sbuf struct, copying the
*   partially created string from the end of full buffer to the new one.
*/
void new_sbuf(struct str_buf *sbuf) {
	struct str_buf_frag *sbf;

	/*
	* The new buffer is larger than the old one to insure that any
	*  size string can be buffered.
	*/
	sbuf->size *= 2;
	sbf = (struct str_buf_frag *)checked_alloc((unsigned int)
	(sizeof(struct str_buf_frag) + (sbuf->size - 1)));
	sbf->next = sbuf->frag_lst;
	sbuf->frag_lst = sbf;
	sbuf->p = sbf->s;
	sbuf->end = sbuf->p + sbuf->size;
}


static char *insert_sbuf(struct str_buf *sb, char *s) {
	char *rv;
	int l = strlen(s);
	if (sb->p + l + 1 >= sb->end)
	new_sbuf(sb);
	strcpy(sb->p, s);
	rv = sb->p;
	sb->p += l + 1;
	return rv;
}

SymbolTable make_sym_table(int size, char* table_name) {

	SymbolTable table;

	table = (SymbolTable) checked_alloc(sizeof(struct sym_table));

	table->tbl = (struct sym_entry **)
		checked_alloc((unsigned int) (size * sizeof(struct sym_entry *))); //* or **?
	table->nBuckets = size;
	table->nEntries = 0;
	table->byte_words = 0;
	table->table_name = table_name;

	return table;

}

int hash(SymbolTable st, char *s) {

	register int h = 0;
	register char c;

	while ((c = *s++)) {
		h += c & 0377;
		h *= 37;
	}

	if (h < 0) h = -h;
		return h % st->nBuckets;

}

int insert_symbol(SymbolTable st, char *s, typeptr t) {

   	int h;
   	struct sym_entry *se;
	se = malloc(sizeof(struct sym_entry));
	memset(se, 0, sizeof(struct sym_entry));

   	h = hash(st, s);
   	for (se = st->tbl[h]; se != NULL; se = se->next)
      	if (!strcmp(s, se->s)) {
         	/*
          	* A copy of the string is already in the table.
          	*/
         	return 0;
        }

   	/*
    * The string is not in the table. Add the copy from the
    *  buffer to the table.
    */
   	se = (SymbolTableEntry) checked_alloc((unsigned int) sizeof (struct sym_entry));
   	se->next = st->tbl[h];
   	se->table = st;
   	st->tbl[h] = se;
   	if (st ==stringpool) se->s = insert_sbuf(&buf, s);
   	else se->s = s;
	se->type = t;

	struct addr *temp = malloc(sizeof(struct addr));
	memset(temp, 0, sizeof(struct addr));
	se->address = temp;
	se->address->region = R_LOCAL;
	se->address->u.offset = (8 * st->byte_words);

   	st->nEntries++;
	st->byte_words++;

   	return 1;
}

SymbolTableEntry lookup_st(SymbolTable st, char *s) {

   	int h;
   	SymbolTableEntry se;

   	h = hash(st, s);
   	for (se = st->tbl[h]; se != NULL; se = se->next)
      	if (!strcmp(s, se->s)) {
         	/*
         	*  Return a pointer to the symbol table entry.
         	*/
        	return se;
        }

   	return NULL;
}

void printsymbols(SymbolTable st, int level) {

	// printf("--- symbol table for: %s\n", st->table_name);
	//
	int i,j;
   	SymbolTableEntry ste;
   	if (st == NULL) return;
   	for (i=0;i<st->nBuckets;i++) {
      	for (ste = st->tbl[i]; ste; ste=ste->next) {

			for (j=0; j < level; j++) printf("   ");
			// printf("%s [%s] ", typename(ste->type),ste->s);
			// print_addr(*ste->address);
			// printf("\n");
			// for (j=0; j < level; j++) printf("  ");

			/* if this symbol has a subscope,
			 print it recursively, indented*/
			if (!ste->type) continue;

			switch (ste->type->basetype) {
				case CLASS_TYPE:
				case FUNC_TYPE:
					for (j=0; j < level; j++) printf("   ");
					printf("--- symbol table for %s: %s [%d words] ---\n",typename(ste->type), ste->s, ste->table->byte_words);
					printsymbols(ste->type->type_sym_table, level + 1);
					for (j=0; j < level; j++) printf("   ");
					printf("---\n\n");
					break;
			}
      }
   }
}

void redeclaration_error(struct token *t) {

	fprintf(stderr, "\n%s:%d: semantic error: Redeclaration of variable: %s \n\n"
		, t->filename, t->lineno, t->text);

	exit(3);
}

void undeclared_error(struct token *t) {

	fprintf(stderr, "\n%s:%d: semantic error: %s is undeclared \n\n"
		, t->filename, t->lineno, t->text);

	exit(3);
}

SymbolTableEntry check_if_undeclared(SymbolTable st, char* s) {
	/*Search through current symbol table. If s not found then search in
	parent symbol table.
	continue until symbol is found or there are no more parent tables.*/

	SymbolTableEntry search = lookup_st(st, s);

	if (search != NULL) {
		//symbol found in current table
		// printf("Symbol: %s found in table %s\n", search->s, st->table_name);
		return search;
	} else if (st->parent == NULL) {
		//No more parent tables to search
		// printf("No Symbol tables left to search for %s\n", s);
		return NULL;
	} else if(search == NULL) {
		//symbol not in current symbol table
		search = check_if_undeclared(st->parent, s);
	}

	return search;
}

void populate_symbol_tables(struct tree * n) {

	int i;
	if (n == NULL) return;

	/* pre-order activity */
	switch (n->prodrule) {

		case prodR_ConstructorDecl: {

			if (lookup_st(current, n->kids[0]->kids[0]->leaf->text)) {
				redeclaration_error(n->kids[0]->kids[0]->leaf);
			}
			enter_newscope(n->kids[0]->kids[0]->leaf->text, CONSTRUCT_TYPE, n);
			break;
		}

		case prodR_MethodDecl: {
			// printf("MethodDecl Here\n");
			if (lookup_st(current, n->kids[0]->kids[1]->kids[0]->leaf->text)) {
				redeclaration_error(n->kids[0]->kids[1]->kids[0]->leaf);
			}
			enter_newscope(n->kids[0]->kids[1]->kids[0]->leaf->text, FUNC_TYPE, n);
		 	break;
		}

		case prodR_ClassDecl: {

			if (lookup_st(current, n->kids[2]->leaf->text)) {
				redeclaration_error(n->kids[2]->leaf);
			}
			enter_newscope(n->kids[2]->leaf->text, CLASS_TYPE, n);
			break;
		}

		case prodR_FieldDecl:
		case prodR_LocalVarDecl:
		case prodR_StaticFieldDecl: {
			// printf("ProdRule: %s\n", n->symbolname);
			typeptr t = NULL;
			int insert_result = 0;

			if (n->nkids == 2 && (n->kids[1]->nkids > 0 && n->kids[1]->kids[0]->prodrule != TOKEN)) {

				int postBracket = strcmp(n->kids[1]->kids[0]->symbolname,
					 "PostBracketArrayDeclarator");
				int preBracket = strcmp(n->kids[1]->kids[0]->symbolname,
					 "PreBracketArrayDeclarator");

				if ((postBracket == 0) || (preBracket == 0)) {

					// printf("Post/PreBracketArray Declarator\n\n");
					t = alctype(conv_to_type(n->kids[0]->leaf->text));
					insert_result = insert_symbol(current,
						 n->kids[1]->kids[0]->kids[0]->leaf->text, t);

				    if (insert_result == 0) {
						// printf("Redeclaration Here***\n");
						redeclaration_error(n->kids[1]->kids[0]->leaf);
					}

					n->stab = current;
			 		n->kids[1]->kids[0]->kids[0]->leaf->type = t;
				}
			} else {
				// printf("Other cases***\n");
				t = alctype(conv_to_type(n->kids[0]->leaf->text));
				insert_result = insert_symbol(current,
					n->kids[1]->kids[0]->leaf->text, t);

				if (insert_result == 0) {
					// printf("Redeclaration Here***\n");
					redeclaration_error(n->kids[1]->kids[0]->leaf);
				}

				n->stab = current;
				n->kids[1]->kids[0]->leaf->type = t;
			}

			break;
		}

		case prodR_TypeAssignment:
		case prodR_FieldDeclAssign: {

			typeptr t = alctype(conv_to_type(n->kids[0]->leaf->text));
			// printf("TYPE: %s\n", n->kids[0]->leaf->text);
			int insert_result = insert_symbol(current,
				 n->kids[1]->leaf->text, t);
			// printf("****%s\n", n->kids[0]->leaf->text);
			if (insert_result == 0) {
				redeclaration_error(n->kids[1]->leaf);
			}

			n->stab = current;
			n->kids[1]->leaf->type = t;

			break;
		}

		case prodR_FormalParm: {

			typeptr t = NULL;
			int insert_result = 0;
			// printf("TYPE: %s\n", n->kids[0]->leaf->text);

			if (n->nkids == 2 && n->kids[1]->nkids > 0) {

				int postBracket = strcmp(n->kids[1]->symbolname, "PostBracketArrayDeclarator");
				int preBracket = strcmp(n->kids[1]->symbolname, "PreBracketArrayDeclarator");

				if ((postBracket == 0) || (preBracket == 0)) {
					// printf("BRACK in FORMAL\n");
					t = alctype(conv_to_type(n->kids[0]->leaf->text));

					insert_result = insert_symbol(current,
						n->kids[1]->kids[0]->leaf->text, t);
					// printf("****%s\n", n->kids[0]->leaf->text);
					if (insert_result == 0) {
						redeclaration_error(n->kids[1]->kids[0]->leaf);
					}

					n->stab = current;
					n->kids[1]->kids[0]->leaf->type = t;

					//Add formal parameter to its method's parameter linked list
					paramlist param = malloc(sizeof(struct param));

					param->name = n->kids[1]->kids[0]->leaf->text;
					param->type = t;

					if (current->scope->u.f.has_param) {

						paramlist last_ptr = current->scope->u.f.parameters;

						while (last_ptr->next != NULL) {
							last_ptr = last_ptr->next;
						}
						// printf("** previous param [%s] adding [%s %s] to next\n", last_ptr->name, typename(t), param->name);
						param->position = last_ptr->position + 1;
						last_ptr->next = param;
					} else {
						// printf("** No initial param, setting initial to [%s %s]\n", typename(t), param->name);
						param->position = 0;
						current->scope->u.f.parameters = param;
						param->next = NULL;
						current->scope->u.f.has_param = 1;
					}
				}
			} else {

				t = alctype(conv_to_type(n->kids[0]->leaf->text));

				insert_result = insert_symbol(current,
					n->kids[1]->leaf->text, t);
					// printf("****%s\n", n->kids[0]->leaf->text);
					if (insert_result == 0) {
						redeclaration_error(n->kids[1]->leaf);
					}

					n->stab = current;
					n->kids[1]->leaf->type = t;

					//Add formal parameter to its method's parameter linked list
					paramlist param = malloc(sizeof(struct param));

					param->name = n->kids[1]->leaf->text;
					param->type = t;

					if (current->scope->u.f.has_param) {

						paramlist last_ptr = current->scope->u.f.parameters;

						while (last_ptr->next != NULL) {
							last_ptr = last_ptr->next;
						}
						// printf("** previous param [%s] adding [%s %s] to next\n", last_ptr->name, typename(t), param->name);
						param->position = last_ptr->position + 1;
						last_ptr->next = param;
					} else {
						// printf("** No initial param, setting initial to [%s %s]\n", typename(t), param->name);
						param->position = 0;
						current->scope->u.f.parameters = param;
						param->next = NULL;
						current->scope->u.f.has_param = 1;
					}
			}

			break;
		}

		case prodR_QualifiedName: {

			//Copy current symbol table and tree (n) to traverse
			SymbolTable traversal = current;
			struct tree * tree_copy = n;


			SymbolTableEntry name_search =
				check_if_undeclared(traversal, tree_copy->kids[0]->leaf->text);

			// printf("Searched for: %s\n",tree_copy->kids[0]->leaf->text);

			if (name_search == NULL) {
				undeclared_error(tree_copy->kids[0]->leaf);
			}


			int end = 0;
			char* next_thing;

			while(name_search != NULL) {
				// printf("HERE: %s\n", name_search->s);
				// printf("%s\n", typename(name_search->type));

				if(name_search->type == NULL){ //Need to add types to builtins
					// printf("Is not a class to dive into\n");
					undeclared_error(tree_copy->kids[1]->leaf);
				}

				switch (name_search->type->basetype) {

					case CLASS_TYPE:
					case FUNC_TYPE:
					case CONSTRUCT_TYPE: {
						//enter scope of discovered entry
						traversal = name_search->type->type_sym_table;

						if (tree_copy->kids[1]->prodrule == prodR_QualifiedName) {

							next_thing = tree_copy->kids[1]->kids[0]->leaf->text;
							// printf("*Next thing:%s\n", next_thing);
							name_search = lookup_st(traversal, next_thing);

							if (name_search == NULL) {
								undeclared_error(tree_copy->kids[1]->kids[0]->leaf);
							}

							tree_copy = tree_copy->kids[1];

						} else {
							next_thing = tree_copy->kids[1]->leaf->text;
							// printf("**Next thing:%s\n", next_thing);
							name_search = lookup_st(traversal, next_thing);

							if (name_search == NULL) {
								undeclared_error(tree_copy->kids[1]->leaf);
							}

							end = 1;
						}
						break;
					}
				}
				//Break out of while loop if there are no more qualified names
				if (end) {break;}
			}
			n->stab = current;
			break;
		}

		case TOKEN: {

			int category = n->leaf->category;

			// printf("%s\n", n->symbolname);

			if (category == IDENTIFIER) {
				SymbolTableEntry check = check_if_undeclared(current, n->leaf->text);
				if (check == NULL) {
					// printf("%s\n", n->leaf->text);
					undeclared_error(n->leaf);
				} else {
					n->stab = check->table;
				}
			}
			break;
		}
	}

	if (n->stab == NULL) { n->stab = current; }


	/* visit children */
	switch (n->prodrule) {
		case prodR_QualifiedName:
			break;
		default:
			for (i = 0; i < n->nkids; i++) {
				populate_symbol_tables(n->kids[i]);
			}
			break;
	}

	/* post-order activity */
	switch (n->prodrule) {

		/* we are back out to a node where we entered a subscope */
		case prodR_ConstructorDecl:
		case prodR_MethodDecl:
		case prodR_ClassDecl:
			popscope();
			break;
	}
}

char *checked_alloc(int size) {

	char *p = calloc(size, sizeof(char));

	if (p == NULL) {
		fprintf(stderr, "Out of memory: %d bytes requested\n", size);
		exit(-1); //proper exit value?
	}

	return p;
}

typeptr get_method_return_type(struct tree *n) {

	if (n != NULL) {
		if (n->kids[0]->kids[0]->prodrule == prodR_PostBracketArray) {
			// printf("Found PostBracketArray\n");
			typeptr array_type = alctype(conv_to_type(n->kids[0]->kids[0]->
				kids[0]->leaf->text));
			typeptr return_type = alcarraytype(array_type, 0);
			return return_type;
		} else {
			typeptr return_type = alctype(conv_to_type(n->kids[0]->kids[0]
				->leaf->text));
				return return_type;
		}
	}

	throw_semantic_error("can not determine return type of method\n", n->leaf->lineno);
	return NULL;
}

int count_params(struct tree *parm_list) {

	int param_count = 0;

	if (strcmp(parm_list->kids[0]->symbolname, "FormalParmList") == 0) {
		int inner_count = count_params(parm_list->kids[0]);
		param_count = param_count + inner_count;
	} else if (strcmp(parm_list->kids[0]->symbolname, "FormalParm") == 0) {
		param_count++;
	}

	param_count++;

	return param_count;
}

int get_param_count(struct tree *formal_parm_listOpt) {

	int param_count;

	if (formal_parm_listOpt) {
		//Check if FormalParm or FormalParmList
		if (strcmp(formal_parm_listOpt->symbolname, "FormalParm") == 0) {
			param_count = 1;
		} else {
			param_count = count_params(formal_parm_listOpt);
		}
	} else {
		param_count = 0;
	}

	return param_count;
}

void enter_newscope(char *s, int typ, struct tree * n) {

	/* allocate a new symbol table */
  	SymbolTable new_st = make_sym_table(20, s);
	struct typeinfo *t;

	if (typ == CLASS_TYPE) {
		t = alcclasstype(new_st);
		t->u.c.name = s;
	} else {

		t = alcfunctype(new_st);
		struct tree *formal_parm_listOpt = n->kids[0]->kids[1]->kids[1];

		t->u.f.returntype = get_method_return_type(n);
		t->u.f.name = s;
		t->u.f.nparams = get_param_count(formal_parm_listOpt);

		// printf("** Method [%s] has [%d] parameters\n", s, t->u.f.nparams);
		// printf("** Return type determined as: %s\n", typename(t->u.f.returntype));
	}

	t->type_sym_table = new_st;
	new_st->scope = t;
	// n->type = t;

	/* insert s into current symbol table */
  	insert_symbol(current, s, t);

	// SymbolTableEntry testing = malloc(sizeof(SymbolTableEntry));
	// testing = lookup_st(current, s);

	/* push new symbol on the stack, making it the current symbol table */
  	pushscope(new_st);

}

void load_builtins() {


	enter_newscope("String", CLASS_TYPE, NULL);


	struct typeinfo *charAt, *equals, *length, *substring, *valueOf, *print, *println, *read, *close;

	 charAt = alctype(FUNC_TYPE);
		 charAt->u.f.returntype = alctype(CHAR_TYPE);
		 charAt->u.f.name = "charAt";
		 charAt->u.f.nparams = 1;

	 equals = alctype(FUNC_TYPE);
		 equals->u.f.returntype = alctype(BOOL_TYPE);
		 equals->u.f.name = "equals";
		 equals->u.f.nparams = 2;

	 length = alctype(FUNC_TYPE);
		 length->u.f.returntype = alctype(INT_TYPE);
		 length->u.f.name = "length";
		 length->u.f.nparams = 0;

	 substring = alctype(FUNC_TYPE);
		 substring->u.f.returntype = alctype(STRING_TYPE);
		 substring->u.f.name = "substring";
		 substring->u.f.nparams = 2;

	 valueOf = alctype(FUNC_TYPE);
		 valueOf->u.f.returntype = alctype(INT_TYPE);
		 valueOf->u.f.name = "valueOf";
		 valueOf->u.f.nparams = 1;

	 print = alctype(FUNC_TYPE);
		 print->u.f.returntype = alctype(STRING_TYPE);
		 print->u.f.name = "print";
		 print->u.f.nparams = 1;

	 println = alctype(FUNC_TYPE);
		 println->u.f.returntype = alctype(STRING_TYPE);
		 println->u.f.name = "println";
		 println->u.f.nparams = 1;

	 read = alctype(FUNC_TYPE);
		 read->u.f.returntype = alctype(INT_TYPE);
		 read->u.f.name = "read";
		 read->u.f.nparams = 0;

	 close = alctype(FUNC_TYPE);
		 close->u.f.returntype = NULL;
		 close->u.f.name = "close";
		 close->u.f.nparams = 0;

	insert_symbol(current, "charAt", charAt);
	insert_symbol(current, "equals", equals);
	insert_symbol(current, "length", length);
	insert_symbol(current, "substring", substring);
	insert_symbol(current, "valueOf", valueOf);
	popscope();

	enter_newscope("System", CLASS_TYPE, NULL);

		enter_newscope("out", CLASS_TYPE, NULL);
		insert_symbol(current, "print", print);
		insert_symbol(current, "println", println);
		popscope();

		enter_newscope("in", CLASS_TYPE, NULL);
		insert_symbol(current, "read", read);
		insert_symbol(current, "close", close);
		popscope();
		popscope();


}
