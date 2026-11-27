AT_SETUP([Basic calculator])

AT_DATA([calculator.y],
[[
%{
#include <stdio.h>
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
    | expression '*' expression { $$ = $1 * $3; }
    ;

%%

int yylex(void)
{
    int c = getchar();

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
]])

AT_BISON_CHECK([calculator.y])

AT_COMPILE([calculator], [calculator.tab.c])

AT_CHECK([echo '2+3' | ./calculator],
         [0],
         [Result: 5
])

AT_CLEANUP
