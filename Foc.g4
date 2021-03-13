grammar Foc;

// Parser rules

program: decls EOF;

decls: funDecl decls
     | /* epsilon */;

varDecl: type ID Semicolon
       | type ID Equal expr Semicolon
       | 'auto' ID Equal expr Semicolon;

funDecl: type ID OpenPar funArgs ClosePar OpenCurly funBody CloseCurly
        | type ID OpenPar ClosePar OpenCurly funBody CloseCurly;

funArgs: type ID Comma funArgs
       | type ID;

funBody: varDecl funBody
       | ID Equal expr Semicolon
       | flow funBody
       | /* epsilon */;

flow: cond
    | loop
    | CONTINUE Semicolon
    | BREAK Semicolon
    | RETURN expr Semicolon;

loop: WHILE OpenPar expr ClosePar OpenCurly funBody CloseCurly;

cond: ifCond elifConds elseCond;

ifCond: IF OpenPar expr ClosePar OpenCurly funBody CloseCurly;

elifConds: ELIF OpenPar expr ClosePar OpenCurly funBody CloseCurly elifConds
        | /* epsilon */;

elseCond: ELSE OpenCurly funBody CloseCurly
        | /* epsilon */;

// Expression stuff

expr: typeexpr
    | OpenPar expr ClosePar
    | ID
    | expr OPERATOR expr
    | Minus expr;

typeexpr: INT
        | CHAR
        | bool_
        | pointerExpr
        | optExpr
        | tupleExpr
        | listExpr
        | getIth
        | funcall;

pointerExpr: Ampersand ID
        | Ampersand Dollar
        | Star expr;

optExpr: QuestionMark expr
        | QuestionMark Dollar
        | ExclMark expr;

tupleExpr: OpenSharp CloseSharp
        | OpenSharp exprlist CloseSharp;

listExpr: OpenCurly CloseCurly
        | OpenCurly exprlist CloseCurly;

exprlist: expr Comma exprlist
        | expr;

getIth: expr OpenSquare expr CloseSquare;

funcall: expr OpenPar ClosePar
    | expr OpenPar untypedArgs ClosePar;

untypedArgs: ID Semicolon untypedArgs
    | ID;

bool_: TRUE | FALSE;

// Type stuff

type: 'int'
    | 'char'
    | 'bool'
    | Ampersand type
    /* pointer */
    | QuestionMark type
    /* optional */
    | OpenSharp CloseSharp
    /* 0-tuple */
    | OpenSharp typelist CloseSharp
    /* tuple */
    | OpenCurly type CloseCurly
    /* array */
    | OpenPar type Arrow type ClosePar;
    /* function type */

typelist: type Comma typelist
    | type;

// Lexer rules

WHILE: 'while';

IF: 'if';
ELIF: 'elif';
ELSE: 'else';

RETURN: 'return';
CONTINUE: 'continue';
BREAK: 'break';

TRUE: 'true';
FALSE: 'false';

INT: DIGIT+;
fragment DIGIT: [0-9];

CHAR: 'todo';

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z];

STRING: '"' .*? '"';

OPERATOR: ('+'|'-'|'*'|'/'|'=='|'!='|'&&'|'||'|'<'|'>'|'<='|'>=');

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
OpenSquare:    '[';
CloseSquare:   ']';
OpenSharp:     '<';
CloseSharp:    '>';
Arrow:        '->';
Comma:         ',';
Ampersand:     '&';
QuestionMark:  '?';
ExclMark:      '!';
Dollar:        '$';
