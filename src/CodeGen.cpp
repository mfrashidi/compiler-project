#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
{
  class ToIRVisitor : public ASTVisitor
  {
    Module *M;
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;

    Value *V;
    Value *tmp1;
    Value *tmp2;
    StringMap<AllocaInst *> nameMap;

  public:
    // Constructor for the visitor class.
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      // Initialize LLVM types and constants.
      VoidTy = Type::getVoidTy(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext());
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);
    }

    // Entry point for generating LLVM IR from the AST.
    void run(AST *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // Create a basic block for the entry point of the main function.
      BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      Builder.SetInsertPoint(BB);

      // Visit the root node of the AST to generate IR.
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);
    }

    // Visit function for the GSM node in the AST.
    virtual void visit(GSM &Node) override
    {
      // Iterate over the children of the GSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    virtual void visit(Assignment &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      Node.getRight()->accept(*this);
      Value *val = V;

      // Get the name of the variable being assigned.
      auto varName = Node.getLeft()->getVal();

      // Create a store instruction to assign the value to the variable.
      Builder.CreateStore(val, nameMap[varName]);

      // Create a function type for the "gsm_write" function.
      FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

      // Create a function declaration for the "gsm_write" function.
      Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

      // Create a call instruction to invoke the "gsm_write" function with the value.
      CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
    };

    virtual void visit(Factor &Node) override
    {
      if (Node.getKind() == Factor::Ident)
      {
        // If the factor is an identifier, load its value from memory.
        V = Builder.CreateLoad(Int32Ty, nameMap[Node.getVal()]);
      }
      else
      {
        // If the factor is a literal, convert it to an integer and create a constant.
        int intval;
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

    virtual void visit(BinaryOp &Node) override
    {
      // Visit the left-hand side of the binary operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;

      // Visit the right-hand side of the binary operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;

      // Perform the binary operation based on the operator type and create the corresponding instruction.
      switch (Node.getOperator())
      {
      case BinaryOp::Plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
      case BinaryOp::Minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
      case BinaryOp::Mul:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case BinaryOp::Div:
        V = Builder.CreateSDiv(Left, Right);
        break;
      case BinaryOp::Mod: {
        tmp1 = Builder.CreateSDiv(Left, Right);
        tmp2 = Builder.CreateNSWMul(tmp1, Right);
        V = Builder.CreateNSWSub(Left, tmp2);
        break;
      }
      case BinaryOp::Power:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case BinaryOp::Or:
        V = Builder.CreateOr(Left, Right);
        break;
      case BinaryOp::And:
        V = Builder.CreateAnd(Left, Right);
        break;
      case BinaryOp::DoubleEqual:
        V = Builder.CreateICmpEQ(Left, Right);
        break;
      case BinaryOp::NotEqual:
        V = Builder.CreateICmpNE(Left, Right);
        break;
      case BinaryOp::GreaterEqual:
        V = Builder.CreateICmpSGE(Left, Right);
        break;
      case BinaryOp::LowerEqual:
        V = Builder.CreateICmpSLE(Left, Right);
        break;
      case BinaryOp::Greater:
        V = Builder.CreateICmpSGT(Left, Right);
        break;
      case BinaryOp::Lower:
        V = Builder.CreateICmpSLT(Left, Right);
        break;
      }
    };

    virtual void visit(Declaration &Node) override {
      Value *val = nullptr;

      
      auto Ie = Node.begin_exprs();
      auto Ee = Node.end_exprs();

      int count_vars = 0;
      int count_exprs = 0;
      bool finishedExprs = false;

      // Iterate over the variables declared in the declaration statement.
      for (auto I = Node.begin_vars(), E = Node.end_vars(); I != E; ++I, ++count_vars, ++count_exprs) {
        StringRef Var = *I;

        if (Ie != Ee) {
          (* Ie) -> accept(*this);
          val = V;
          Ie++;
        } else if (Ie == Ee || finishedExprs) {
          finishedExprs = true;
          val = Builder.getInt1(0);
        }
      
        // Create an alloca instruction to allocate memory for the variable.
        nameMap[Var] = Builder.CreateAlloca(Int32Ty);

        // Store the initial value (if any) in the variable's memory location.
        if (val != nullptr) {
          Builder.CreateStore(val, nameMap[Var]);
        }
      }
      while (Ie != Ee || count_exprs <= count_vars) {
        count_exprs++;
      }
      if (count_exprs > count_vars) {} //TODO: Should raise error
    };

    virtual void visit(IfElse &Node) override {
        llvm::errs() << "here1 \n";
        llvm::SmallVector<Expr *> conditions = Node.getConditions();
        llvm::SmallVector<llvm::SmallVector<Assignment *>> assignments = Node.getAssignments();
        int count_conditions = 0;
        int count_exprs = 0;
        llvm::errs() << "here2 \n";


        for (auto I = assignments.begin(), E = assignments.end(); I != E; ++I) count_exprs++;
        for (auto I = conditions.begin(), E = conditions.end(); I != E; ++I) count_conditions++;
        llvm::errs() << "here3 \n";


        if (count_exprs == count_conditions) {
          llvm::errs() << "here4 \n";

          int correct_index = -1;
          count_conditions = 0;
          for (auto I = conditions.begin(), E = conditions.end(); I != E; ++I) {
            llvm::errs() << "here44 \n";
            Factor* f = (Factor *) I;
            int condition_val;
            f -> getVal().getAsInteger(10, condition_val);
            if (condition_val != 0) {
              correct_index = count_conditions;
              break;
            }
            llvm::errs() << "here45 \n";
            count_conditions++;
          }
          llvm::errs() << "here5 \n";

          if (correct_index == -1) return;

          count_exprs = 0;
          for (auto I = assignments.begin(), E = assignments.end(); I != E; ++I) {
            if (count_exprs == correct_index) {
              llvm::errs() << "here6 \n";

              for (auto II = I -> begin(), EE = E -> end(); II != EE; ++II) {
                (*II) -> getRight()->accept(*this);
                Value *val = V;

                auto varName = (*II)->getLeft()->getVal();

                // Create a store instruction to assign the value to the variable.
                Builder.CreateStore(val, nameMap[varName]);

                // Create a function type for the "gsm_write" function.
                FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

                // Create a function declaration for the "gsm_write" function.
                Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

                // Create a call instruction to invoke the "gsm_write" function with the value.
                CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
                llvm::errs() << "here7 \n";

              }
              return;
            }
            count_exprs++;
          }
        } else if (count_conditions + 1 == count_exprs) {
          int correct_index = -1;
          count_conditions = 0;
          for (auto I = conditions.begin(), E = conditions.end(); I != E; ++I) {
            Factor* f = (Factor *) I;
            int condition_val;
            f -> getVal().getAsInteger(10, condition_val);
            if (condition_val != 0) {
              correct_index = count_conditions;
              break;
            }
            count_conditions++;
          }
          if (correct_index == -1) {
            correct_index = count_exprs;
          }

          count_exprs = 0;
          for (auto I = assignments.begin(), E = assignments.end(); I != E; ++I) {
            if (count_exprs == correct_index) {
              for (auto II = I -> begin(), EE = E -> end(); II != EE; ++II) {
                (*II) -> getRight()->accept(*this);
                Value *val = V;

                auto varName = (*II)->getLeft()->getVal();

                // Create a store instruction to assign the value to the variable.
                Builder.CreateStore(val, nameMap[varName]);

                // Create a function type for the "gsm_write" function.
                FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

                // Create a function declaration for the "gsm_write" function.
                Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

                // Create a call instruction to invoke the "gsm_write" function with the value.
                CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
              }
              return;
            }
            count_exprs++;
          }

        } else {
          //TODO: Should raise error
        }
    };

  virtual void visit(Loop &Node) override {
        llvm::SmallVector<Assignment *> assignments = Node.getAssignments();

        Factor* f = (Factor *) Node.getCondition();
        int condition_val;
        f -> getVal().getAsInteger(10, condition_val);
        while (condition_val != 0) {
          for (auto I = assignments.begin(), E = assignments.end(); I != E; ++I) {
            (*I) -> getRight()->accept(*this);
            Value *val = V;

            auto varName = (*I) -> getLeft()->getVal();

            // Create a store instruction to assign the value to the variable.
            Builder.CreateStore(val, nameMap[varName]);

            // Create a function type for the "gsm_write" function.
            FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

            // Create a function declaration for the "gsm_write" function.
            Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

            // Create a call instruction to invoke the "gsm_write" function with the value.
            CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});

            f = (Factor *) Node.getCondition();
            int condition_val;
            f -> getVal().getAsInteger(10, condition_val);
        }
    };
  };
};
}; // namespace

void CodeGen::compile(AST *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("calc.expr", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  ToIRVisitor ToIR(M);
  ToIR.run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}
