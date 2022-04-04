#include "tree.h"
#include "dot.h"

struct tree *allocate_tree() {

	struct tree *tree = malloc(sizeof (struct tree));
	memset(tree, 0, sizeof(struct tree));

	return tree;

}

struct tree *create_leaf(int category_value, char* yytext, int lineno, char* filename){

	struct tree *tree = allocate_tree();
	struct token *leaf_token = allocate_token();

	tree->prodrule = TOKEN;
	tree->leaf = leaf_token;
	tree->dotId = dot_serialId++;

	leaf_token->category = category_value;
	leaf_token->text = strdup(yytext);
	leaf_token->lineno = yylineno;
	leaf_token->filename = filename;
	leaf_token->ival = 0;
	leaf_token->dval = 0;
	leaf_token->sval = NULL;

	return tree;

}

struct tree *create_branch(prodrule prodrule, char *symbolname, int nkids, ...) {

	struct tree *branch = allocate_tree();

	va_list kids;
	va_start(kids, nkids);

	branch->prodrule = prodrule;
	branch->nkids = nkids;
	branch->dotId = dot_serialId++;
	branch->symbolname = symbolname;

	for(int i = 0; i < nkids; i++) {
		branch->kids[i] = va_arg(kids, struct tree *);
	}

	va_end(kids);

	return branch;

}

char* humanreadable(prodrule rule) {

	switch (rule) {

		case TOKEN : return "Token";
		case prodR_ClassDecl : return "ClassDecl";
		case prodR_ClassBodyDecls : return "ClassBodyDecls";
		case prodR_FieldDecl : return "FieldDecl";
		case prodR_QualifiedName : return "QualifiedName";
		case prodR_VarDecls : return "VarDecls";
		case prodR_MethodDecl : return "MethodDecl";
		case prodR_MethodHeader : return "MethodHeader";
		case prodR_MethodDeclarator : return "MethodDeclarator";
		case prodR_FormalParmList : return "FormalParmList";
		case prodR_FormalParm : return "FormalParm";
		case prodR_ConstructorDecl : return "ConstructorDecl";
		case prodR_ConstructorDeclarator : return "ConstructorDeclarator";
		case prodR_BlockStmts : return "BlockStmts";
		case prodR_LocalVarDecl : return "LocalVarDecl";
		case prodR_IfThenStmt : return "IfThenStmt";
		case prodR_IfThenElseStmt : return "IfThenElseStmt";
		case prodR_IfThenElseIfStmt : return "IfThenElseIfStmt";
		case prodR_ElseIfSequence : return "ElseIfSequence";
		case prodR_WhileStmt : return "WhileStmt";
		case prodR_ForStmt : return "ForStmt";
		case prodR_StmtExprList : return "StmtExprList";
		case prodR_InstantiationExpr : return "InstantiationExpr";
		case prodR_ArgList : return "ArgList";
		case prodR_FieldAccess : return "FieldAccess";
		case prodR_MethodCall : return "MethodCall";
		case prodR_UnaryExpr : return "UnaryExpr";
		case prodR_MulExpr : return "MulExpr";
		case prodR_AddExpr : return "AddExpr";
		case prodR_RelExpr : return "RelExpr";
		case prodR_EqExpr : return "EqExpr";
		case prodR_CondAndExpr : return "CondAndExpr";
		case prodR_CondOrExpr : return "CondOrExpr";
		case prodR_Assignment : return "Assignment";
		default: return "";

	}
}

int print_tree(struct tree* tree, int depth) {


	if (tree == NULL) {
		return 0;
	}

	if (tree->nkids == 0) {

		printf("%*s %s %d: %s\n", depth*4, " ", humanreadable(tree->prodrule),
		tree->leaf->category, tree->leaf->text);

		return 0;

	}

	printf("%*s %s: %d\n", depth*4, " ", humanreadable(tree->prodrule),
	 tree->nkids);

	for(int i = 0; i < tree->nkids; i++) {
		print_tree(tree->kids[i], depth+1);
	}

	return 0;

}

// int free_tree(struct tree* tree, int depth) {
//
// 	if (tree == NULL) {
// 		return 0;
// 	}
//
// 	if (tree->nkids == 0) {
// 		free(tree->leaf->text);
// 		free(tree);
//
// 		return 0;
//
// 	}
//
// 	for(int i = 0; i < tree->nkids; i++) {
// 		if (tree->kids[i] != NULL) {
// 			free(tree->leaf->text);
// 			free_tree(tree->, depth+1);
// 		}
// 	}
//
// 	free(tree->leaf->text);
// 	//free(tree);
//
// 	return 0;
//
// }
