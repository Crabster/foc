grammar Foc;

// Parser rules ***********************************************************************************

program: decls EOF;

decls: funDecl decls
     | COMMENT decls
     | /* epsilon */;

comment: COMMENT;

funDecl: type ID OpenPar funArgs ClosePar OpenCurly funBody CloseCurly;

funArgs: funArg
       | /* epsilon */;

funArg: type ID Comma funArg
      | type ID;

funBody: varDecl funBody
       | assignment funBody
       | flow funBody
       | comment funBody
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

listIDs: listID
       | /* epsilon */;

listID: ID Comma listID
      | ID;

assignment: assignExpr Equal expr Semicolon
          | ID Equal expr Semicolon;

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

// Expression stuff *******************************************************************************

expr: expr_ operator_ expr
    | expr_ funCall
    | assignExpr
    | expr_;

expr_: typeExpr
     | OpenPar expr ClosePar
     | ID
     | Minus expr;

assignExpr: expr_ OpenSquare expr CloseSquare
          | expr_ OpenSharp expr CloseSharp;

funCall: OpenPar listExprs ClosePar; // funCall
//       | OpenPar listExprs ClosePar;

typeExpr: INT
        | CHAR
        | STRING
        | bool_
        | ptrExpr
        | optExpr
        | tupleExpr
        | arrayExpr;

ptrExpr: Ampersand expr
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

// Lexer rules ************************************************************************************

COMMENT: Slash Star (UnescapedChar | ' ')* Star Slash;

WHILE: 'while';

IF  : 'if';
ELIF: 'elif';
ELSE: 'else';

RETURN: 'return';
CONTINUE: 'continue';
BREAK: 'break';

TRUE: 'T';
FALSE: 'F';

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z];

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
QuestionMark:  '?';
ExclMark:      '!';
Dollar:        '$';
Slash:         '/';

CHAR: '\'' UnescapedChar '\'';
STRING: '"' UnescapedChar* '"';
fragment UnescapedChar: [\u0032-\u0126];

WS: [ \t\r\n]+ -> channel(HIDDEN);
