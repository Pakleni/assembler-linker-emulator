%{
    /* C */
    #include "../out/parser.h"
    void error();
%}

%option outfile="out/lexer.c" header-file="out/lexer.h"

BLANK               [ \r\n\t]*
COMMENT             "#"[^\n\r]*
HEX                 0[Xx][0-9A-Fa-f]+
DEC                 [0-9]+
IDENT               [A-Za-z_][A-Za-z0-9_]*
LABEL               {IDENT}":"
DIRECTIVE           "."[a-z]*

/* rules */
%%
{COMMENT}           {  }
{HEX}               {
                        sscanf(yytext, "%x", &yylval.num);
                        return TOKEN_NUM;
                    }
{DEC}               {
                        sscanf(yytext, "%d", &yylval.num);
                        return TOKEN_NUM;
                    }
{LABEL}             {
                        // yylval.label = copy_str(yytext);
                        return TOKEN_LABEL;
                    }
{DIRECTIVE}         {
                        // yylval.directive = copy_str(yytext);
                        return TOKEN_DIRECTIVE;
                    }
{BLANK}             {  }
.                   {
                        return yyerror("syntax error");;
                    }
%%

yywrap() {}

int yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}