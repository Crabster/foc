grammar Foc;

// Parser rules

program: decls EOF;

decls: var_decl decls
     | var_decl
     | fun_decl decls
     | /* epsilon */;

var_decl: type ID Semicolon 
        | type ID Equal expr Semicolon;

fun_decl: type ID OpenPar fun_args ClosePar OpenCurly fun_body CloseCurly;

fun_args: type ID fun_args 
        | /* epsilon */;

fun_body: var_decl fun_body 
        | flow fun_body
        | expr Semicolon fun_body
        | /* epsilon */; 

flow: cond 
    | loop
    | CONTINUE Semicolon
    | RETURN expr Semicolon;

loop: WHILE OpenPar expr ClosePar OpenCurly fun_body CloseCurly;

cond: if_cond 
    | if_cond elif_conds 
    | if_cond elif_conds else_cond;
             
if_cond: IF OpenPar expr ClosePar OpenCurly fun_body CloseCurly;

elif_conds: ELIF OpenPar expr ClosePar OpenCurly fun_body CloseCurly 
          | /* epsilon */;

else_cond: ELSE OpenCurly fun_body CloseCurly;

expr: INT
    | STRING
    | bool;

bool: TRUE | FALSE;

type: 'int';

// Lexer rules

WHILE: 'while';

IF: 'if';
ELIF: 'elif';
ELSE: 'else';

RETURN: 'return';
CONTINUE: 'continue';

TRUE: 'true';
FALSE: 'false';

INT: DIGIT+;
fragment DIGIT: [0-9];

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z];

STRING: '"' .*? '"';

Colon:         ':';
Semicolon:     ';';
Equal:         '=';
Plus:          '+';
Minus:         '-';
Star:          '*';
OpenPar:       '(';
ClosePar:      ')';
OpenCurly:     '{';
CloseCurly:    '}';
Comma:         ',';
Ampersand:     '&';
