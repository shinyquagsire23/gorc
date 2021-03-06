%option reentrant
%option never-interactive
%option bison-bridge
%option bison-locations
%option yylineno
%option noyywrap
%option nodefault
%option case-insensitive
%option prefix="gra_"
%option stack

%{
    #include "instance.h"
    #include "base/diagnostics/helper.h"
    #include "parser.hpp"

    #include <string.h>
    #include <stdlib.h>
    #include <sstream>
    #include <boost/format.hpp>
    
    using namespace gorc::cog;
    #define YY_EXTRA_TYPE grammar::instance*
    
    #define YY_USER_ACTION {                                            \
        yylloc->filename = yyextra->get_filename();                        \
        if(yylloc->first_line != (unsigned int)yylineno) yycolumn = 1;    \
        yylloc->first_line = yylloc->last_line = yylineno;                \
        yylloc->first_column = yycolumn;                                \
        yylloc->last_column = yycolumn + yyleng - 1;                    \
        yycolumn += yyleng;                                                \
    }
    
    #define YY_INPUT(buf, result, max_size) {                            \
        buf[0] = yyextra->get_next();                                    \
        if(buf[0] == '\0') result = YY_NULL;                            \
        else result = 1;                                                \
    }

    int parse_hex_int(char*);

%}

D            [0-9]
L            [_a-zA-Z]
H            [a-fA-F0-9]
E            [Ee][+-]?{D}+

%x CODE_SECTION
%x SYMBOL_FIELD
%x SYMBOL_SECTION
%x SYMBOL_DEF
%x SYMBOL_DEF_ASSIGN
%x SYMBOL_EXT_SECTION
%x LINE_COMMENT
%x STRING

%%

<LINE_COMMENT>\n            { yy_pop_state(yyextra->get_scanner()); yycolumn = 1; }
<LINE_COMMENT>.                /* eat non-terminal input */

<SYMBOL_FIELD>{
    [+-]?0x{H}+                {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->integer = parse_hex_int(yytext);
                                return INTEGER_LITERAL;
                            }

    [+-]?{D}+                {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->integer = atoi(yytext);
                                return INTEGER_LITERAL;
                            }

    [+-]?{D}+{E}            {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->floating = static_cast<float>(atof(yytext));
                                return FLOAT_LITERAL;
                            }

    [+-]?{D}*"."{D}+({E})?    {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->floating = static_cast<float>(atof(yytext));
                                return FLOAT_LITERAL;
                            }

    [+-]?{D}+"."{D}*({E})?    {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->floating = static_cast<float>(atof(yytext));
                                return FLOAT_LITERAL;
                            }

    [^[:space:]^=^,]+        {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->string = strdup(yytext);
                                return STRING_FRAGMENT;
                            }

    [[:space:]]                /* Ignore other whitespace */
}

<SYMBOL_SECTION,SYMBOL_DEF,SYMBOL_DEF_ASSIGN,SYMBOL_EXT_SECTION>{
    "#"|"//"                {
                                yy_pop_state(yyextra->get_scanner());
                                yy_push_state(SYMBOL_SECTION, yyextra->get_scanner());
                                yy_push_state(LINE_COMMENT, yyextra->get_scanner());
                                return ENDLINE;
                            }

    "end"                    { yy_pop_state(yyextra->get_scanner()); return END; }

    "="                        {
                                yy_pop_state(yyextra->get_scanner());
                                yy_push_state(SYMBOL_EXT_SECTION, yyextra->get_scanner());
                                yy_push_state(SYMBOL_FIELD, yyextra->get_scanner());
                                return '=';
                            }

    "\n"                    {
                                yy_pop_state(yyextra->get_scanner());
                                yy_push_state(SYMBOL_SECTION, yyextra->get_scanner());
                                return ENDLINE;
                            }

    [[:space:]]                /* Ignore other whitespace */
}

<SYMBOL_EXT_SECTION>{
    {L}({L}|{D})*            { yylval->string = strdup(yytext); return IDENTIFIER; }
    ","                        /* Ignore commas */
}

<SYMBOL_DEF_ASSIGN>{L}({L}|{D})*    {
                                yy_pop_state(yyextra->get_scanner());
                                yy_push_state(SYMBOL_EXT_SECTION, yyextra->get_scanner());
                                yylval->string = strdup(yytext);
                                return IDENTIFIER;
                            }

<SYMBOL_DEF>{L}({L}|{D})*    {
                                yy_pop_state(yyextra->get_scanner());
                                yy_push_state(SYMBOL_DEF_ASSIGN, yyextra->get_scanner());
                                yylval->string = strdup(yytext);
                                return IDENTIFIER;
                            }

<SYMBOL_SECTION>{L}({L}|{D})*    {
                                    yy_pop_state(yyextra->get_scanner());
                                    yy_push_state(SYMBOL_DEF, yyextra->get_scanner());
                                    yylval->string = strdup(yytext);
                                    return IDENTIFIER;
                                }

<STRING>{
    <<EOF>>                    {
                                gorc::diagnostics::helper::eof_in_string_literal(yyextra->report, *yylloc);
                            }

    \"                        {
                                yy_pop_state(yyextra->get_scanner());
                                yylval->string = strdup(yyextra->token_buffer.c_str());
                                return STRING_LITERAL;
                            }

    \n                        {
                                gorc::diagnostics::helper::unescaped_newline_in_string(yyextra->report, *yylloc);
                            }

    \\\n                    /* Consume escaped newlines */

    \\["\\]                    { yyextra->token_buffer.push_back(yytext[1]); }

    \\n                        { yyextra->token_buffer.push_back('\n'); }

    \\.                        {
                                gorc::diagnostics::helper::unknown_escape_sequence(yyextra->report, yytext, *yylloc);
                            }

    .                        { yyextra->token_buffer.append(yytext); }
}

<CODE_SECTION>{
    "#"                        { yy_push_state(LINE_COMMENT, yyextra->get_scanner()); }
    "//"                    { yy_push_state(LINE_COMMENT, yyextra->get_scanner()); }
    "end"                    { yy_pop_state(yyextra->get_scanner()); return END; }

    \"                        { yy_push_state(STRING, yyextra->get_scanner()); yyextra->token_buffer.clear(); }

    "break"                    { return BREAK; }
    "call"                    { return CALL; }
    "do"                    { return DO; }
    "else"                    { return ELSE; }
    "for"                    { return FOR; }
    "if"                    { return IF; }
    "return"                { return RETURN; }
    "stop"                    { return RETURN; }
    "while"                    { return WHILE; }

    {L}({L}|{D})*            { yylval->string = strdup(yytext); return IDENTIFIER; }

    0x{H}+                    { yylval->integer = parse_hex_int(yytext); return INTEGER_LITERAL; }
    {D}+                    { yylval->integer = atoi(yytext); return INTEGER_LITERAL; }

    {D}+{E}                    { yylval->floating = static_cast<float>(atof(yytext)); return FLOAT_LITERAL; }
    {D}*"."{D}+({E})?        { yylval->floating = static_cast<float>(atof(yytext)); return FLOAT_LITERAL; }
    {D}+"."{D}*({E})?        { yylval->floating = static_cast<float>(atof(yytext)); return FLOAT_LITERAL; }

    "=="                    { return EQ_OP; }
    "!="                    { return NE_OP; }
    ">="                    { return GE_OP; }
    "<="                    { return LE_OP; }

    "&&"                    { return LOGICAL_AND_OP; }
    "||"                    { return LOGICAL_OR_OP; }

    ":"                        { return ':'; }
    ";"                        { return ';'; }
    ","                        { return ','; }

    "\'"                    { return '\''; }

    "+"                        { return '+'; }
    "-"                        { return '-'; }
    "*"                        { return '*'; }
    "/"                        { return '/'; }
    "%"                        { return '%'; }

    "!"                        { return '!'; }

    "&"                        { return '&'; }
    "|"                        { return '|'; }
    "^"                        { return '^'; }

    "="                        { return '='; }

    "{"                        { return '{'; }
    "}"                        { return '}'; }
    "("                        { return '('; }
    ")"                        { return ')'; }
    "<"                        { return '<'; }
    ">"                        { return '>'; }
    "["                        { return '['; }
    "]"                        { return ']'; }

    [[:space:]]                /* Ignore whitespace */
}

<INITIAL>{
    "#"                        { yy_push_state(LINE_COMMENT, yyextra->get_scanner()); }
    
    "flags"                    { return FLAGS; }
    "="                        { return '='; }
    0x{H}+                    { yylval->integer = parse_hex_int(yytext); return INTEGER_LITERAL; }
    
    "symbols"                { yy_push_state(SYMBOL_SECTION, yyextra->get_scanner()); return SYMBOLS; }
    "code"                    { yy_push_state(CODE_SECTION, yyextra->get_scanner()); return CODE; }

    [[:space:]]                /* Ignore whitespace */
}

<INITIAL,SYMBOL_FIELD,SYMBOL_SECTION,SYMBOL_DEF,SYMBOL_DEF_ASSIGN,SYMBOL_EXT_SECTION,CODE_SECTION>.        {
                                gorc::diagnostics::helper::unrecognized_input(yyextra->report,
                                    boost::str(boost::format("unrecognized input \'%s\'") % yytext), *yylloc);
                            }

%%

int parse_hex_int(char* str) {
    return static_cast<int>(strtol(str, nullptr, 0));
}

void gorc::cog::grammar::instance::init_scanner()
{
    yylex_init(&scanner);
    yyset_extra(this, scanner);
}

void gorc::cog::grammar::instance::destroy_scanner()
{
    yylex_destroy(scanner);
}

