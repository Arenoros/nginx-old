%{

#define YYSTYPE ngx_let_node_t*

#include "let.h"
#include "let.tab.h"

#include <ngx_http.h>

static ngx_conf_t *conf;
static unsigned inpos;
static ngx_let_node_t *result;

int yylex();

void yyerror(char *s) {
	
	ngx_str_t* token = conf->args->elts;

	if (inpos < conf->args->nelts)
		token += inpos;
	else
		token = 0;
	
	ngx_log_debug(NGX_LOG_INFO, conf->log, 0, 
			"error parsing let expression: %s[%d]: %*s", s, inpos,
			token ? token->len : 0, token ? (char*)token->data : "");
	
	yylval = NULL;
}

ngx_let_node_t* ngx_let_binop_node_create(
	ngx_let_node_t* left,
	char op,
	ngx_let_node_t* right) 
{
	ngx_let_node_t** args;
	ngx_let_node_t* node;
	
	node = ngx_pcalloc(conf->pool, sizeof(ngx_let_node_t));

	node->type = NGX_LTYPE_OPERATION;

	node->index = op;

	ngx_array_init(&node->args, conf->pool, 2, sizeof(ngx_let_node_t*));

	args = ngx_array_push_n(&node->args, 2);
	args[0] = left;
	args[1] = right;

/*	yylval = node;*/
	
	ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let operation reduce '%c'", op);

	return node;
}

ngx_let_node_t* ngx_let_fun_arg(
	ngx_let_node_t* fun,
	ngx_let_node_t* arg) 
{
	ngx_let_node_t** args;

	if (!fun->args.nalloc)
		ngx_array_init(&fun->args, conf->pool, 1, sizeof(ngx_let_node_t*));

	args = ngx_array_push(&fun->args);
	*args = arg;

	ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let function argument: %d : %d", fun->type, arg->type);

/*	yylval = fun;*/

	return fun;
}

%}


%%

%token NGXLEAF NGXFUNC NGXFUNC0 NGXDONE;

%left '+' '-' ;

%left '*' '/' '%' '.' ;

%left '&' '|' ;

letexpr : expr NGXDONE { result = $1; YYACCEPT; }

expr: '(' expr ')' { $$ = $2; }

| NGXLEAF

| NGXFUNC0

| funopen ')'

| expr '*' expr    { $$ = ngx_let_binop_node_create($1, '*', $3); }
 
| expr '/' expr    { $$ = ngx_let_binop_node_create($1, '/', $3); }

| expr '%' expr    { $$ = ngx_let_binop_node_create($1, '%', $3); }

| expr '+' expr    { $$ = ngx_let_binop_node_create($1, '+', $3); }

| expr '-' expr    { $$ = ngx_let_binop_node_create($1, '-', $3); }

| expr '&' expr    { $$ = ngx_let_binop_node_create($1, '&', $3); }

| expr '|' expr    { $$ = ngx_let_binop_node_create($1, '|', $3); }

| expr '.' expr    { $$ = ngx_let_binop_node_create($1, '.', $3); }

;

funopen: NGXFUNC

| funopen expr     { $$ = ngx_let_fun_arg($1, $2); }

;

%%

int yylex() {

	ngx_str_t* str;
	ngx_let_node_t* node;

	++inpos;
	
	if (conf == NULL
		|| conf->args == NULL
		|| conf->args->nelts <= inpos)
	{
		return NGXDONE;
	}

	str = ((ngx_str_t*)conf->args->elts + inpos);

	if (str->len == 1 && strchr("+-*/%&|.()", str->data[0])) {

		/* terminal */

		yylval = 0;
		
		ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let terminal '%c'", str->data[0]);

		return str->data[0];
	}
	
	node = ngx_pcalloc(conf->pool, sizeof(ngx_let_node_t));
	yylval = node;

	if (str->len > 1 && str->data[0] == '$') {
		
		/* variable */

		ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let variable %*s", str->len, str->data);
		
		str->data++;
		str->len--;
		
		if (str->data[0] >= '1' && str->data[0] <= '9') {

			node->type = NGX_LTYPE_CAPTURE;
			node->index = str->data[0] - '0';

		} else {

			node->type = NGX_LTYPE_VARIABLE;
			node->index = ngx_http_get_variable_index(conf, str);

		}

		return NGXLEAF;

	} else if (str->len > 2 && str->data[str->len - 2] == '(' 
			&& str->data[str->len - 1] == ')') {

		/* function w/o arguments*/

		ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let function0 %*s", str->len - 1, str->data);

		node->type = NGX_LTYPE_FUNCTION;
		node->name = *str;
		node->name.len -= 2;

		return NGXFUNC0;

	} else if (str->len > 1 && str->data[str->len - 1] == '(' ) {

		/* function */

		ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let function %*s", str->len - 1, str->data);

		node->type = NGX_LTYPE_FUNCTION;
		node->name = *str;
		node->name.len--;

		return NGXFUNC;

	} else {

		/* literal */
		
		ngx_log_debug(NGX_LOG_INFO, conf->log, 0, "let literal %*s", str->len, str->data);
		
		node->type = NGX_LTYPE_LITERAL;
		node->name = *str;

		return NGXLEAF;
	}
}

ngx_let_node_t* ngx_parse_let_expr(ngx_conf_t* cf) {
	conf = cf;
	inpos = 1; /* #1 is dest variable */
	result = 0;
	yyparse();
	return result;
}

