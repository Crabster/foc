#include "code_generator.hpp"

namespace foc {

//*********************************************
// Common:
//     Allocation on stack is from biggest to smallest address, but
//     memory blocks have their data in big endian (different direction).
//     Register rsp points to last allocated block of memory, not behind it.
//
// Functions:
//     For each function a label and corresponding body is generated.
//
//     We need to ensure that we save and change rbp on start and end,
//     allocate space for return value and push parameters on stack before
//     fun call.
//
//     After function call the return value will always be first
//     block of memory on the stack and its address will be in rax.
//
//     We need to beware of registers overwriting in recursive generate_asm
//     calls. Always use r_x for local computation and rN for specific use cases.
//
//     Return address is on [rbp], previous rbp is on [rbp + 8] and
//     return value is on [rbp + 8 + ret_val_size].
//
// Expressions:
//     After generating asm for expression, the result of it will always be
//     first block of memory on the stack with its address in rax.
//
//     For ID we need to be able to use it as value and to assign values to it.
//     So we always push the ID value on stack. But the address will point to
//     the space we allocated for the definition of ID, not the value on top
//     if the stack we have pushed.
//
//     For pointer like structures we will store their pointers also in rax,
//     so we are able to assign to them same way as with ID.
//
//     When in function scope, expression are always located on top of the stack,
//     nothing else is inbetween them.
//
// Moving:
//     Because we allocate space for each variable or return value on stack,
//     we need to be able to move memory blocks to the address of variables /
//     return values.
//
//     For this purpose we have function move_memory_block, which simply moves
//     the memory from address in rbx ending in address rsp to address in rax.
//
//


void CodeGenerator::generate_asm(const Expr& expr) {
    expr.var.visit([&](auto arg) { generate_asm(arg); });
    if (expr.minus) {
        out_file << "    pop_ rbx\n"
                 << "    neg rbx\n"
                 << "    push_ rbx" << std::endl;
    }
}

void CodeGenerator::generate_asm(const TypeExpr& type_expr) {
    std::visit([&](auto arg) { generate_asm(arg); }, type_expr.expr);
}

void CodeGenerator::generate_asm(const BinOperation& bin_op) {
    generate_asm(*bin_op.left_expr);
    generate_asm(*bin_op.right_expr);
    generate_asm(bin_op.op);
}

void CodeGenerator::generate_asm(BinOperation::Operator op) {
    if (op == BinOperation::Operator::PLUS) {
        out_file << "    add_op" << std::endl;
    } else if (op == BinOperation::Operator::MINUS) {
        out_file << "    sub_op" << std::endl;
    } else if (op == BinOperation::Operator::STAR) {
        out_file << "    mul_op" << std::endl;
    } else if (op == BinOperation::Operator::SLASH) {
        out_file << "    div_op" << std::endl;
    } else if (op == BinOperation::Operator::IS_EQUAL) {
        out_file << "    equal" << std::endl;
    } else if (op == BinOperation::Operator::NOT_EQUAL) {
        out_file << "    not_equal" << std::endl;
    } else if (op == BinOperation::Operator::AND) {
        out_file << "    and_op" << std::endl;
    } else if (op == BinOperation::Operator::OR) {
        out_file << "    or_op" << std::endl;
    } else if (op == BinOperation::Operator::LESS) {
        out_file << "    less" << std::endl;
    } else if (op == BinOperation::Operator::GREATER) {
        out_file << "    greater" << std::endl;
    } else if (op == BinOperation::Operator::LEQ) {
        out_file << "    leq" << std::endl;
    } else if (op == BinOperation::Operator::GEQ) {
        out_file << "    geq" << std::endl;
    }
}

void CodeGenerator::generate_asm(const DerefArray& deref_array) {
    int64_t array_el_size = std::get<Type::Array>(deref_array.array_expr->type->var).first.byte_size();

    generate_asm(*deref_array.array_expr);
    out_file << "    push_ rax" << std::endl;

    generate_asm(*deref_array.deref_expr);
    out_file << "    pop_ rax\n"
             << "    pop_ rbx\n"
             << "    mov rcx, " << array_el_size << "\n"
             << "    deref_array " << id_gen++ << "\n"
             << "    mov rax, rsp\n"
             << "    add rax, " << array_el_size << "\n" << std::endl;
}

void CodeGenerator::generate_asm(const DerefTuple& deref_tuple) {
    int64_t tuple_size = deref_tuple.tuple_expr->type->byte_size();
    int tuple_idx = std::get<int>(std::get<TypeExpr>(deref_tuple.deref_expr->var).expr);

    Type::Tuple tuple_type = std::get<Type::Tuple>(deref_tuple.tuple_expr->type->var);
    int64_t offset = 0;
    for (int i = 0; i < tuple_idx; ++i) {
        offset += tuple_type[i].byte_size();
    }

    int64_t tuple_el_size = tuple_type[tuple_idx].byte_size();

    generate_asm(*deref_tuple.tuple_expr);
    out_file << "    mov rcx, rax\n"
             << "    sub rax, " << offset << "\n"
             << "    mov rbx, rax\n"
             << "    sub rbx, " << tuple_el_size << "\n"
             << "    copy " << id_gen++ << "\n"
             << "    mov rsp, rcx\n"
             << "    mov rax, rsp\n"
             << "    add rax, " << tuple_el_size << "\n" << std::endl;
}

void CodeGenerator::generate_asm(const std::monostate&) {}

void CodeGenerator::generate_asm(const FunCall& fun_call) {
    if (fun_call.fun_args) {
        for (int i = fun_call.fun_args->size() - 1; i >= 0; --i) {
            out_file << "    ; fun arg number " << i << std::endl;
            generate_asm((*fun_call.fun_args)[i]);
        }
    }

    generate_asm(*fun_call.fun);
    out_file << "    pop_ rax\n"
             << "    call_ " << id_gen++ << ", rax\n" << std::endl;
}

void CodeGenerator::generate_asm(const ID& id) {
    auto fun_it = fun_ids.find(id);
    if (fun_it != fun_ids.end()) {
        out_file << "    mov rbx, " << id.name << "\n"
                 << "    push_ rbx\n" << std::endl;
        return;
    }

    for (const auto& sym_table : fun_scope.scopes_sym_tables) {
        auto it = sym_table.find(id);
        if (it != sym_table.end()) {
            int64_t local_address = it->second.local_address;
            int64_t end_address = it->second.end_address;
            out_file << "    ; id expr\n"
                     << "    mov rax, rbp\n"
                     << "    sub rax, " << local_address << "\n"
                     << "    mov rbx, rbp\n"
                     << "    sub rbx, " << end_address << "\n"
                     << "    mov rcx, rsp\n"
                     << "    sub rsp, " << end_address - local_address << "\n"
                     << "    copy " << id_gen++ << "\n"
                     << "    mov rax, rsp\n"
                     << "    add rax, " << end_address - local_address << "\n"
                     << "    mov rbx, rbp\n"
                     << "    sub rbx, " << local_address << "\n" << std::endl;
            break;
        }
    }
}

void CodeGenerator::generate_asm(int i) {
    out_file << "    push_ " << i << " ; int expr" << std::endl;
}

void CodeGenerator::generate_asm(bool b) {
    out_file << "    push_ " << b << " ; bool expr" << std::endl;
}

void CodeGenerator::generate_asm(const std::string& str) {
    for (int i = 0; i < str.size(); ++i) {
        out_file << "    mov dword [rsp - " << i * 8 << "], " << int(str[i]) << std::endl;
    }
    out_file << "    mov rax, rsp\n"
             << "    sub rsp, " << str.size() * 8 << std::endl;

}

void CodeGenerator::generate_asm(char c) {
    out_file << "    push_ " << int(c) << " ; char expr" << std::endl;
}

void CodeGenerator::generate_asm(const PtrExpr& ptr_expr) {
    if (ptr_expr.ref_expr) {
        generate_asm(*ptr_expr.ref_expr);
        out_file << "    mov rsp, rax\n"
                 << "    push_ rbx" << std::endl;
    } else if (ptr_expr.deref_expr) {
        generate_asm(*ptr_expr.deref_expr);
        int64_t expr_size = std::get<Type::Ptr>(ptr_expr.deref_expr->type->var)->byte_size();
        out_file << "    pop_ rax\n"
                 << "    mov rbx, rax\n"
                 << "    sub rbx, " << expr_size << "\n"
                 << "    mov rcx, rsp\n"
                 << "    copy " << id_gen++ << "\n"
                 << "    mov rax, rsp\n"
                 << "    add rsp, " << expr_size << std::endl;
    } else {
        out_file << "    push_ 0 ; ptr expr null\n"
                 << "    mov rax, rsp\n"
                 << "    add rax, 8" << std::endl;
    }
}

void CodeGenerator::generate_asm(const ArrayExpr& array_expr) {
    int64_t array_el_size = array_expr.exprs[0].type->byte_size();

    for (const Expr& expr : array_expr.exprs) {
        generate_asm(expr);
    }
    out_file << "    mov rax, rsp\n"
             << "    add rax, " << array_expr.exprs.size() * array_el_size << std::endl;
}

void CodeGenerator::generate_asm(const TupleExpr& tuple_expr) {
    int64_t tuple_size = 0;
    for (const Expr& expr : tuple_expr.exprs) {
        generate_asm(expr);
        tuple_size += expr.type->byte_size();
    }
    out_file << "    mov rax, rsp\n"
             << "    add rax, " << tuple_size << std::endl;
}

void CodeGenerator::generate_asm(const Print& print) {
    generate_asm(print.expr);
    out_file << "    print" << std::endl;
}

void CodeGenerator::generate_asm(const VarDecl& var_decl) {
    out_file << "    ; var decl" << std::endl;

    if (var_decl.expr) {
        generate_asm(*var_decl.expr);
    }

    if (var_decl.ids) {
        const auto& var = var_decl.type->var;
        FunScope::SymbolTable& sym_table = fun_scope.scopes_sym_tables.back();

        if (std::holds_alternative<Type::Array>(var)) {
            const Type::Array& array = std::get<Type::Array>(var);
            int64_t array_el_size = array.first.byte_size();
            int64_t array_size = array.second * array_el_size;

            if (var_decl.ids->size() == 1) {
                sym_table[(*var_decl.ids)[0]] = Symbol{
                    .local_address = fun_scope.local_rsp,
                    .end_address   = fun_scope.local_rsp + array_size,
                };
            } else {
                for (int i = 0; i < array.second; ++i) {
                    sym_table[(*var_decl.ids)[i]] = Symbol{
                        .local_address = fun_scope.local_rsp + i * array_el_size,
                        .end_address   = fun_scope.local_rsp + (i + 1) * array_el_size,
                    };
                }
            }

            fun_scope.local_rsp += array_size;

            if (!var_decl.expr) {
                out_file << "    sub rsp, " << array_size << std::endl;
            }
        } else if (std::holds_alternative<Type::Tuple>(var)) {
            const Type::Tuple& tuple = std::get<Type::Tuple>(var);
            int64_t tuple_size = var_decl.type->byte_size();

            if (var_decl.ids->size() == 1) {
                sym_table[(*var_decl.ids)[0]] = Symbol{
                    .local_address = fun_scope.local_rsp,
                    .end_address   = fun_scope.local_rsp + tuple_size,
                };
                fun_scope.local_rsp += tuple_size;
            } else {
                for (int i = 0; i < tuple.size(); ++i) {
                    sym_table[(*var_decl.ids)[i]] = Symbol{
                        .local_address = fun_scope.local_rsp,
                        .end_address   = fun_scope.local_rsp + static_cast<int64_t>(tuple[i].byte_size()),
                    };
                    fun_scope.local_rsp += tuple[i].byte_size();
                }
            }

            if (!var_decl.expr) {
                out_file << "    sub rsp, " << tuple_size << std::endl;
            }
        } else {
            sym_table[(*var_decl.ids)[0]] = Symbol{
                .local_address = fun_scope.local_rsp,
                .end_address   = fun_scope.local_rsp + 8
            };
            fun_scope.local_rsp += 8;

            if (!var_decl.expr) {
                out_file << "    sub rsp, 8" << std::endl;
            }
        }
    } else {
        out_file << "    add rsp, " << var_decl.type->byte_size() << std::endl;
    }

    out_file << std::endl;
}

void CodeGenerator::generate_asm(const Assign& assign) {
    out_file << "    ; assign\n"
             << "    mov r8, rsp" << std::endl;

    generate_asm(assign.expr);

    out_file << "    mov r9, rsp" << std::endl;

    generate_asm(assign.assign_expr);

    out_file << "    mov rsp, r9\n"
             << "    mov rax, rbx\n"
             << "    mov rbx, r8\n"
             << "    move " << id_gen++ << "\n"
             << "    ; end of assign\n" << std::endl;
}

void CodeGenerator::generate_asm(const Cond& cond) {
    int64_t if_id = id_gen++;
    for (const IfCond& if_cond : cond.if_conds) {
        generate_asm(if_cond.expr);

        int64_t loc_if_id = id_gen++;
        out_file << "    ; if       \n"
                 << "    pop_ rbx   \n"
                 << "    cmp rbx, 0 \n"
                 << "    je .end_loc_if" << loc_if_id << std::endl;

        fun_scope.scopes_sym_tables.push_back({});
        generate_asm(if_cond.body);
        fun_scope.scopes_sym_tables.pop_back();

        out_file << "    jmp .end_if" << if_id << std::endl;
        out_file << "  .end_loc_if" << loc_if_id << ":" << std::endl;
    }

    if (cond.else_body) {
        generate_asm(*cond.else_body);
    }
    out_file << "  .end_if" << if_id << ":\n" << std::endl;
}

void CodeGenerator::generate_asm(const Loop& loop) {
    int64_t id = id_gen++;
    out_file << "    ; loop \n"
             << "  .loop" << id << ":" << std::endl;

    generate_asm(loop.expr);

    out_file << "    ; if       \n"
             << "    pop_ rbx   \n"
             << "    cmp rbx, 0 \n"
             << "    je .loop_end" << id << std::endl;

    fun_scope.loop_labels_ids.push_back(id);
    generate_asm(loop.body);
    fun_scope.loop_labels_ids.pop_back();

    out_file << "    jmp .loop" << id << "\n"
             << "  .loop_end" << id << ":\n" << std::endl;
}

void CodeGenerator::generate_asm(const Flow::Control& control) {
    if (control.first == Flow::ControlTypes::RETURN && control.second) {
        generate_asm(*control.second);
        out_file << "    jmp .fun_end" << std::endl;
    } else if (control.first == Flow::ControlTypes::CONTINUE) {
        out_file << "    jmp .loop" << fun_scope.loop_labels_ids.back() << std::endl;
    } else if (control.first == Flow::ControlTypes::BREAK) {
        out_file << "    jmp .loop_end" << fun_scope.loop_labels_ids.back() << std::endl;
    }
}

void CodeGenerator::generate_asm(const Flow& flow) {
    std::visit([&](auto arg) { generate_asm(arg); }, flow.var);
}

void CodeGenerator::generate_asm(const FunBodyPart& fun_body_part) {
    if (std::holds_alternative<Expr>(fun_body_part.var)) {
        out_file << "    mov r10, rsp" << std::endl;
        generate_asm(std::get<Expr>(fun_body_part.var));
        out_file << "    mov rsp, r10" << std::endl;
    } else {
        fun_body_part.var.visit([&](auto arg) { generate_asm(arg); });
    }
}

void CodeGenerator::generate_asm(const FunBody& fun_body) {
    for (const FunBodyPart& part : fun_body.parts) {
        generate_asm(part);
    }
}

void CodeGenerator::generate_asm(const FunDecl& fun_decl) {
    auto fun_args_fold = [](int64_t acc, const FunArg& fun_arg) {
        return fun_arg.type.byte_size() + acc;
    };
    int64_t args_size = std::accumulate(fun_decl.args.begin(), fun_decl.args.end(), 0, fun_args_fold);
    int64_t return_val_size = fun_decl.ret_type.byte_size();
    int64_t offset = args_size < return_val_size ? return_val_size - args_size : 0;

    FunScope::SymbolTable sym_table;

    int64_t args_offset = 0;
    for (const FunArg& fun_arg : fun_decl.args) {
        args_offset += fun_arg.type.byte_size();
        sym_table[fun_arg.id] = Symbol{
            .local_address = -(16 + offset + args_offset),
            .end_address   = -(16 + offset + args_offset - fun_arg.type.byte_size()),
        };
    }
    fun_scope.scopes_sym_tables.push_back(sym_table);

    fun_ids.insert(fun_decl.id);
    out_file << fun_decl.id.name << ":\n"
             << "    fun_init " << offset << "\n" << std::endl;

    fun_scope.local_rsp = 0;
    generate_asm(fun_decl.body);
    fun_scope.scopes_sym_tables.pop_back();

    out_file << "  .fun_end:" << std::endl;
    if (return_val_size > 0) {
        out_file << "    ret_val " << id_gen++ << ", " <<  return_val_size  << std::endl;
    }
    out_file << "    ret_\n" << std::endl;
}


void CodeGenerator::generate_asm(const Program& program) {
    out_file << asm_macros <<
        "section .text                                              \n"
        "    global _start                                          \n"
        "                                                           \n"
        "_start:                                                    \n"
        "    sub rsp, 8                                             \n"
        "    call_ " << id_gen++ <<  ", main                        \n"
        "    pop_ rdi                                               \n"
        "    mov rax, 60                                            \n"
        "    syscall                                                \n"
        << std::endl;

    for (const FunDecl& decl : program.decls) {
        generate_asm(decl);
    }
}

}
