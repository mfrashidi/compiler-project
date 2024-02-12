#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

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
        else if (Name == "print")
            kind = Token::KW_print;
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
    } else if (charinfo::isSpecialCharacter(*BufferPtr)) {
        const char *endWithOneLetter = BufferPtr + 1;
        const char *endWithTwoLetter = BufferPtr + 2;
        const char *end;
        llvm::StringRef NameWithOneLetter(BufferPtr, endWithOneLetter - BufferPtr);
        llvm::StringRef NameWithTwoLetter(BufferPtr, endWithTwoLetter - BufferPtr);
        Token::TokenKind kind;
        bool isFound = false;
        if (NameWithTwoLetter == "=="){
            kind = Token::double_equal;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithOneLetter == "=") {
            kind = Token::equal;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithTwoLetter == "!="){
            kind = Token::not_equal;
            isFound = true;
            end = endWithTwoLetter;
        }else if (NameWithTwoLetter == "+="){
            kind = Token::equal_plus;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "-="){
            kind = Token::equal_minus;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "*="){
            kind = Token::equal_star;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "*/"){
            kind = Token::end_comment;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "/="){
            kind = Token::equal_slash;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "/*"){
            kind = Token::start_comment;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "%="){
            kind = Token::equal_mod;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == ">="){
            kind = Token::greater_equal;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "<="){
            kind = Token::lower_equal;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithOneLetter == "+"){
            kind = Token::plus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "-"){
            kind = Token::minus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "*"){
            kind = Token::star;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "/"){
            kind = Token::slash;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "+"){
            kind = Token::plus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ">"){
            kind = Token::greater;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "<"){
            kind = Token::lower;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "("){
            kind = Token::l_paren;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ")"){
            kind = Token::r_paren;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ";"){
            kind = Token::semicolon;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ","){
            kind = Token::comma;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "%"){
            kind = Token::module;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "^"){
            kind = Token::power;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ":"){
            kind = Token::colon;
            isFound = true;
            end = endWithOneLetter;
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
