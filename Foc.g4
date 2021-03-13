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

expr: typeExpr
    | OpenPar expr ClosePar
    | ID
    | expr Operator expr
    | Minus expr;

typeExpr: INT
        | CHAR
        | bool_
        | pointerExpr
        | optExpr
        | tupleExpr
        | listExpr
        | getIth
        | funCall;

pointerExpr: Ampersand ID
        | Ampersand Dollar
        | Star expr;

optExpr: QuestionMark expr
        | QuestionMark Dollar
        | ExclMark expr;

tupleExpr: OpenSharp CloseSharp
        | OpenSharp exprList CloseSharp;

listExpr: OpenCurly CloseCurly
        | OpenCurly exprList CloseCurly;

exprList: expr Comma exprList
        | expr;

getIth: expr OpenSquare expr CloseSquare;

funCall: expr OpenPar ClosePar
    | expr OpenPar untypedArgs ClosePar;

untypedArgs: ID Comma untypedArgs
    | ID;

bool_: TRUE | FALSE;

// Type stuff

type: 'int'
    | 'char'
    | 'bool'
    | Ampersand type                    /* pointer */
    | QuestionMark type                 /* optional */
    | OpenSharp CloseSharp              /* 0-tuple */
    | OpenSharp typelist CloseSharp     /* tuple */
    | OpenCurly type CloseCurly         /* array */
    | OpenPar type Arrow type ClosePar; /* function type */

typelist: type Comma typelist
    | type;


Operator: '+'
    | '-' | '*'
    | '/' | '=='
    | '!='| '&&'
    | '||'| '<'
    | '>' | '<='
    | '>=';

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

CHAR: '\'' [\u0032-\u0126] '\'';

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z];

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
