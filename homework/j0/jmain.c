#include "defs.h"
#include "tree.h"
#include "j0gram.tab.h"

extern int yydebug;
char *filename;
extern struct tree *root;

void free_tokenlist(struct tokenlist* head);
int check_file_extension(char *file);
int yyerror(char *s);


int main(int argc, char *argv[]) {

  if (argc == 1) {
    printf("Must provide file name as command line argument\n");
    return 0;
  } else {
		while (--argc > 0) {
			if ((yyin = fopen(*++argv, "r")) == NULL) {
				printf("\nCan not open '%s': File does not exist\n\n", *argv);
			} else if(check_file_extension(*argv) != 1) {
		  		printf("\nCan not open '%s': File does not have .java extension\n\n", *argv);
			} else {

	    		filename = *argv;
				printf("\nBeginning Parse of: %s\n", filename);
				// yydebug = 1;
				yyparse();
				printf("\n");
				print_tree(root, 0);
				printf("\n");
			}
		}
	}
}

int yyerror(char *s) {
   fprintf(stderr, "\n%s:%d: error: %s with token: %s \n\n", filename,
	  yylineno, s, yytext);
	 exit(1);
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
