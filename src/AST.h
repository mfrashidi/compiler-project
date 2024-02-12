#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

// Forward declarations of classes used in the AST
class AST;
class Expr;
class GSM;
class Factor;
class BinaryOp;
class Assignment;
class Declaration;
class IfElse;
class Loop;
class Print;

// ASTVisitor class defines a visitor pattern to traverse the AST
class ASTVisitor
{
public:
  // Virtual visit functions for each AST node type
  virtual void visit(AST &) {}               // Visit the base AST node
  virtual void visit(Expr &) {}              // Visit the expression node
  virtual void visit(GSM &) = 0;             // Visit the group of expressions node
  virtual void visit(Factor &) = 0;          // Visit the factor node
  virtual void visit(BinaryOp &) = 0;        // Visit the binary operation node
  virtual void visit(Assignment &) = 0;      // Visit the assignment expression node
  virtual void visit(Declaration &) = 0;     // Visit the variable declaration node
  virtual void visit(IfElse &) {}     // Visit the variable declaration node
  virtual void visit(Loop &) {}     // Visit the variable declaration node
  virtual void visit(Print &) {}     // Visit the variable declaration node
};

// AST class serves as the base class for all AST nodes
class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0;    // Accept a visitor for traversal
};

// Expr class represents an expression in the AST
class Expr : public AST
{
public:
  Expr() {}
};


// GSM class represents a group of expressions in the AST
class GSM : public Expr
{
  using ExprVector = llvm::SmallVector<Expr *>;

private:
  ExprVector exprs;                          // Stores the list of expressions

public:
  GSM(llvm::SmallVector<Expr *> exprs) : exprs(exprs) {}

  llvm::SmallVector<Expr *> getExprs() { return exprs; }

  ExprVector::const_iterator begin() { return exprs.begin(); }

  ExprVector::const_iterator end() { return exprs.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Factor class represents a factor in the AST (either an identifier or a number)
class Factor : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind;                            // Stores the kind of factor (identifier or number)
  llvm::StringRef Val;                       // Stores the value of the factor

public:
  Factor(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) {}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// BinaryOp class represents a binary operation in the AST (plus, minus, multiplication, division and etc)
class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    Power,
    Or,
    And,
    DoubleEqual,
    NotEqual,
    GreaterEqual,
    LowerEqual,
    Greater,
    Lower
  };

private:
  Expr *Left;                               // Left-hand side expression
  Expr *Right;                              // Right-hand side expression
  Operator Op;                              // Operator of the binary operation

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Assignment class represents an assignment expression in the AST
class Assignment : public Expr
{
public:
  enum Type
  {
    Equal,
    EqualPlus,
    EqualMinus,
    EqualStar,
    EqualSlash,
    EqualMod
  };

private:
  Factor *Left;                             // Left-hand side factor (identifier)
  Expr *Right;                              // Right-hand side expression
  Type type;                              // Right-hand side expression

public:

  Assignment(Factor *L, Expr *R, Type T) : Left(L), Right(R), type(T) {}

  Factor *getLeft() { return Left; }

  Expr *getRight() { 
    if (type == Type::Equal)
      return Right;
    else if (type == EqualPlus) 
      return new BinaryOp(BinaryOp::Operator::Plus, Left, Right);
    else if (type == EqualMinus) 
      return new BinaryOp(BinaryOp::Operator::Minus, Left, Right);
    else if (type == EqualMod) 
      return new BinaryOp(BinaryOp::Operator::Mod, Left, Right);
    else if (type == EqualSlash) 
      return new BinaryOp(BinaryOp::Operator::Div, Left, Right);
    else if (type == EqualStar) 
      return new BinaryOp(BinaryOp::Operator::Mul, Left, Right);
    return Right;
  }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class Declaration : public Expr
{
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  using ExprVector = llvm::SmallVector<Expr *, 8>;
  VarVector Vars;                           // Stores the list of variables
  ExprVector Exprs;       // Expression serving as the initializer

public:
  Declaration(llvm::SmallVector<llvm::StringRef, 8> Vars, llvm::SmallVector<Expr *, 8> Expr) : Vars(Vars), Exprs(Expr) {}

  VarVector::const_iterator begin_vars() { return Vars.begin(); }

  VarVector::const_iterator end_vars() { return Vars.end(); }

  ExprVector::const_iterator begin_exprs() { return Exprs.begin(); }

  ExprVector::const_iterator end_exprs() { return Exprs.end(); }

  llvm::SmallVector<Expr *, 8> getExprs() { return Exprs; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class IfElse : public Expr
{
private:
  llvm::SmallVector<Expr *> conditions;
  llvm::SmallVector<llvm::SmallVector<Assignment *>> assignments;

public:
  IfElse(llvm::SmallVector<Expr *> c, llvm::SmallVector<llvm::SmallVector<Assignment *>> a) : conditions(c), assignments(a) {}

  llvm::SmallVector<llvm::SmallVector<Assignment *>> getAssignments() { return assignments; }
  llvm::SmallVector<Expr *> getConditions() { return conditions; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class Loop : public Expr
{
private:
  Expr *Condition;
  llvm::SmallVector<Assignment *> assignments;

public:
  Loop(Expr *c, llvm::SmallVector<Assignment *> a) : Condition(c), assignments(a) {}

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }

  Expr *getCondition() { return Condition; }

  llvm::SmallVector<Assignment *> getAssignments() { return assignments; }
};

class Print : public Expr
{
private:
  Expr *E;

public:
  Print(Expr *e) : E(e) {}

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }

  Expr *getExpr() { return E; }
};

#endif
