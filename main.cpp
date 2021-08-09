#include <iostream>
#include "FocLexer.h"
#include "src/code_visitor.hpp"
#include "src/syntax_check.hpp"

int main() {
    std::ifstream stream;
    stream.open("example.foc");

    // Maybe make option to get these from command line
    unsigned limit = 20;
    bool debug_mode = false;

    antlr4::ANTLRInputStream input(stream);
    foc::FocLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    foc::FocParser parser(&tokens);
    foc::FocParser::ProgramContext* tree = parser.program();
    foc::CodeVisitor visitor;
    foc::Program program = visitor.visitProgram(tree).as<foc::Program>();
    if (debug_mode) {
        std::cout << program.to_string() << "\n----------------------\n" << std::endl;
    }
    auto errors = foc::syntax_check(program, debug_mode, limit);
    if (errors == 0) {
        std::cout << "Compilation was succesfull." << std::endl;
    } else if (errors >= limit) {
        std::cout << "Too many errors, compilations stopped" << std::endl;
    } else {
        std::cout << "Compilation was not succesfull: " << errors << " errors!" << std::endl;
    }
}
