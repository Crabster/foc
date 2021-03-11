grammar Foc;

// Parser rules

program: decls EOF;

decls: varDecl decls
     | funDecl decls
     | /* epsilon */;

varDecl: type ID Semicolon 
       | type ID Equal expr Semicolon;

funDecl: type ID OpenPar funArgs ClosePar OpenCurly funBody CloseCurly;

funArgs: type ID Comma funArgs 
       | type ID
       | /* epsilon */;

funBody: varDecl funBody 
       | flow funBody
       | expr Semicolon funBody
       | /* epsilon */; 

flow: cond 
    | loop
    | CONTINUE Semicolon
    | RETURN expr Semicolon;

loop: WHILE OpenPar expr ClosePar OpenCurly funBody CloseCurly;

cond: ifCond 
    | ifCond elifConds 
    | ifCond elifConds elseCond;
             
ifCond: IF OpenPar expr ClosePar OpenCurly funBody CloseCurly;

elifConds: ELIF OpenPar expr ClosePar OpenCurly funBody CloseCurly 
         | /* epsilon */;

elseCond: ELSE OpenCurly funBody CloseCurly;

expr: INT
    | STRING
    | bool_;

bool_: TRUE | FALSE;

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

WS: [ \t\r\n]+ -> channel(HIDDEN);

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
