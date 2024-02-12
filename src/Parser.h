#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

class Parser
{
    Lexer &Lex;    // retrieve the next token from the input
    Token Tok;     // stores the next token
    bool HasError; // indicates if an error was detected

    void error(){
        llvm::errs() << "Unexpected: " << Tok.getText() << "\n";
        HasError = true;
    }

    void error(const char * inp){
        llvm::errs() << "Unexpected: " << Tok.getText() << ", Expected: " << inp << "\n";
        HasError = true;
    }

    // retrieves the next token from the lexer.expect()
    // tests whether the look-ahead is of the expected kind
    // void advance() { llvm::errs() << "Consumed: " << Tok.getText() << "\n"; Lex.next(Tok); }
    void advance() { Lex.next(Tok); }

    bool expect(Token::TokenKind Kind)
    {
        if (Tok.getKind() != Kind)
        {
            // llvm::errs() << "Got: " << Tok.getText() << ", " << Tok.getKind() << " Expect: " << Kind << "\n";
            error();
            return true;
        }
        return false;
    }

    // retrieves the next token if the look-ahead is of the expected kind
    bool consume(Token::TokenKind Kind)
    {
        if (expect(Kind))
            return true;
        advance();
        return false;
    }

    AST *parseGSM();
    Expr *parseDec();
    Assignment *parseAssign();
    Expr *parseExpr();
    Expr *parseExpr1();
    Expr *parseExpr2();
    Expr *parseExpr3();
    Expr *parseExpr4();
    Expr *parseExpr5();
    Expr *parseExpr6();
    Expr *parseExpr7();
    Expr *parseFactor();
    Expr *parseIfElse();
    Expr *parseLoop();
    Expr *parsePrint();
    void parseComment();

public:
    // initializes all members and retrieves the first token
    Parser(Lexer &Lex) : Lex(Lex), HasError(false)
    {
        advance();
    }

    // get the value of error flag
    bool hasError() { return HasError; }

    AST *parse();
};

#endif