grammar DecafParser;
options { tokenVocab=DecafLexer; }

// Top level
topLevel:
    classDef+;

extendClause:
    EXTENDS id;

classDef:
    CLASS id (extendClause)? LBRACE field* RBRACE;

field:
    varDef
    | methodDef;

varDef:
    var SEMI;

methodDef:
    STATIC? type id LPAREN varList RPAREN block;

var:
    type id;

paraVarDef:
    var;

varList:
    paraVarDef (COMMA paraVarDef)*
    |; /* epsilon */


classType:
    CLASS id;


// Types
type:
    INT
    | BOOL
    | STRING
    | VOID
    | classType
    | type LBRACKET RBRACKET;


forInit:
    localVarDef
    | assign
    ;

forUpdate:
    assign
    | expr
    ;

forControl:
    forInit? SEMI expr SEMI forUpdate?;

localVarDef:
    var (bop=ASSIGN expr)?;

assign:
    lValue bop=ASSIGN expr;

// statements
ifStmt:
    IF LPAREN expr RPAREN stmt (ELSE stmt)?;

whileStmt:
    WHILE LPAREN expr RPAREN stmt;

forStmt:
    FOR LPAREN forControl RPAREN stmt;

breakStmt:
    BREAK SEMI;

returnStmt:
    RETURN expr? SEMI;

printStmt:
    PRINT LPAREN exprList RPAREN SEMI;

emptyStmt:
    SEMI;

localVarDefStmt:
    localVarDef SEMI;

assignStmt:
    assign SEMI;

exprStmt:
    expr SEMI;

// Statements
stmt:
    block
    | emptyStmt
    | exprStmt
    | assignStmt
    | ifStmt
    | whileStmt
    | forStmt
    | breakStmt
    | returnStmt
    | printStmt
    ;

block:
    LBRACE (blockStmt)* RBRACE;

blockStmt:
    localVarDefStmt
    | stmt
    ;

lValue:
    (expr DOT)? id # varSelLValue
    | expr LBRACKET expr RBRACKET # indexSelLValue
    ;

// Expressions
expr:
    lit # litExpr
    | THIS # thisExpr

    // Cannot just use 'lValue' as it's left-recursive
    | id # idExpr
    
    | LPAREN expr RPAREN # parenExpr
    | expr DOT id # varSelExpr
    | expr LBRACKET expr RBRACKET # indexSelExpr
    | expr DOT id LPAREN exprList RPAREN # varCallExpr
    | id LPAREN exprList RPAREN # localCallExpr

    // Unary Operator
    | uop=NOT expr # unaryNotExpr
    | uop=SUB expr # unarySubExpr
    | LPAREN CLASS id RPAREN expr # castExpr

    // Binary Operator
    | expr bop=(MUL | DIV | MOD) expr # multiplicativeExpr
    | expr bop=(ADD | SUB) expr # addictiveExpr
    | expr bop=(LE | LT | GE | GT) expr # relationExpr
    | expr bop=(EQ | NE) expr # equalityExpr
    | expr bop=AND expr # logicalAndExpr
    | expr bop=OR expr # logicalOrExpr
    
    | READINTEGER LPAREN RPAREN # readIntExpr
    | READLINE LPAREN RPAREN # readLineExpr
    | NEW id LPAREN RPAREN # classNewExpr
    | NEW type LBRACKET expr RBRACKET # arrayNewExpr
    | INSTANCEOF LPAREN expr COMMA id RPAREN  # instanceofExpr
    ;

lit:
    INTLIT # intLit
    | (TRUE | FALSE) # boolLit
    | NULLLIT # nullLit
    | STRING_LIT # stringLit
    ;

unaryOp:
    SUB
    | NOT;

exprList:
    expr (COMMA expr)*
    |; /* epsilon */

id:
    IDENTIFIER;