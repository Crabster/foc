grammar Foc;

// Parser rules ***********************************************************************************

program: decls EOF;

decls: decls funDecl
     | decls COMMENT
     | /* epsilon */;

comment: COMMENT;

funDecl: type ID UNIT_TYPE OpenCurly funBody CloseCurly
       | type ID OpenPar funArgs ClosePar OpenCurly funBody CloseCurly;

funArgs: funArgs Comma type ID
       | type ID;

funBody: funBody varDecl
       | funBody assignment
       | funBody flow
       | funBody comment
       | funBody expr Semicolon
       | funBody PRINT OpenPar expr ClosePar Semicolon
       | /* epsilon */;

varDecl: type ID Semicolon
       | type ID Equal expr Semicolon
       | AUTO ID Equal expr Semicolon
       | type OpenSquare listIDs CloseSquare Semicolon
       | type OpenSquare listIDs CloseSquare Equal expr Semicolon
       | AUTO OpenSquare listIDs CloseSquare Equal expr Semicolon
       | type OpenSharp listIDs CloseSharp Semicolon
       | type OpenSharp listIDs CloseSharp Equal expr Semicolon
       | AUTO OpenSharp listIDs CloseSharp Equal expr Semicolon;

listIDs: listIDs Comma ID
       | ID;

assignment: expr Equal expr Semicolon;

flow: cond
    | loop
    | CONTINUE Semicolon
    | BREAK Semicolon
    | RETURN expr Semicolon;

loop: WHILE OpenPar expr ClosePar OpenCurly funBody CloseCurly;

cond: ifCond elifConds elseCond;

ifCond: IF OpenPar expr ClosePar OpenCurly funBody CloseCurly;

elifConds: elifConds ELIF OpenPar expr ClosePar OpenCurly funBody CloseCurly
         | /* epsilon */;

elseCond: ELSE OpenCurly funBody CloseCurly
        | /* epsilon */;

// Expression stuff *******************************************************************************

expr: expr OpenPar listExprs ClosePar
    | expr operator_ expr
    | OpenPar expr ClosePar
    | expr UNIT_TYPE
    | expr OpenSquare expr CloseSquare
    | expr OpenSharp expr CloseSharp
    | typeExpr
    | Minus expr
    | ID;

typeExpr: INT
        | CHAR
        | STRING
        | bool_
        | ptrExpr
        | tupleExpr
        | arrayExpr;

ptrExpr: Ampersand expr
       | Ampersand Dollar
       | Star expr;

tupleExpr: OpenSharp listExprs CloseSharp;

arrayExpr: OpenSquare listExprs CloseSquare;

listExprs: listExprs Comma expr
         | expr;

operator_: Plus
         | Minus
         | Star
         | Slash
         | IsEqual
         | NotEqual
         | And
         | Or
         | less
         | greater
         | Leq
         | Geq;

less:     OpenSharp;
greater: CloseSharp;

bool_: TRUE | FALSE;

// Type stuff *************************************************************************************

type: UNIT_TYPE                                                  /* () */
    | INT_TYPE                                                   /* int */
    | CHAR_TYPE                                                  /* char */
    | BOOL_TYPE                                                  /* bool */
    | Star type                                                  /* pointer */
    | OpenSharp CloseSharp                                       /* 0-tuple */
    | OpenSharp typeList CloseSharp                              /* tuple */
    | OpenSquare type Comma INT CloseSquare                      /* array */
    | OpenPar OpenSharp CloseSharp Arrow type ClosePar           /* function type with 0 args */
    | OpenPar OpenSharp typeList CloseSharp Arrow type ClosePar; /* function type */

typeList: typeList Comma type
        | type;

// Lexer rules ************************************************************************************

COMMENT: Slash Star (UnescapedChar | ' ')* Star Slash;

PRINT: 'print';
WHILE: 'while';

IF  : 'if';
ELIF: 'elif';
ELSE: 'else';

RETURN: 'return';
CONTINUE: 'continue';
BREAK: 'break';

TRUE: 'T';
FALSE: 'F';

ID: LETTER (LETTER | '0'..'9' | '_')*;
fragment LETTER : [a-zA-Z];

UNIT_TYPE: '()';
INT_TYPE:  '#';
CHAR_TYPE: '@';
BOOL_TYPE: '~';
AUTO:      '_';

INT: DIGIT+;
fragment DIGIT: [0-9];

IsEqual:       '==';
NotEqual:      '!=';
And:           '&&';
Or:            '||';
Leq:           '<=';
Geq:           '>=';

OpenCurly:     '{';
CloseCurly:    '}';
OpenPar:       '(';
ClosePar:      ')';
Equal:         '=';
Colon:         ':';
Semicolon:     ';';
Plus:          '+';
Minus:         '-';
Star:          '*';
OpenSquare:    '[';
CloseSquare:   ']';
OpenSharp:     '<';
CloseSharp:    '>';
Arrow:        '->';
Comma:         ',';
Ampersand:     '&';
Dollar:        '$';
Slash:         '/';

CHAR: '\'' UnescapedChar '\'';
STRING: '"' UnescapedChar* '"';
fragment UnescapedChar: [ \u0032-\u0126];

WS: [ \t\r\n]+ -> channel(HIDDEN);
