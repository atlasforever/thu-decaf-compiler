lexer grammar DecafLexer;


BOOL            : 'bool';
BREAK           : 'break';
CLASS           : 'class';
ELSE            : 'else';
EXTENDS         : 'extends';
FOR             : 'for';
IF              : 'if';
INT             : 'int';
NEW             : 'new';
NULLLIT         : 'null';
RETURN          : 'return';
STRING          : 'string';
THIS            : 'this';
VOID            : 'void';
WHILE           : 'while';
STATIC          : 'static';
PRINT           : 'Print';
READINTEGER     : 'ReadInteger';
READLINE        : 'ReadLine';
INSTANCEOF      : 'instanceof';

TRUE            : 'true';
FALSE           : 'false';

ADD             : '+';
SUB             : '-';
MUL             : '*';
DIV             : '/';
MOD             : '%';
LT              : '<';
LE              : '<=';
GT              : '>';
GE              : '>=';
ASSIGN          : '=';
EQ              : '==';
NE              : '!=';
AND             : '&&';
OR              : '||';
NOT             : '!';
SEMI            : ';';
COMMA           : ',';
DOT             : '.';
LBRACKET        : '[';
RBRACKET        : ']';
LPAREN          : '(';
RPAREN          : ')';
LBRACE          : '{';
RBRACE          : '}';


fragment LETTER : [a-zA-Z];
fragment DIGIT  : [0-9];

INTLIT          : (DIGIT+ | '0x'(DIGIT | [a-f] | [A-F])+);

IDENTIFIER      : LETTER(LETTER | DIGIT | '_')*;
WHITESPACE      : [ \t\r\n]+ -> skip;

COMMENT         : '//'.*? (('\r')?'\n') -> skip;

STRING_LIT      : '"' -> pushMode(IN_STRING);

UNKNWON_TOKEN   : .;
// ------------ INSIDE STRING MODE ------------
mode IN_STRING;
STRING_EOF       : EOF -> popMode;
STRING_TERM      : '"' -> popMode;
ILL_ESCAPE       : ('\\'~["ntr\\]);
ILL_NEWLINE      : ('\n' | '\r\n');
ALLOWED_CHARS    : ( '\\'["ntr\\] | ~["\r\n\\])+;

