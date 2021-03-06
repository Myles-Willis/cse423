#include "defs.h"
#include "tree.h"
#include "error.h"
#include "symboltable.h"
#include "intermediate.h"

extern int yydebug;
char *filename;
extern struct tree *root;
extern struct icn_string *icn_strings;

//Flag set boolean values
int symtab_print_flag = 0;
int tree_print_flag = 0;

int check_file_extension(char *file);
void set_flag (char* flag);

int main(int argc, char *argv[]) {

	if (argc == 1) {
		printf("Must provide file name as command line argument\n");
    	return 0;
  	} else {

	 	int flag_count = 0;
		int nonflag = 0;

		//set flags before running input files
		for (int i = 1; i < argc; i++) {

			if (nonflag && (argv[i][0] == '-')) {
				printf("\nusage: ./j0 [flags] [input files]\n");
				throw_error("flags not placed before input files");
			}

			if (argv[i][0] == '-') {
				flag_count++;
				set_flag(argv[i]);
			} else {
				nonflag = 1;
			}
		}

		argv += flag_count;
		argc -= flag_count;

		while (--argc > 0) {

			if ((yyin = fopen(*++argv, "r")) == NULL) {
				printf("\nCan not open '%s': File does not exist\n\n", *argv);
			} else if(check_file_extension(*argv) != 1) {
		  		printf("\nCan not open '%s': File does not have .java extension\n\n", *argv);
			} else {

	    		filename = *argv;
				char* simplified_name = strrchr(filename, '/') + 1;
				char* icn_file_name = strdup(simplified_name);
				icn_file_name[strlen(icn_file_name)-4] = 0;
				strcat(icn_file_name, "icn");

				printf("\n\n---------------------------------------\n");
				printf("Opened File: %s\n", simplified_name);
				printf("---------------------------------------\n");
				// yydebug = 1;
				yyparse();

				if (tree_print_flag) {
					printf("\n");
					print_tree(root, 0);
				}

				globals = make_sym_table(20, "global");
				current = globals;
				load_builtins();
				populate_symbol_tables(root);

				if (symtab_print_flag) {
					printf("\n\nprintsymbols() output:\n");
					printf("---------------------------------------\n\n");
					printsymbols(globals, 1);
				}
				check_types(root);

				// print_intermediate_tree(root, 0);
				genfirst(root);
				genfollow(root);
				gentargets(root);
				gen_intermediate_code(root);
				// print_intermediate_tree(root, 0);

				// printf("\n\n_____Final Tac Print_____\n\n");
				FILE *icn_out = fopen(icn_file_name, "w");
				print_icn_strings(icn_strings, icn_out);
				tacprint(root->icode, icn_out);
				printf("\n");
				fclose(icn_out);
				exit(0);
				//free_tree(root, 0);
			}
		}
	}
}

int check_file_extension(char *file) {

  if (strlen(file) >= 6) {
    char *dot_javaLocation = (strlen(file) - 5) + file;
    if (strcmp(dot_javaLocation, ".java") != 0) {
      return 0;
    } else {
      return 1;
    }
  }

  return 0;

}

void set_flag (char* flag) {
	if (strcmp(flag, "-symtab") == 0) {
		symtab_print_flag = 1;
	} else if(strcmp(flag, "-tree") == 0) {
		tree_print_flag = 1;
	} else {
		printf("\nAvailable flags include: -symtab, -tree\n");
		throw_error("unknown flag");
	}

}
