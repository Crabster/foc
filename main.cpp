#include <cstdlib>
#include <iostream>
#include "FocLexer.h"
#include "src/code_visitor.hpp"
#include "src/code_generator.hpp"
#include "src/syntax_check.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    std::string file_name = argv[1];
    std::ifstream stream;
    stream.open(file_name);

    antlr4::ANTLRInputStream input(stream);

    foc::FocLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    foc::FocParser parser(&tokens);
    foc::FocParser::ProgramContext* tree = parser.program();

    foc::CodeVisitor visitor;
    foc::Program program = visitor.visitProgram(tree).as<foc::Program>();

    std::cout << program.to_string() << "\n----------------------\n" << std::endl;
    if (foc::syntax_check(program)) {
        std::cout << "All OK" << std::endl;
    } else {
        std::cout << "Some problem" << std::endl;
    }

    foc::CodeGenerator code_gen("example.asm");
    code_gen.generate_asm(program);
    std::system("nasm -f elf64 -o example.o example.asm && ld -o example example.o");
}
