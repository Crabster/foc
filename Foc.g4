grammar Foc;

// Parser rules

program: decls EOF;

decls: funDecl decls
     | comment decls
     | /* epsilon */;

comment: Slash Star CHAR* Star Slash;

varDecl: type ID Semicolon
       | type ID Equal expr Semicolon
       | AUTO ID Equal expr Semicolon
       | type OpenSquare listIDs CloseSquare Semicolon
       | type OpenSquare listIDs CloseSquare Equal expr Semicolon
       | AUTO OpenSquare listIDs CloseSquare Equal expr Semicolon
       | type OpenSharp listIDs CloseSharp Semicolon
       | type OpenSharp listIDs CloseSharp Equal expr Semicolon
       | AUTO OpenSharp listIDs CloseSharp Equal expr Semicolon;

funDecl: type ID OpenPar funArgs ClosePar OpenCurly funBody CloseCurly;

funArgs: funArg
       | /* epsilon */;

funArg: type ID Comma funArg
      | type ID;

funBody: varDecl funBody
       | ID Equal expr Semicolon
       | flow funBody
       | comment
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

expr: expr_ operator_ expr
    | expr_ expr exprApply
    | expr_ exprApply
    | expr_;

expr_: typeExpr
     | OpenPar expr ClosePar
     | ID
     | Minus expr;

exprApply: getIth
         | funCall;

typeExpr: INT
        | CHAR
        | STRING
        | bool_
        | pointerExpr
        | optExpr
        | tupleExpr
        | arrayExpr;

pointerExpr: Ampersand ID
           | Ampersand Dollar
           | Star expr;

optExpr: QuestionMark expr
       | QuestionMark Dollar
       | ExclMark expr;

tupleExpr: OpenSharp listExprs CloseSharp;

arrayExpr: OpenSquare listExprs CloseSquare;

listExprs: listExpr
         | /* epsilon */;

listExpr: expr Comma listExpr
        | expr;

getIth: OpenSquare expr CloseSquare;

funCall: OpenPar listIDs ClosePar;

listIDs: listID
       | /* epsilon */;

listID: ID Comma listID
      | ID;

operator_: Plus
         | Minus
         | Star
         | Slash
         | IsEqual
         | NotEqual
         | And
         | Or
         | Less
         | Greater
         | Leq
         | Geq;

bool_: TRUE | FALSE;

// Type stuff

type: INT_TYPE                              /* int */
    | CHAR_TYPE                             /* char */
    | BOOL_TYPE                             /* bool */
    | Star type                             /* pointer */
    | QuestionMark type                     /* optional */
    | OpenSharp CloseSharp                  /* 0-tuple */
    | OpenSharp typeList CloseSharp         /* tuple */
    | OpenSquare type Comma INT CloseSquare /* array */
    | OpenPar type Arrow type ClosePar;     /* function type */

typeList: type Comma typeList
        | type;

// Lexer rules

WHILE: 'while';

IF  : 'if';
ELIF: 'elif';
ELSE: 'else';

RETURN: 'return';
CONTINUE: 'continue';
BREAK: 'break';

TRUE: 'T';
FALSE: 'F';

INT_TYPE:  '#';
CHAR_TYPE: '@';
BOOL_TYPE: '~';
AUTO:      '_';

INT: DIGIT+;
fragment DIGIT: [0-9];

CHAR: '\'' [\u0032-\u0126] '\'';

STRING: '"' CHAR* '"';

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z];

WS: [ \t\r\n]+ -> channel(HIDDEN);

IsEqual:       '==';
NotEqual:      '!=';
And:           '&&';
Or:            '||';
Leq:           '<=';
Geq:           '>=';
Less:     OpenSharp;
Greater: CloseSharp;

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
Slash:         '/';
