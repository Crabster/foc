#pragma once

#include "id_context.hpp"

namespace foc {

unsigned syntax_check(const VarDecl& decl, std::shared_ptr<IDContext> context, unsigned limit);
unsigned syntax_check(const Assign& ass, std::shared_ptr<IDContext> context, unsigned limit);
unsigned syntax_check(const IfCond& ifflow, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const Cond& cond, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const Loop& loop, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const Flow::Control& ctrl, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const Flow& flow, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const FunBody& body, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type, unsigned limit);
unsigned syntax_check(const FunDecl& fun_decl, std::shared_ptr<IDContext> par_context, unsigned limit);
unsigned syntax_check(const Program& prog, bool debug_mode, unsigned limit);

std::optional<Type> get_expr_type(const FunCall& expr, std::shared_ptr<IDContext> context);
std::optional<Type> get_expr_type(const TypeExpr& expr, std::shared_ptr<IDContext> context);
std::optional<Type> get_expr_type(const ID& expr, std::shared_ptr<IDContext> context);
std::optional<Type> get_expr_type(const Expr& expr, std::shared_ptr<IDContext> context);

bool is_lvalue(const Expr& expr);

}
