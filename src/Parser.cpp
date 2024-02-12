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
                    error();
                break;
            case Token::ident:
                a = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                    error();
                }
                if (a)
                    exprs.push_back(a);
                else
                    error();
                break;
            case Token::ifc:
                // llvm::SmallVector<Expr *> exprs;
                d = parseIfElse();

                // if (!exprs.empty()){
                //     for (Expr *a : exprs) {
                //         exprs.push_back(a);
                //     }
                // } else error();
                if (d){
                    exprs.push_back(d);
                } else error();
                break;
            case Token::loopc:
                // llvm::SmallVector<Expr *> exprs;
                // exprs = parseLoop();

                // if (!exprs.empty()){
                //     for (Expr * a : exprs) {
                //         exprs.push_back(a);
                //     }
                // } else error();

                d = parseLoop();

                // if (!exprs.empty()){
                //     for (Expr *a : exprs) {
                //         exprs.push_back(a);
                //     }
                // } else error();
                if (d){
                    exprs.push_back(d);
                } else error();
                break;
            default:
                error();
                break;
        }
        advance(); // TODO: watch this part
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

    if (expect(Token::semicolon) || count_exprs > count_vars) {
        error();
        goto _error;
    }


    return new Declaration(Vars, Exprs);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Assignment *Parser::parseAssign()
{
    Expr *E;
    Factor *F;
    F = (Factor *)(parseFactor());

    if (!Tok.is(Token::equal))
    {
        error();
        return nullptr;
    }

    advance();
    E = parseExpr();
    return new Assignment(F, E);
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
        BinaryOp::Operator Op;
        if (Tok.is(Token::star)) BinaryOp::Operator Op = BinaryOp::Mul;
        else if (Tok.is(Token::slash)) BinaryOp::Operator Op = BinaryOp::Div;
        else BinaryOp::Operator Op = BinaryOp::Mod;

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
        if (!consume(Token::r_paren))
            break;
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
                error((const char *)"kossher");
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
    
    return new Loop(Condition, assignments);
    _error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}