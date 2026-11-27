%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *msg);
%}

%token NUMBER
%left '+' '-'
%left '*' '/'

%%

input:
    expression '\n' {
        printf("Result: %d\n", $1);
    }
    ;

expression:
      NUMBER
    | expression '+' expression { $$ = $1 + $3; }
    | expression '-' expression { $$ = $1 - $3; }
    | expression '*' expression { $$ = $1 * $3; }
    | expression '/' expression {
        if ($3 == 0) {
            yyerror("division by zero");
            YYABORT;
        }
        $$ = $1 / $3;
    }
    ;

%%

int yylex(void)
{
    int c;

    while ((c = getchar()) == ' ')
        ;

    if (c >= '0' && c <= '9') {
        yylval = c - '0';
        return NUMBER;
    }

    return c;
}

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

int main(void)
{
    return yyparse();
}
