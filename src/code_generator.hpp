#include <iostream>
#include <fstream>
#include <numeric>
#include <unordered_set>

#include "syntax_tree.hpp"

namespace foc {

struct Symbol {
    int64_t local_address = 0;
    int64_t end_address = 0;
};

struct FunScope {
    using SymbolTable = std::unordered_map<ID, Symbol>;

    std::vector<SymbolTable> scopes_sym_tables;
    std::vector<int64_t> loop_labels_ids;
    int64_t local_rsp;
};

class CodeGenerator {
public:
    CodeGenerator(const std::string& file_name) : out_file(file_name) {}
    ~CodeGenerator() {
        out_file.close();
    }

    void generate_asm(const Expr& expr);
    void generate_asm(const TypeExpr& type_expr);
    void generate_asm(const BinOperation& bin_op);
    void generate_asm(const DerefArray& deref_array);
    void generate_asm(const DerefTuple& deref_tuple);
    void generate_asm(const FunCall& fun_call);
    void generate_asm(const std::monostate&);
    void generate_asm(BinOperation::Operator op);
    void generate_asm(const ID& id);
    void generate_asm(int i);
    void generate_asm(bool b);
    void generate_asm(const std::string& str);
    void generate_asm(char c);
    void generate_asm(const PtrExpr& ptr_expr);
    void generate_asm(const ArrayExpr& array_expr);
    void generate_asm(const TupleExpr& tuple_expr);
    void generate_asm(const Print& print);
    void generate_asm(const VarDecl& var_decl);
    void generate_asm(const Assign& assign);
    void generate_asm(const Cond& cond);
    void generate_asm(const Loop& loop);
    void generate_asm(const Flow::Control& control);
    void generate_asm(const Flow& flow);
    void generate_asm(const FunBodyPart& fun_body_part);
    void generate_asm(const FunBody& fun_body);
    void generate_asm(const FunDecl& fun_decl);
    void generate_asm(const Program& program);

private:
    FunScope fun_scope;
    std::unordered_set<ID> fun_ids;
    int64_t id_gen = 0;

    std::ofstream out_file;
};

const std::string asm_macros =
        "BITS 64;                                                   \n"
        "                                                           \n"
        "%macro push_ 1                                             \n"
        "    mov qword [rsp], %1                                    \n"
        "    sub rsp, 8                                             \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro pop_ 1                                              \n"
        "    add rsp, 8                                             \n"
        "    mov %1, qword [rsp]                                    \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro call_ 2                                             \n"
        "    push_ .fun_ret%1                                       \n"
        "    jmp %2                                                 \n"
        "  .fun_ret%1:                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro ret_val 2                                           \n"
        "    mov rax, rbp                                           \n"
        "    add rax, 16 + %2                                       \n"
        "    mov rbx, rsp                                           \n"
        "    add rbx, %2                                            \n"
        "                                                           \n"
        "    move %1                                                \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro ret_ 0                                              \n"
        "    mov rsp, rbp                                           \n"
        "    pop_ rbp                                               \n"
        "    pop_ rbx                                               \n"
        "    jmp rbx                                                \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro move 1                                              \n"
        "    mov rdx, rbx                                           \n"
        "  .move%1:                                                 \n"
        "    mov rcx, [rbx]                                         \n"
        "    mov [rax], rcx                                         \n"
        "    sub rax, 8                                             \n"
        "    sub rbx, 8                                             \n"
        "    cmp rsp, rbx                                           \n"
        "    jl .move%1                                             \n"
        "    mov rsp, rdx                                           \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro copy 1                                              \n"
        "  .copy%1:                                                 \n"
        "     mov rdx, [rax]                                        \n"
        "     mov [rcx], rdx                                        \n"
        "     sub rax, 8                                            \n"
        "     sub rcx, 8                                            \n"
        "     cmp rbx, rax                                          \n"
        "     jl .copy%1                                            \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro fun_init 1                                          \n"
        "    pop_ rax     ; moving return address to rax            \n"
        "    sub rsp, %1  ; make space for return value             \n"
        "    push_ rax    ; save return address on stack            \n"
        "    push_ rbp    ; save last frame address on stack        \n"
        "    mov rbp, rsp ; saving frame address to rbp             \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro print 0                                             \n"
        "    mov rsi, rsp                                           \n"
        "    add rsi, 8                                             \n"
        "    mov rax, 1                                             \n"
        "    mov rdx, 8                                             \n"
        "    syscall                                                \n"
        "    add rsp, 8                                             \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "; rax = idx, rbx = array address, rcx = el size            \n"
        "%macro deref_array 1                                       \n"
        "    mul rcx                                                \n"
        "    mov rdx, rbx                                           \n"
        "    sub rbx, rax                                           \n"
        "    mov rax, rbx                                           \n"
        "    sub rbx, rcx                                           \n"
        "    mov rcx, rdx                                           \n"
        "    copy %1                                                \n"
        "    mov rsp, rcx                                           \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro add_op 0                                            \n"
        "    pop_ rbx                                               \n"
        "    add [rsp + 8], rbx                                     \n"
        "    mov rax, [rsp + 8]                                     \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro sub_op 0                                            \n"
        "    pop_ rbx                                               \n"
        "    sub [rsp + 8], rbx                                     \n"
        "    mov rax, [rsp + 8]                                     \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro mul_op 0                                            \n"
        "    pop_ rax                                               \n"
        "    mul dword [rsp + 8]                                    \n"
        "    mov [rsp + 8], rax                                     \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro div_op 0                                            \n"
        "    pop_ rbx                                               \n"
        "    pop_ rax                                               \n"
        "    div rbx                                                \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro equal 0                                             \n"
        "    mov rcx, 0                                             \n"
        "    mov rdx, 1                                             \n"
        "    pop_ rax                                               \n"
        "    pop_ rbx                                               \n"
        "    cmp rax, rbx                                           \n"
        "    cmovne rax, rcx                                        \n"
        "    cmove rax, rdx                                         \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro not_equal 0                                         \n"
        "    equal                                                  \n"
        "    xor dword [rsp + 8], 1                                 \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro and_op 0                                            \n"
        "    pop_ rbx                                               \n"
        "    and [rsp + 8], rbx                                     \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro or_op 0                                             \n"
        "    pop_ rbx                                               \n"
        "    or [rsp + 8], rbx                                      \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro less 0                                              \n"
        "    mov rcx, 0                                             \n"
        "    mov rdx, 1                                             \n"
        "    pop_ rbx                                               \n"
        "    pop_ rax                                               \n"
        "    cmp rax, rbx                                           \n"
        "    cmovge rax, rcx                                        \n"
        "    cmovl rax, rdx                                         \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro greater 0                                           \n"
        "    mov rcx, 0                                             \n"
        "    mov rdx, 1                                             \n"
        "    pop_ rbx                                               \n"
        "    pop_ rax                                               \n"
        "    cmp rax, rbx                                           \n"
        "    cmovle rax, rcx                                        \n"
        "    cmovg rax, rdx                                         \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro leq 0                                               \n"
        "    mov rcx, 0                                             \n"
        "    mov rdx, 1                                             \n"
        "    pop_ rbx                                               \n"
        "    pop_ rax                                               \n"
        "    cmp rax, rbx                                           \n"
        "    cmovg rax, rcx                                         \n"
        "    cmovle rax, rdx                                        \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n"
        "                                                           \n"
        "%macro geq 0                                               \n"
        "    mov rcx, 0                                             \n"
        "    mov rdx, 1                                             \n"
        "    pop_ rbx                                               \n"
        "    pop_ rax                                               \n"
        "    cmp rax, rbx                                           \n"
        "    cmovl rax, rcx                                         \n"
        "    cmovge rax, rdx                                        \n"
        "    push_ rax                                              \n"
        "%endmacro                                                  \n";

}
