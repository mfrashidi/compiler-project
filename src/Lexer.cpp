#include "Lexer.h"

// classifying characters
namespace charinfo
{
    // ignore whitespaces
    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
               c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    LLVM_READNONE inline bool isSpecialCharacter(char c)
    {
        return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '!' || c == '>' || c == '<' || c == '(' || c == ')' || c == ',' || c == ';' || c == '%' || c == '^' || c == ':';
    }
}

void Lexer::next(Token &token) {
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr)) {
        ++BufferPtr;
    }
    // make sure we didn't reach the end of input
    if (!*BufferPtr) {
        token.Kind = Token::eoi;
        return;
    }
    // collect characters and check for keywords or ident
    if (charinfo::isLetter(*BufferPtr)) {
        const char *end = BufferPtr + 1;
        while (charinfo::isLetter(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name == "loopc")
            kind = Token::loopc;
        else if (Name == "if")
            kind = Token::ifc;
        else if (Name == "elif")
            kind = Token::elif;
        else if (Name == "else")
            kind = Token::elsec;
        else if (Name == "begin")
            kind = Token::begin;
        else if (Name == "end")
            kind = Token::end;
        else if (Name == "and")
            kind = Token::andc;
        else if (Name == "or")
            kind = Token::orc;
        else
            kind = Token::ident;
        // generate the token
        formToken(token, end, kind);
        return;
    } else if (charinfo::isDigit(*BufferPtr)) { // check for numbers
        const char *end = BufferPtr + 1;
        while (charinfo::isDigit(*end))
            ++end;
        formToken(token, end, Token::number);
        return;
    } else if (charinfo::isSpecialCharacter(*BufferPtr)) { // check if starts with equal
        const char *end = BufferPtr + 1;
        while (charinfo::isSpecialCharacter(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        bool isFound = false;
        if (Name == "=") {
            kind = Token::equal;
            isFound = true;
        }
        else if (Name == "=="){
            kind = Token::double_equal;
            isFound = true;
        }
        else if (Name == "!="){
            kind = Token::not_equal;
            isFound = true;
        }
        else if (Name == "+="){
            kind = Token::equal_plus;
            isFound = true;
        }
        else if (Name == "-="){
            kind = Token::equal_minus;
            isFound = true;
        }
        else if (Name == "*="){
            kind = Token::equal_star;
            isFound = true;
        }
        else if (Name == "/="){
            kind = Token::equal_slash;
            isFound = true;
        }
        else if (Name == "%="){
            kind = Token::equal_mod;
            isFound = true;
        }
        else if (Name == ">="){
            kind = Token::greater_equal;
            isFound = true;
        }
        else if (Name == "<="){
            kind = Token::lower_equal;
            isFound = true;
        }
        else if (Name == "+"){
            kind = Token::plus;
            isFound = true;
        }
        else if (Name == "-"){
            kind = Token::minus;
            isFound = true;
        }
        else if (Name == "*"){
            kind = Token::star;
            isFound = true;
        }
        else if (Name == "/"){
            kind = Token::slash;
            isFound = true;
        }
        else if (Name == "+"){
            kind = Token::plus;
            isFound = true;
        }
        else if (Name == ">"){
            kind = Token::greater;
            isFound = true;
        }
        else if (Name == "<"){
            kind = Token::lower;
            isFound = true;
        }
        else if (Name == "("){
            kind = Token::l_paren;
            isFound = true;
        }
        else if (Name == ")"){
            kind = Token::r_paren;
            isFound = true;
        }
        else if (Name == ";"){
            kind = Token::semicolon;
            isFound = true;
        }
        else if (Name == ","){
            kind = Token::comma;
            isFound = true;
        }
        else if (Name == "%"){
            kind = Token::module;
            isFound = true;
        }
        else if (Name == "^"){
            kind = Token::power;
            isFound = true;
        }
        else if (Name == ":"){
            kind = Token::colon;
            isFound = true;
        }
        // generate the token
        if (isFound) formToken(token, end, kind);
        else formToken(token, BufferPtr + 1, Token::unknown); 
        return;
    } else {
        formToken(token, BufferPtr + 1, Token::unknown); 
        return;         
    }
    return;
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}
