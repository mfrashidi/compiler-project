#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}

AST *Parser::parseGSM()
{
    llvm::SmallVector<Expr *> exprs;
    Expr *d;
    Expr *a;
    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind()) {
            case Token::KW_int:
                d = parseDec();
                if (d)
                    exprs.push_back(d);
                else
                    break;
                if (expect(Token::semicolon))
                    error();
                advance();
                break;
            case Token::KW_print:
                d = parsePrint();
                if (d)
                    exprs.push_back(d);
                else
                    error();
                break;
            case Token::ident:
                a = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                }
                if (a)
                    exprs.push_back(a);
                else
                    error();
                if (expect(Token::semicolon))
                    error();
                advance();
                break;
            case Token::ifc:
                d = parseIfElse();
                if (d){
                    exprs.push_back(d);
                } else error();
                break;
            case Token::loopc:
                d = parseLoop();
                if (d){
                    exprs.push_back(d);
                } else error();
                break;
            case Token::start_comment:
                parseComment();
                if (!Tok.is(Token::end_comment))
                    error();
                advance();
                break;
            default:
                error();
                break;
        }
    }
    return new GSM(exprs);

_error2:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseDec()
{
    Expr *E;
    int count_vars = 0;
    int count_exprs = 0;
    llvm::SmallVector<llvm::StringRef, 8> Vars;
    llvm::SmallVector<Expr *, 8> Exprs;
    if (expect(Token::KW_int)) {
        error();
        goto _error;
    }


    advance();

    if (expect(Token::ident)) {
        error();
        goto _error;
    }

    Vars.push_back(Tok.getText());
    advance();

    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::ident)) {
            error();
            goto _error;
        }

        Vars.push_back(Tok.getText());
        count_vars++;
        advance();
    }

    if (Tok.is(Token::equal))
    {
        advance();
        E = parseExpr();
        Exprs.push_back(E);

        while (Tok.is(Token::comma))
        {
            advance();
            E = parseExpr();
            Exprs.push_back(E);
            count_exprs++;
        }
    }

    if (!Tok.is(Token::semicolon)) {
        error((const char *)";");
        goto _error;
    }


    return new Declaration(Vars, Exprs);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    
    return nullptr;
}

Expr *Parser::parsePrint()
{
    Expr *E;

    if (expect(Token::KW_print)) {
        error();
        goto _error;
    }

    advance();

    E = parseExpr();

    if (expect(Token::semicolon)) {
        error();
        goto _error;
    }
    advance();

    return new Print(E);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Assignment *Parser::parseAssign()
{
    Expr *E;
    Factor *F;
    Assignment::Type T;
    F = (Factor *)(parseFactor());

    if (Tok.is(Token::equal)){
        T = Assignment::Type::Equal;
    } else if (Tok.is(Token::equal_minus)){
        T = Assignment::Type::EqualMinus;
    } else if (Tok.is(Token::equal_mod)){
        T = Assignment::Type::EqualMod;
    } else if (Tok.is(Token::equal_plus)){
        T = Assignment::Type::EqualPlus;
    } else if (Tok.is(Token::equal_slash)){
        T = Assignment::Type::EqualSlash;
    } else if (Tok.is(Token::equal_star)){
        T = Assignment::Type::EqualStar;
    } else {
        error();
        return nullptr;
    }

    advance();
    E = parseExpr();
    return new Assignment(F, E, T);
}

Expr *Parser::parseExpr()
{
    Expr *Left = parseExpr1();
    while (Tok.is(Token::orc))
    {
        BinaryOp::Operator Op = BinaryOp::Or;
        advance();
        Expr *Right = parseExpr1();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr1()
{
    Expr *Left = parseExpr2();
    while (Tok.is(Token::andc))
    {
        BinaryOp::Operator Op = BinaryOp::And;
        advance();
        Expr *Right = parseExpr2();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr2()
{
    Expr *Left = parseExpr3();
    while (Tok.isOneOf(Token::double_equal, Token::not_equal))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::double_equal) ? BinaryOp::DoubleEqual : BinaryOp::NotEqual;
        advance();
        Expr *Right = parseExpr3();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr3()
{
    Expr *Left = parseExpr4();
    while (Tok.isOneOf(Token::greater_equal, Token::lower_equal))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::greater_equal) ? BinaryOp::GreaterEqual : BinaryOp::LowerEqual;
        advance();
        Expr *Right = parseExpr4();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr4()
{
    Expr *Left = parseExpr5();
    while (Tok.isOneOf(Token::greater, Token::lower))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::greater) ? BinaryOp::Greater : BinaryOp::Lower;
        advance();
        Expr *Right = parseExpr5();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr5()
{
    Expr *Left = parseExpr6();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
        advance();
        Expr *Right = parseExpr6();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr6()
{
    Expr *Left = parseExpr7();
    while (Tok.isOneOf(Token::star, Token::slash, Token::module))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::star) ? BinaryOp::Mul : (Tok.is(Token::slash) ? BinaryOp::Div : BinaryOp::Mod);
        advance();
        Expr *Right = parseExpr7();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseExpr7()
{
    Expr *Left = parseFactor();
    while (Tok.is(Token::power))
    {
        BinaryOp::Operator Op = BinaryOp::Power;
        advance();
        Expr *Right = parseFactor();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseFactor()
{
    Expr *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::number:
        Res = new Factor(Factor::Number, Tok.getText());
        advance();
        break;
    case Token::ident:
        Res = new Factor(Factor::Ident, Tok.getText());
        advance();
        break;
    case Token::l_paren:
        advance();
        Res = parseExpr();
        if (!expect(Token::r_paren)) {
            advance();
            break;
        }
    default: // error handling
        if (!Res)
            error();
        while (!Tok.isOneOf(Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi))
            advance();
        break;
    }
    return Res;
}

Expr *Parser::parseIfElse()
{
    Assignment *A;
    Expr *E;
    llvm::SmallVector<Expr *> conditions;
    llvm::SmallVector<llvm::SmallVector<Assignment *>> assignments;

    if (expect(Token::ifc))
        error((const char *)"ifc");
    advance();

    E = parseExpr();
    conditions.push_back(E);

    if (expect(Token::colon))
        error((const char *)"colon");
    advance();

    if (expect(Token::begin))
        error((const char *)"begin");
    advance();

    llvm::SmallVector<Assignment *> currentAssignments;
    while (!Tok.is(Token::end)) {
        if (Tok.is(Token::ident)) {
            A = parseAssign();

            if (!Tok.is(Token::semicolon))
            {
                error((const char *)";");
            }
            advance();
            if (A)
                currentAssignments.push_back(A);
            else
                error((const char *)"random");
        } else error((const char *)"ident");
    }
    assignments.push_back(currentAssignments);
    if (expect(Token::end))
        error((const char *)"end");
    advance();

    while (Tok.is(Token::elif)) {
        if (expect(Token::elif))
            error();
        advance();

        E = parseExpr();
        conditions.push_back(E);

        if (expect(Token::colon))
            error();
        advance();

        if (expect(Token::begin))
            error();
        advance();

        currentAssignments.clear();
        while (!Tok.is(Token::end)) {
            if (Tok.is(Token::ident)) {
                A = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                }
                advance();
                if (A)
                    currentAssignments.push_back(A);
                else
                    error();
            } else error();
        }
        assignments.push_back(currentAssignments);
        advance();
    }

    if (Tok.is(Token::elsec)) {
        advance();

        if (expect(Token::colon))
            error();
        advance();

        if (expect(Token::begin))
            error();
        advance();

        currentAssignments.clear();
        while (!Tok.is(Token::end)) {
            if (Tok.is(Token::ident)) {
                A = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                    goto _error;
                }
                advance();
                if (A)
                    currentAssignments.push_back(A);
                else
                    error();
            } else error();
        }
        assignments.push_back(currentAssignments);
        advance();
    }
    
    return new IfElse(conditions, assignments);
    _error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseLoop()
{
    Assignment *A;
    Expr *Condition;
    llvm::SmallVector<Assignment *> assignments;

    if (expect(Token::loopc)) {
        error();
        goto _error;
    }

    advance();

    Condition = parseExpr();

    if (expect(Token::colon)) {
        error();
        goto _error;
    }

    advance();

    if (expect(Token::begin)) {
        error();
        goto _error;
    }
    advance();

    while (!Tok.is(Token::end)) {
        if (Tok.is(Token::ident)) {
            A = parseAssign();

            if (!Tok.is(Token::semicolon)) {
                error();
                goto _error;
            }
            advance();
            if (A)
                assignments.push_back(A);
            else {
                error();
                goto _error;
            }
        } else {
            error();
                goto _error;
        }

    }

    if (expect(Token::end)) {
        error();
        goto _error;
    }
    advance();
    
    return new Loop(Condition, assignments);
    _error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

void Parser::parseComment()
{
    if (expect(Token::start_comment)) {
        error();
        goto _error;
    }
    advance();

    while (!Tok.isOneOf(Token::end_comment, Token::eoi)) advance();

    return;
    _error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
}