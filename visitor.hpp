#pragma once

#include <ast.hpp>
#include <get_idx.hpp>
#include <visit.hpp>
#include <exceptions.hpp>

#include <iostream>
#include <limits>
#include <unordered_map>

namespace pas {
namespace visitor {

class Printer {
public:
  Printer(std::ostream &stream) : stream_(stream) {}

private:
  MAKE_VISIT_STMT_FRIEND();

  void print_indent() {
    for (size_t i = 0; i < depth_; ++i) {
      stream_ << ' ';
    }
  }

  void visit(pas::ast::ProgramModule &pm) {
    print_indent();
    stream_ << "ProgramModule name=" << pm.program_name_ << '\n';
    depth_ += 1;
    visit(pm.block_);
    depth_ -= 1;
  }
  void visit(pas::ast::Block &block) {
    print_indent();
    stream_ << "Block" << '\n';
    depth_ += 1;
    assert(block.decls_.get() != nullptr);
    visit(*block.decls_.get());
    for (auto &stmt : block.stmt_seq_) {
      visit_stmt(*this, stmt);
    }
    depth_ -= 1;
  }
  void visit(pas::ast::Declarations &decls) {
    print_indent();
    stream_ << "Declarations" << '\n';
    depth_ += 1;
    for (auto &const_def : decls.const_defs_) {
      visit(const_def);
    }
    for (auto &type_def : decls.type_defs_) {
      visit(type_def);
    }
    for (auto &var_decl : decls.var_decls_) {
      visit(var_decl);
    }
    for (auto &subprog_decl : decls.subprog_decls_) {
      visit(subprog_decl);
    }
    depth_ -= 1;
  }
  void visit(pas::ast::ConstDef &const_def) {
    print_indent();
    stream_ << "ConstDef name=" << const_def.ident_ << '\n';
    depth_ += 1;
    visit(const_def.const_expr_);
    depth_ -= 1;
  }
  void visit(pas::ast::TypeDef &type_def) {
    print_indent();
    stream_ << "TypeDef name=" << type_def.ident_ << '\n';
    depth_ += 1;
    visit(type_def.type_);
    depth_ -= 1;
  }
  void visit(pas::ast::VarDecl &var_decl) {
    print_indent();
    stream_ << "VarDecl names=";
    bool first = true;
    for (auto &name : var_decl.ident_list_) {
      if (!first) {
        stream_ << ',';
      }
      stream_ << name;
      first = false;
    }
    stream_ << '\n';
    depth_ += 1;
    visit(var_decl.type_);
    depth_ -= 1;
  }
  void visit(pas::ast::ConstExpr &const_expr) {
    print_indent();
    stream_ << "ConstExpr";
    depth_ += 1;
    if (const_expr.unary_op_.has_value()) {
        switch (const_expr.unary_op_.value()) {
        case pas::ast::UnaryOp::Plus: {
          stream_ << " unary_op=plus";
          break;
        }
        case pas::ast::UnaryOp::Minus: {
          stream_ << " unary_op=minus";
          break;
        }
        default:
          assert(false);
          __builtin_unreachable();
        }
      }
    stream_ << '\n';
    visit(const_expr.factor_);
    depth_ -= 1;
  }
  void visit(pas::ast::ConstFactor &const_factor) {
    print_indent();
    stream_ << "ConstFactor ";
    switch (const_factor.index()) {
    case get_idx(pas::ast::ConstFactorKind::Identifier): {
      stream_ << "identifier " << std::get<std::string>(const_factor) << '\n';
      break;
    }
    case get_idx(pas::ast::ConstFactorKind::Number): {
      stream_ << "number " << std::get<int>(const_factor) << '\n';
      break;
    }
    case get_idx(pas::ast::ConstFactorKind::Bool): {
      stream_ << "bool " << (std::get<bool>(const_factor) ? "true" : "false")
              << '\n';
      break;
    }
    case get_idx(pas::ast::ConstFactorKind::Nil): {
      stream_ << "nil" << '\n';
      break;
    }
    default:
      assert(false);
      __builtin_unreachable();
    }
  }
  void visit(pas::ast::Type &node) {}
  void visit(pas::ast::ArrayType &node) {}
  void visit(pas::ast::Subrange &node) {}
  void visit(pas::ast::RecordType &node) {}
  void visit(pas::ast::SetType &node) {}
  void visit(pas::ast::PointerType &node) {}
  void visit(pas::ast::FieldList &node) {}
  void visit(pas::ast::Assignment &assignment) {
    print_indent();
    stream_ << "Assignment" << '\n';
    depth_ += 1;
    visit(assignment.designator_);
    visit(assignment.expr_);
    depth_ -= 1;
  }
  void visit(pas::ast::ProcCall &proc_call) {
    print_indent();
    stream_ << "ProcCall name=" << proc_call.proc_ident_ << '\n';
    depth_ += 1;
    for (pas::ast::Expr &expr : proc_call.params_) {
      visit(expr);
    }
    depth_ -= 1;
  }
  void visit(pas::ast::IfStmt &if_stmt) {
    print_indent();
    stream_ << "IfStmt" << '\n';
    depth_ += 1;
    visit(if_stmt.cond_expr_);
    visit_stmt(*this, if_stmt.then_stmt_);
    if (if_stmt.else_stmt_.has_value()) {
      visit_stmt(*this, if_stmt.else_stmt_.value());
    }
    depth_ -= 1;
  }
  void visit(pas::ast::CaseStmt &node) {}
  void visit(pas::ast::Case &node) {}
  void visit(pas::ast::WhileStmt &while_stmt) {
    print_indent();
    stream_ << "WhileStmt" << '\n';
    depth_ += 1;
    visit(while_stmt.cond_expr_);
    visit_stmt(*this, while_stmt.inner_stmt_);
    depth_ -= 1;
  }
  void visit(pas::ast::RepeatStmt &node) {}
  void visit(pas::ast::ForStmt &for_stmt) {
    print_indent();
    stream_ << "ForStmt ident=" << for_stmt.ident_ << " ";
    switch (for_stmt.dir_) {
    case pas::ast::WhichWay::To: {
      stream_ << "dir=To";
      break;
    }
    case pas::ast::WhichWay::DownTo: {
      stream_ << "dir=DownTo";
      break;
    }
    default:
      assert(false);
      __builtin_unreachable();
    }
    stream_ << '\n';

    depth_ += 1;
    visit(for_stmt.start_val_expr_);
    visit(for_stmt.finish_val_expr_);
    visit_stmt(*this, for_stmt.inner_stmt_);
    depth_ -= 1;
  }
  void visit(pas::ast::Designator &designator) {
    print_indent();
    stream_ << "Designator ident=" << designator.ident_ << '\n';
    depth_ += 1;
    for (pas::ast::DesignatorItem &item : designator.items_) {
      visit(item);
    }
    depth_ -= 1;
  }
  void visit(pas::ast::DesignatorItem &designator_item) {
    switch (designator_item.index()) {
    case get_idx(pas::ast::DesignatorItemKind::ArrayAccess): {
      visit(std::get<pas::ast::DesignatorArrayAccess>(designator_item));
      break;
    }
    case get_idx(pas::ast::DesignatorItemKind::FieldAccess): {
      visit(std::get<pas::ast::DesignatorFieldAccess>(designator_item));
      break;
    }
    case get_idx(pas::ast::DesignatorItemKind::PointerAccess): {
      visit(std::get<pas::ast::DesignatorPointerAccess>(designator_item));
      break;
    }
    }
  }
  void visit(pas::ast::DesignatorArrayAccess &array_access) {
    print_indent();
    stream_ << "DesignatorArrayAccess" << '\n';
    depth_ += 1;
    for (std::unique_ptr<pas::ast::Expr> &expr_ptr : array_access.expr_list_) {
      visit(*expr_ptr);
    }
    depth_ -= 1;
  }
  void visit(pas::ast::DesignatorFieldAccess &field_access) {}
  void visit(pas::ast::DesignatorPointerAccess &pointer_access) {}
  void visit(pas::ast::MemoryStmt &node) {}
  void visit(pas::ast::EmptyStmt &empty_stmt) {}
  void visit(pas::ast::Expr &expr) {
    if (!expr.op_.has_value()) {
      visit(expr.start_expr_);
    } else {
      print_indent();
      stream_ << "Expr op=";
      switch (expr.op_.value().rel) {
      case pas::ast::RelOp::Equal: {
        stream_ << "equal";
        break;
      }
      case pas::ast::RelOp::Greater: {
        stream_ << "greater";
        break;
      }
      case pas::ast::RelOp::GreaterEqual: {
        stream_ << "greater_equal";
        break;
      }
      case pas::ast::RelOp::In: {
        stream_ << "in";
        break;
      }
      case pas::ast::RelOp::Less: {
        stream_ << "less";
        break;
      }
      case pas::ast::RelOp::LessEqual: {
        stream_ << "less_equal";
        break;
      }
      case pas::ast::RelOp::NotEqual: {
        stream_ << "not_equal";
        break;
      }
      default:
        assert(false);
        __builtin_unreachable();
      }

      stream_ << '\n';
      depth_ += 1;
      visit(expr.start_expr_);
      visit(expr.op_.value().expr);
      depth_ -= 1;
    }
  }
  void visit(pas::ast::SimpleExpr &simple_expr) {
    if (simple_expr.ops_.empty() && !simple_expr.unary_op_.has_value()) {
      visit(simple_expr.start_term_);
    } else {
      print_indent();
      stream_ << "SimpleExpr";
      if (simple_expr.unary_op_.has_value()) {
        stream_ << " ";
        switch (simple_expr.unary_op_.value()) {
        case pas::ast::UnaryOp::Plus: {
          stream_ << "unary_op=plus";
          break;
        }
        case pas::ast::UnaryOp::Minus: {
          stream_ << "unary_op=minus";
          break;
        }
        default:
          assert(false);
          __builtin_unreachable();
        }
      }
      stream_ << '\n';

      depth_ += 1;
      visit(simple_expr.start_term_);
      for (pas::ast::SimpleExpr::Op &op : simple_expr.ops_) {
        switch (op.op) {
        case pas::ast::AddOp::Minus: {
          print_indent();
          stream_ << "Minus" << '\n';
          break;
        }
        case pas::ast::AddOp::Plus: {
          print_indent();
          stream_ << "Plus" << '\n';
          break;
        }
        case pas::ast::AddOp::Or: {
          print_indent();
          stream_ << "Or" << '\n';
          break;
        }
        default:
          assert(false);
          __builtin_unreachable();
        }

        visit(op.term);
      }
      depth_ -= 1;
    }
  }
  void visit(pas::ast::Term &term) {
    if (term.ops_.empty()) {
      visit(term.start_factor_);
    } else {
      print_indent();
      stream_ << "Term" << '\n';
      depth_ += 1;
      visit(term.start_factor_);
      for (pas::ast::Term::Op &op : term.ops_) {
        switch (op.op) {
        case pas::ast::MultOp::And: {
          print_indent();
          stream_ << "And" << '\n';
          break;
        }
        case pas::ast::MultOp::IntDiv: {
          print_indent();
          stream_ << "IntDiv" << '\n';
          break;
        }
        case pas::ast::MultOp::Modulo: {
          print_indent();
          stream_ << "Modulo" << '\n';
          break;
        }
        case pas::ast::MultOp::Multiply: {
          print_indent();
          stream_ << "Multiply" << '\n';
          break;
        }
        case pas::ast::MultOp::RealDiv: {
          print_indent();
          stream_ << "RealDiv" << '\n';
          break;
        }
        default:
          assert(false);
          __builtin_unreachable();
        }
        visit(op.factor);
        depth_ -= 1;
      }
    }
  }
  void visit(pas::ast::Factor &factor) {
      switch (factor.index()) {
      case get_idx(pas::ast::FactorKind::Bool): {
          print_indent();
          bool value = std::get<bool>(factor);
          stream_ << "Factor Bool" << (value ? "true" : "false") << '\n';
          break;
      }
      case get_idx(pas::ast::FactorKind::Designator): {
          visit(std::get<pas::ast::Designator>(factor));
          break;
      }
      case get_idx(pas::ast::FactorKind::Expr): {
          visit(*std::get<pas::ast::ExprUP>(factor));
          break;
      }
      case get_idx(pas::ast::FactorKind::Negation): {
          print_indent();
          stream_ << "Factor Not" << '\n';
          visit(std::get<pas::ast::NegationUP>(factor)->factor_);
          break;
      }
      case get_idx(pas::ast::FactorKind::Nil): {
          print_indent();
          stream_ << "Factor Nil" << '\n';
          break;
      }
      case get_idx(pas::ast::FactorKind::Number): {
          print_indent();
          stream_ << "Factor number=" << std::get<int>(factor) << '\n';
          break;
      }
      case get_idx(pas::ast::FactorKind::String): {
          print_indent();
          stream_ << "Factor string=\"" << std::get<std::string>(factor) << "\"" << '\n';
          break;
      }
      case get_idx(pas::ast::FactorKind::FuncCall): {
          visit(*std::get<pas::ast::FuncCallUP>(factor));
          break;
      }
      default: assert(false); __builtin_unreachable();
      }
  }
  void visit(pas::ast::FuncCall &func_call) {
      print_indent();
      stream_ << "FuncCall ident=" << func_call.func_ident_ << '\n';
      depth_ += 1;
      for (pas::ast::Expr& expr: func_call.params_) {
          visit(expr);
      }
      depth_ -= 1;
  }
  void visit(pas::ast::Element &node) {}
  void visit(pas::ast::SubprogDecl &node) {}
  void visit(pas::ast::ProcDecl &node) {}
  void visit(pas::ast::FuncDecl &node) {}
  void visit(pas::ast::ProcHeading &node) {}
  void visit(pas::ast::FormalParam &node) {}
  void visit(pas::ast::StmtSeq &stmt_seq) {
    print_indent();
    stream_ << "StmtSeq" << '\n';
    depth_ += 1;
    for (pas::ast::Stmt &stmt : stmt_seq.stmts_) {
      visit_stmt(*this, stmt);
    }
    depth_ -= 1;
  }

public:
  void visit(pas::ast::CompilationUnit &cu) { visit(cu.pm_); }

private:
  std::ostream &stream_;
  size_t depth_ = 0;
};

// https://stackoverflow.com/a/25066044
//   Won't work if I have any other functions
//   with name "visit" in child class.
//   Ok, let's change tactics.
// class NotImplementedVisitor {
// public:
// #define FOR_EACH_STMT(stmt_type, stmt_kind)                 \
//    void visit(stmt_type& stmt) {                           \
//        throw NotImplementedException("not implemented");   \
//    }
// #include <enum_stmt.hpp>
// #undef FOR_EACH_STMT
//};

class Interpreter /* : public NotImplementedVisitor */ {
public:
  Interpreter() {
    // Add unique original names for basic types.
    ident_to_item_["Integer"] =
        std::make_shared<Type>(std::in_place_index<get_idx(TypeKind::Integer)>);
    ident_to_item_["Char"] =
        std::make_shared<Type>(std::in_place_index<get_idx(TypeKind::Char)>);
    ident_to_item_["String"] =
        std::make_shared<Type>(std::in_place_index<get_idx(TypeKind::String)>);
  }

  void interpret(pas::ast::CompilationUnit &cu) { interpret(cu.pm_); }

private:
  MAKE_VISIT_STMT_FRIEND();

  void interpret(pas::ast::ProgramModule &pm) { interpret(pm.block_); }

  void interpret(pas::ast::Block &block) {
    // Decl field should always be there, it can just have
    //   no actual decls inside.
    assert(block.decls_.get() != nullptr);
    process_decls(*block.decls_);
    for (pas::ast::Stmt &stmt : block.stmt_seq_) {
      visit_stmt(*this, stmt);
    }
  }

  void process_decls(pas::ast::Declarations &decls) {
    if (!decls.subprog_decls_.empty()) {
      throw NotImplementedException("function decls are not implemented yet");
    }
    if (!decls.const_defs_.empty()) {
      throw NotImplementedException("const defs are not implemented yet");
    }
    for (auto &type_def : decls.type_defs_) {
      process_type_def(type_def);
    }
    for (auto &var_decl : decls.var_decls_) {
      process_var_decl(var_decl);
    }
  }

  // Не очень хорошая модель памяти для паскаля, со счетчиком ссылок и т.п.
  //   Она лучше подойдет для джавы какой-то. А в паскале низкоуровневый
  //   доступ, надо эмулировать память, поддерживать движение указателей
  //   по памяти и т.п. Паскаль лучше сразу компилировать, а не
  //   интерпретировать, чтобы не эмулировать оперативную память.
  // Зато наш код в каком-то виде напоминает код typechecker-а для паскаля.
  //   Так ведь можно было бы и реализовать typechecker, ведь все типы
  //   известны. Заодно проверить, что под идентификаторами скрываются
  //   нужные объекты, когда тип, когда значение.

  struct PointerType;
  enum class TypeKind : size_t {
    // Base types
    Integer = 0,
    Char = 1,
    String = 2,

    // Pointer to an existing type
    //        PointerType = 2
  };

  // Always unveiled, synonims are expanded, when type is added to the
  // identifier mapping.
  using Type = std::variant<std::monostate, std::monostate,
                            std::monostate>; //, std::shared_ptr<PointerType>>;
                                             //    struct PointerType {
  //        // Types must be referenced as shared_ptrs:
  //        //   the objects in unordered_map (and plain map, almost any
  //        container)
  //        //   can move around memory, we have to account for that.
  //        // For an unordered map, if it is a hashtable, it happens, then
  //        //   there's too much chance of collision, we have to expand
  //        //   storage so that items are evenly spread.
  //        std::shared_ptr<Type> ref_type;
  //    };

  //    struct ValuePointer;
  enum class ValueKind : size_t {
    // Base types
    Integer = 0,
    Char = 1,
    String = 2,

    //          // Pointer to another value.
    //          Pointer = 3
  };
  using Value =
      std::variant<int, char, std::string>; //, std::shared_ptr<ValuePointer>>;
  //    struct ValuePointer {
  //        std::vector<std::shared_ptr<Value>> value;
  //    };

private:
  Value eval(pas::ast::Factor &factor) {
    switch (factor.index()) {
    case get_idx(pas::ast::FactorKind::Bool): {
      // No dedicated bool type for now for simplicity
      //   (I don't have much time, too many things to do).
      return Value(static_cast<int>(std::get<bool>(factor)));
    }
    case get_idx(pas::ast::FactorKind::Number): {
      return Value(std::get<int>(factor));
    }
    case get_idx(pas::ast::FactorKind::String): {
      return Value(std::get<std::string>(factor));
    }
    case get_idx(pas::ast::FactorKind::Nil): {
      throw NotImplementedException("Nil is not supported yet");
    }
    case get_idx(pas::ast::FactorKind::FuncCall): {
      return eval(*std::get<pas::ast::FuncCallUP>(factor));
      break;
    }

    case get_idx(pas::ast::FactorKind::Negation): {
      Value inner_value = eval(std::get<pas::ast::NegationUP>(factor)->factor_);
      if (inner_value.index() != 0) {
        throw SemanticProblemException(
            "Negation is only applicable to integer type");
      }
      return ~std::get<int>(inner_value);
    }
    case get_idx(pas::ast::FactorKind::Expr): {
      Value inner_value = eval(*std::get<pas::ast::ExprUP>(factor));
      return inner_value;
    }
    case get_idx(pas::ast::FactorKind::Designator): {
      auto &designator = std::get<pas::ast::Designator>(factor);
      // TODO: check if item with identifier exists in the first place!!
      // IMPORTANT!
      std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
          ident_to_item_[designator.ident_];
      if (item.index() != 1) {
        throw SemanticProblemException(
            "designator must reference a value, not a type: " +
            designator.ident_);
      }
      Value base_value = *std::get<std::shared_ptr<Value>>(item);

      for (pas::ast::DesignatorItem &item : designator.items_) {
        switch (item.index()) {
        case get_idx(pas::ast::DesignatorItemKind::FieldAccess): {
          throw NotImplementedException("field access is not implemented");
        }
        case get_idx(pas::ast::DesignatorItemKind::PointerAccess): {
          throw NotImplementedException("pointer access is not implemented");
        }
        case get_idx(pas::ast::DesignatorItemKind::ArrayAccess): {
          //          if (base_value.index() != get_idx(ValueKind::Pointer)) {
          //            throw NotImplementedException(
          //                "value must be a pointer for array access");
          //          }
          if (base_value.index() != get_idx(ValueKind::String)) {
            throw NotImplementedException(
                "value must be a string for array access");
          }
          pas::ast::DesignatorArrayAccess &array_access =
              std::get<pas::ast::DesignatorArrayAccess>(item);
          if (array_access.expr_list_.size() != 1) {
            throw NotImplementedException(
                "array access for more than one index is not supported");
          }
          Value value_index = eval(*array_access.expr_list_[0]);
          if (value_index.index() != get_idx(ValueKind::Integer)) {
            throw SemanticProblemException(
                "can only do indexing with integer type");
          }
          int index = std::get<int>(value_index);
          auto &value = std::get<std::string>(base_value);
          if (index < 0 || index >= value.size()) {
            // Won't be reported if it's a compiler, not an interpreter.
            //   Only a thing like valgrind or memory sanitizer.
            throw RuntimeProblemException("index is out of bounds: " +
                                          std::to_string(index));
          }
          base_value = Value(std::in_place_type<char>, value[index]);
          break;
        }
        }
      }
      return base_value;
    }
    default:
      assert(false);
      __builtin_unreachable();
    }
  }

  Value eval(pas::ast::Term &term) {
    Value value = eval(term.start_factor_);
    for (pas::ast::Term::Op &op : term.ops_) {
      if (value.index() != 0) {
        throw SemanticProblemException("can only do math with integer type");
      }
      Value rhs_value = eval(op.factor);
      if (rhs_value.index() != 0) {
        throw SemanticProblemException("can only do math with integer type");
      }
      switch (op.op) {
      case pas::ast::MultOp::And: {
        value = std::get<int>(value) & std::get<int>(rhs_value);
        break;
      }
      case pas::ast::MultOp::IntDiv: {
        value = std::get<int>(value) / std::get<int>(rhs_value);
        break;
      }
      case pas::ast::MultOp::Modulo: {
        value = std::get<int>(value) % std::get<int>(rhs_value);
        break;
      }
      case pas::ast::MultOp::Multiply: {
        value = std::get<int>(value) * std::get<int>(rhs_value);
        break;
      }
      case pas::ast::MultOp::RealDiv: {
        throw NotImplementedException("real numbers are not supported");
      }
      default:
        assert(false);
        __builtin_unreachable();
      }
    }
    return value;
  }

  Value eval(pas::ast::SimpleExpr &simple_expr) {
    // NOTE: unary op is ignored for now.
    Value value = eval(simple_expr.start_term_);
    for (pas::ast::SimpleExpr::Op &op : simple_expr.ops_) {
      if (value.index() != 0) {
        throw SemanticProblemException("can only do math with integer type");
      }
      Value rhs_value = eval(op.term);
      if (rhs_value.index() != 0) {
        throw SemanticProblemException("can only do math with integer type");
      }
      switch (op.op) {
      case pas::ast::AddOp::Plus: {
        value = std::get<int>(value) + std::get<int>(rhs_value);
        break;
      }
      case pas::ast::AddOp::Minus: {
        value = std::get<int>(value) - std::get<int>(rhs_value);
        break;
      }
      case pas::ast::AddOp::Or: {
        value = std::get<int>(value) | std::get<int>(rhs_value);
        break;
      }
      default:
        assert(false);
        __builtin_unreachable();
      }
    }
    return value;
  }

  Value eval(pas::ast::Expr &expr) {
    Value value = eval(expr.start_expr_);
    if (expr.op_.has_value()) {
      pas::ast::Expr::Op &op = expr.op_.value();
      switch (op.rel) {
      case pas::ast::RelOp::Equal: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value == rhs_value);
      }
      case pas::ast::RelOp::GreaterEqual: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value >= rhs_value);
      }
      case pas::ast::RelOp::Greater: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value > rhs_value);
      }
      case pas::ast::RelOp::LessEqual: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value <= rhs_value);
      }
      case pas::ast::RelOp::Less: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value < rhs_value);
      }
      case pas::ast::RelOp::NotEqual: {
        Value rhs_value = eval(op.expr);
        return Value(std::in_place_type<int>, value != rhs_value);
      }
      case pas::ast::RelOp::In: {
        throw NotImplementedException("relation \"in\" is not supported");
      }
      default:
        assert(false);
        __builtin_unreachable();
      }
    }
    return value;
  }

  void visit(pas::ast::MemoryStmt &memory_stmt) {}
  void visit(pas::ast::RepeatStmt &repeat_stmt) {}
  void visit(pas::ast::CaseStmt &case_stmt) {}

  void visit(pas::ast::StmtSeq &stmt_seq) {
    for (pas::ast::Stmt &stmt : stmt_seq.stmts_) {
      visit_stmt(*this, stmt);
    }
  }

  void visit(pas::ast::IfStmt &if_stmt) {
    Value value = eval(if_stmt.cond_expr_);
    if (value.index() != get_idx(ValueKind::Integer)) {
      throw SemanticProblemException("condition must evaluate to Integer");
    }
    if (std::get<int>(value) != 0) {
      visit_stmt(*this, if_stmt.then_stmt_);
    } else if (if_stmt.else_stmt_.has_value()) {
      visit_stmt(*this, if_stmt.else_stmt_.value());
    }
  }

  void visit(pas::ast::EmptyStmt &empty_stmt) {}

  void visit(pas::ast::ForStmt &for_stmt) {
    Value start_value = eval(for_stmt.start_val_expr_);
    Value end_value = eval(for_stmt.finish_val_expr_);

    if (start_value.index() != get_idx(ValueKind::Integer) ||
        end_value.index() != get_idx(ValueKind::Integer)) {
      throw SemanticProblemException(
          "expressions in for statement must evaluate to int");
    }

    int start_index = std::get<int>(start_value);
    int end_index = std::get<int>(end_value);

    if (ident_to_item_.contains(for_stmt.ident_)) {
      throw SemanticProblemException("identifier is already in use: " +
                                     for_stmt.ident_);
    }
    std::shared_ptr<Value> counter =
        std::make_shared<Value>(std::in_place_type<int>, start_index);
    ident_to_item_[for_stmt.ident_] = counter;

    switch (for_stmt.dir_) {
    case pas::ast::WhichWay::To: {
      for (int i = start_index; i <= end_index; ++i) {
        visit_stmt(*this, for_stmt.inner_stmt_);
        std::get<int>(*counter) += 1;
      }
      break;
    }
    case pas::ast::WhichWay::DownTo: {
      for (int i = start_index; i >= end_index; --i) {
        visit_stmt(*this, for_stmt.inner_stmt_);
        std::get<int>(*counter) -= 1;
      }
      break;
    }
    }

    ident_to_item_.erase(for_stmt.ident_);
  }

  void visit(pas::ast::WhileStmt &while_stmt) {
    Value value = eval(while_stmt.cond_expr_);
    if (value.index() != get_idx(ValueKind::Integer)) {
      throw SemanticProblemException(
          "condition expression in while statement must evaluate to int");
    }

    if (std::get<int>(value) == 0) {
      return;
    }

    do {
      visit_stmt(*this, while_stmt.inner_stmt_);
    } while (std::get<int>(eval(while_stmt.cond_expr_)) != 0);
  }

  void visit(pas::ast::Assignment &assignment) {
    Value new_value = eval(assignment.expr_);
    pas::ast::Designator &designator = assignment.designator_;

    if (!ident_to_item_.contains(designator.ident_)) {
      throw SemanticProblemException(
          "assignment references an undeclared identifier: " +
          designator.ident_);
    }

    std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
        ident_to_item_[designator.ident_];
    if (item.index() != 1) {
      throw SemanticProblemException(
          "assignment must reference a value, not a type: " +
          designator.ident_);
    }
    auto value = std::get<std::shared_ptr<Value>>(item);

    if (!designator.items_.empty()) {

      if (value->index() != get_idx(ValueKind::String)) {
        throw SemanticProblemException(
            "pointer and array access are only allowed for strings");
      }

      if (designator.items_.size() >= 2) {
        throw SemanticProblemException(
            "a string may have only one array access in assignment");
      }

      if (designator.items_[0].index() !=
          get_idx(pas::ast::DesignatorItemKind::ArrayAccess)) {
        throw SemanticProblemException(
            "only direct and array accesses are supported in assignment");
      }

      auto &array_access =
          std::get<pas::ast::DesignatorArrayAccess>(designator.items_[0]);

      if (array_access.expr_list_.size() >= 2) {
        throw NotImplementedException(
            "array access for more than one index is not supported");
      }

      Value value_index = eval(*array_access.expr_list_[0]);
      if (value_index.index() != get_idx(ValueKind::Integer)) {
        throw SemanticProblemException(
            "can only do indexing with integer type");
      }

      if (new_value.index() != get_idx(ValueKind::Char)) {
        throw SemanticProblemException("string item assignment can only accept "
                                       "a Char on the right hand size");
      }

      std::get<std::string>(*value)[std::get<int>(value_index)] =
          std::get<char>(new_value);
      return;
    }

    if (new_value.index() != value->index()) {
      throw SemanticProblemException(
          "incompatible types, must be of the same type for assignment");
    }

    *value = new_value; // Copy assign a new value.
  }

  Value eval_read_char(pas::ast::FuncCall &func_call) {
    if (!func_call.params_.empty()) {
      throw SemanticProblemException(
          "function read_char doesn't accept parameters");
    }
    char chr = 0;
    std::cin >> chr;
    return Value(std::in_place_type<char>, chr);
  }

  Value eval_read_str(pas::ast::FuncCall &func_call) {
    if (!func_call.params_.empty()) {
      throw SemanticProblemException(
          "function read_str doesn't accept parameters");
    }
    std::string str;
    std::cin >> str;
    return Value(std::in_place_type<std::string>, str);
  }

  Value eval_read_int(pas::ast::FuncCall &func_call) {
    if (!func_call.params_.empty()) {
      throw SemanticProblemException(
          "function read_int doesn't accept parameters");
    }
    int value = 0;
    std::cin >> value;
    return Value(std::in_place_type<int>, value);
  }

  Value eval_strlen(pas::ast::FuncCall &func_call) {
    if (func_call.params_.size() != 1) {
      throw SemanticProblemException(
          "function strlen accepts only one parameter of type String");
    }

    Value arg = eval(func_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException(
          "function strlen parameter must be of type String");
    }

    auto &str = std::get<std::string>(arg);

    if (std::numeric_limits<int>::max() < str.size()) {
      throw RuntimeProblemException(
          "string length is too big for Integer type");
    }
    // CAREFUL: May be longer than MAX_INT (std::numeric_limits<int>::max())!
    return Value(std::in_place_type<int>, static_cast<int>(str.size()));
  }

  Value eval_ord(pas::ast::FuncCall &func_call) {
    if (func_call.params_.size() != 1) {
      throw SemanticProblemException("function ord accepts only one parameter "
                                     "of type Char or String (of length 1)");
    }

    Value arg = eval(func_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::Char) &&
        arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException("function ord parameter must be of type "
                                     "Char or a String of length 1");
    }

    char chr = '\0';

    if (arg.index() == get_idx(ValueKind::Char)) {
      chr = std::get<char>(arg);
    } else if (arg.index() == get_idx(ValueKind::String)) {
      auto &str = std::get<std::string>(arg);
      if (str.size() != 1) {
        throw RuntimeProblemException(
            "Too short or too long string for ord, should be of length 1");
      }
      chr = str[0];
    }

    return Value(std::in_place_type<int>, static_cast<int>(chr));
  }

  Value eval_chr(pas::ast::FuncCall &func_call) {
    if (func_call.params_.size() != 1) {
      throw SemanticProblemException(
          "function chr accepts only one parameter of type Int");
    }

    Value arg = eval(func_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::Integer)) {
      throw SemanticProblemException(
          "function read_int parameter must be of type Char");
    }

    auto chr_code = std::get<int>(arg);

    if (chr_code < std::numeric_limits<char>::min() ||
        chr_code > std::numeric_limits<char>::max()) {
      throw RuntimeProblemException("character code out of bounds");
    }

    return Value(std::in_place_type<char>, static_cast<char>(chr_code));
  }

  Value eval(pas::ast::FuncCall &func_call) {
    const std::string &func_name = func_call.func_ident_;

    if (func_name == "read_char") {
      return eval_read_char(func_call);
    } else if (func_name == "read_str") {
      return eval_read_str(func_call);
    } else if (func_name == "read_int") {
      return eval_read_int(func_call);
    } else if (func_name == "strlen") {
      return eval_strlen(func_call);
    } else if (func_name == "ord") {
      return eval_ord(func_call);
    } else if (func_name == "chr") {
      return eval_chr(func_call);
    } else {
      throw NotImplementedException(
          "function calls are not supported yet, except read_char, read_str, "
          "read_int, strlen, ord, chr");
    }
  }

  void visit_write_char(pas::ast::ProcCall &proc_call) {
    if (proc_call.params_.size() != 1) {
      throw SemanticProblemException(
          "procedure write_char accepts only one parameter of type Char");
    }

    Value arg = eval(proc_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::Char)) {
      throw SemanticProblemException(
          "procedure write_char parameter must be of type Char");
    }

    std::cout << std::get<char>(arg);
  }

  void visit_write_str(pas::ast::ProcCall &proc_call) {
    if (proc_call.params_.size() != 1) {
      throw SemanticProblemException(
          "procedure write_str accepts only one parameter of type String");
    }

    Value arg = eval(proc_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException(
          "procedure write_str parameter must be of type String");
    }

    std::cout << std::get<std::string>(arg);
  }

  void visit_write_int(pas::ast::ProcCall &proc_call) {
    if (proc_call.params_.size() != 1) {
      throw SemanticProblemException(
          "procedure write_int accepts only one parameter of type Integer");
    }

    Value arg = eval(proc_call.params_[0]);

    if (arg.index() != get_idx(ValueKind::Integer)) {
      throw SemanticProblemException(
          "procedure write_int parameter must be of type Integer");
    }

    std::cout << std::get<int>(arg);
  }

  Value &eval_ref(pas::ast::Expr &expr) {
    if (expr.op_.has_value()) {
      throw SemanticProblemException("expected identifier, not an operation");
    }
    pas::ast::SimpleExpr &simple_expr = expr.start_expr_;
    if (!simple_expr.ops_.empty() || simple_expr.unary_op_.has_value()) {
      throw SemanticProblemException("expected identifier, not an operation");
    }
    pas::ast::Term &term = simple_expr.start_term_;
    if (!term.ops_.empty()) {
      throw SemanticProblemException("expected identifier, not an operation");
    }
    pas::ast::Factor &factor = term.start_factor_;
    if (factor.index() != get_idx(pas::ast::FactorKind::Designator)) {
      throw SemanticProblemException("expected identifier, not an expression");
    }

    pas::ast::Designator &designator = std::get<pas::ast::Designator>(factor);
    if (!designator.items_.empty()) {
      // Array access is not supported for now in value reference evaluation.
      throw SemanticProblemException(
          "unexpected array access, expected an identifier");
    }

    std::string &ident = designator.ident_;
    if (!ident_to_item_.contains(ident)) {
      throw SemanticProblemException("reference to undeclared " + ident);
    }
    std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
        ident_to_item_[ident];
    if (item.index() != 1) {
      throw SemanticProblemException(
          "identifier must reference a value, not a type");
    }
    return *std::get<std::shared_ptr<Value>>(item);
  }

  void visit_append(pas::ast::ProcCall &proc_call) {
    if (proc_call.params_.size() != 2) {
      throw SemanticProblemException(
          "procedure append accepts only two parameters: String, Ch`ar");
    }

    Value &dst_arg = eval_ref(proc_call.params_[0]);
    Value src_arg = eval(proc_call.params_[1]);

    if (dst_arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException(
          "procedure append first parameter must be of type String");
    }
    if (src_arg.index() != get_idx(ValueKind::Char) &&
        src_arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException(
          "procedure append second parameter must be of type Char or String");
    }

    if (src_arg.index() == get_idx(ValueKind::Char)) {
      std::get<std::string>(dst_arg).push_back(std::get<char>(src_arg));
    } else if (src_arg.index() == get_idx(ValueKind::String)) {
      auto &dst = std::get<std::string>(dst_arg);
      auto &src = std::get<std::string>(src_arg);
      dst.insert(dst.end(), src.begin(), src.end());
    }
  }

  void visit_drop(pas::ast::ProcCall &proc_call) {
    if (proc_call.params_.size() != 1) {
      throw SemanticProblemException(
          "procedure drop accepts only one parameter: String");
    }

    Value &str_arg = eval_ref(proc_call.params_[0]);

    if (str_arg.index() != get_idx(ValueKind::String)) {
      throw SemanticProblemException(
          "procedure drop parameter must be of type String");
    }

    std::get<std::string>(str_arg).pop_back();
  }

  void visit(pas::ast::ProcCall &proc_call) {
    const std::string &proc_name = proc_call.proc_ident_;

    //      std::cerr << "ProcCall proc_name = " << proc_name << std::endl;

    if (proc_name == "write_char") {
      visit_write_char(proc_call);
    } else if (proc_name == "write_str") {
      visit_write_str(proc_call);
    } else if (proc_name == "write_int") {
      visit_write_int(proc_call);
    } else if (proc_name == "append") {
      visit_append(proc_call);
    } else if (proc_name == "drop") {
      visit_drop(proc_call);
    } else {
      throw NotImplementedException(
          "procedure calls are not supported yet, except write_char, "
          "write_str, write_int, append, drop");
    }
  }

  // ProcCall для scanf, printf.
  // Запустить пример со строками.
  // FuncCall разрешить только для scanf, printf. У scanf всегда два аргумента,
  // у printf -- один или два.

  void process_type_def(const pas::ast::TypeDef &type_def) {
    size_t type_index = type_def.type_.index();
    switch (type_index) {
    case get_idx(pas::ast::TypeKind::Array):
    case get_idx(pas::ast::TypeKind::Record):
    case get_idx(pas::ast::TypeKind::Set): {
      throw NotImplementedException(
          "only pointer types, basic types (Integer, Char) and their synonims "
          "are supported for now");
    }
      //    case get_idx(pas::ast::TypeKind::Pointer): {
      //      if (ident_to_item_.contains(type_def.ident_)) {
      //        throw SemanticProblemException("identifier is already in use: "
      //        +
      //                                       type_def.ident_);
      //      }
      //      const auto &ptr_type_up =
      //          std::get<pas::ast::PointerTypeUP>(type_def.type_);
      //      const pas::ast::PointerType &ptr_type = *ptr_type_up;
      //      if (!ident_to_item_.contains(ptr_type.ref_type_name_)) {
      //        throw SemanticProblemException(
      //            "pointer type references an undeclared identifier: " +
      //            ptr_type.ref_type_name_);
      //      }
      //      std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
      //          ident_to_item_[ptr_type.ref_type_name_];
      //      if (item.index() != 0) {
      //        throw SemanticProblemException(
      //            "pointer type must reference a type, not a value: " +
      //            ptr_type.ref_type_name_);
      //      }
      //      auto type_item = std::get<std::shared_ptr<Type>>(item);
      //      auto pointer_type =
      //      std::make_shared<PointerType>(PointerType{type_item}); auto
      //      new_type_item = std::make_shared<Type>(pointer_type); auto
      //      new_item =
      //          std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>>(
      //              std::move(new_type_item));
      //      ident_to_item_[type_def.ident_] = new_item;
      //      break;
      //    }
    case get_idx(pas::ast::TypeKind::Pointer): {
      throw NotImplementedException("pointer types are not supported now");
    }
    case get_idx(pas::ast::TypeKind::Named): {
      if (ident_to_item_.contains(type_def.ident_)) {
        throw SemanticProblemException("identifier is already in use: " +
                                       type_def.ident_);
      }
      const auto &named_type_up =
          std::get<pas::ast::NamedTypeUP>(type_def.type_);
      const pas::ast::NamedType &named_type = *named_type_up;
      if (!ident_to_item_.contains(named_type.type_name_)) {
        throw SemanticProblemException(
            "named type references an undeclared identifier: " +
            named_type.type_name_);
      }
      std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
          ident_to_item_[named_type.type_name_];
      if (item.index() != 0) {
        throw SemanticProblemException(
            "named type must reference a type, not a value: " +
            named_type.type_name_);
      }
      auto type_item = std::get<std::shared_ptr<Type>>(item);
      auto new_type_item = type_item;
      auto new_item =
          std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>>(
              std::move(new_type_item));
      ident_to_item_[type_def.ident_] = new_item;
      break;
    }
    default:
      assert(false);
      __builtin_unreachable();
    }
  }

  // Won't pick std::shared_ptr<Type> up, because as a declaration
  //   this function is before Type declaration (using in private section).
  //   Inside of this function, all declarations are already processed
  //   and the body is already a definition.
  std::shared_ptr<Type> make_var_decl_type(const pas::ast::Type &type) {
    size_t type_index = type.index();
    switch (type_index) {
    case get_idx(pas::ast::TypeKind::Array):
    case get_idx(pas::ast::TypeKind::Record):
    case get_idx(pas::ast::TypeKind::Set): {
      //      throw NotImplementedException(
      //          "only pointer types, basic types (Integer, Char) and their
      //          synonims " "are supported for now");
      throw NotImplementedException(
          "only basic types (Integer, Char) and strings are supported for now");
    }
    case get_idx(pas::ast::TypeKind::Pointer): {
      throw NotImplementedException("pointer types are not supported now");
    }

      //    case get_idx(pas::ast::TypeKind::Pointer): {
      //      const auto &ptr_type_up = std::get<pas::ast::PointerTypeUP>(type);
      //      const pas::ast::PointerType &ptr_type = *ptr_type_up;
      //      if (!ident_to_item_.contains(ptr_type.ref_type_name_)) {
      //        throw SemanticProblemException(
      //            "pointer type references an undeclared identifier: " +
      //            ptr_type.ref_type_name_);
      //      }
      //      std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
      //          ident_to_item_[ptr_type.ref_type_name_];
      //      if (item.index() != 0) {
      //        throw SemanticProblemException(
      //            "pointer type must reference a type, not a value: " +
      //            ptr_type.ref_type_name_);
      //      }
      //      auto type_item = std::get<std::shared_ptr<Type>>(item);
      //      auto pointer_type =
      //      std::make_shared<PointerType>(PointerType{type_item}); auto
      //      new_type_item = std::make_shared<Type>(pointer_type); return
      //      new_type_item; break;
      //    }
    case get_idx(pas::ast::TypeKind::Named): {
      const auto &named_type_up = std::get<pas::ast::NamedTypeUP>(type);
      const pas::ast::NamedType &named_type = *named_type_up;
      if (!ident_to_item_.contains(named_type.type_name_)) {
        throw SemanticProblemException(
            "named type references an undeclared identifier: " +
            named_type.type_name_);
      }
      std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>> item =
          ident_to_item_[named_type.type_name_];
      if (item.index() != 0) {
        throw SemanticProblemException(
            "named type must reference a type, not a value: " +
            named_type.type_name_);
      }
      auto type_item = std::get<std::shared_ptr<Type>>(item);
      return type_item;
      break;
    }
    default:
      assert(false);
      __builtin_unreachable();
    }
  }

  std::shared_ptr<Value> make_uninit_value_of_type(std::shared_ptr<Type> type) {
    switch (type->index()) {
    case get_idx(TypeKind::Integer): {
      return std::make_shared<Value>(
          std::in_place_index<get_idx(ValueKind::Integer)>, 0);
      break;
    }
    case get_idx(TypeKind::Char): {
      return std::make_shared<Value>(
          std::in_place_index<get_idx(ValueKind::Char)>, '\0');
      break;
    }
    case get_idx(TypeKind::String): {
      return std::make_shared<Value>(
          std::in_place_index<get_idx(ValueKind::String)>, std::string());
      break;
    }
      //    case get_idx(TypeKind::PointerType): {
      //      return std::make_shared<Value>(
      //          std::in_place_index<get_idx(ValueKind::Pointer)>,
      //          make_uninit_value_of_type(
      //              std::get<std::shared_ptr<PointerType>>(*type)->ref_type));
      //      break;
      //    }
    default:
      assert(false);
      __builtin_unreachable();
    }
  }

  void process_var_decl(const pas::ast::VarDecl &var_decl) {
    std::shared_ptr<Type> var_type = make_var_decl_type(var_decl.type_);
    for (const std::string &ident : var_decl.ident_list_) {
      if (ident_to_item_.contains(ident)) {
        throw SemanticProblemException("identifier is already in use: " +
                                       ident);
      }
      std::shared_ptr<Value> value = make_uninit_value_of_type(var_type);
      ident_to_item_[ident] = value;
    }
  }

private:
  std::unordered_map<
      std::string, std::variant<std::shared_ptr<Type>, std::shared_ptr<Value>>>
      ident_to_item_;
};

} // namespace visitor
} // namespace pas
