#include <cstdlib>
#include <iostream>
#include <sstream>
#include "FocLexer.h"
#include "src/code_visitor.hpp"
#include "src/code_generator.hpp"
#include "src/syntax_check.hpp"

void print_help() {
    std::cout << "This is a compiler for a `Foc` language.\n";
    std::cout << "Usage: ./build/demo [args]\n";
    std::cout << "Possible arguments:\n";
    std::cout << "\t -h \t\t -> Prints help\n";
    std::cout << "\t -i `path` \t -> Compiles file on `path`\n";
    std::cout << "\t -d \t\t -> Enables debug mode for the compiler\n";
    std::cout << "\t -e `num` \t -> Compilation stops after `num` errors (default 10)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Invalid use, no arguments" << std::endl;
        print_help();
        return 1;
    }

    std::string file_name;
    bool set_filename = false;
    unsigned limit = 10;
    bool debug_mode = false;

    for (unsigned i = 1; i < argc; ++i) {
        std::string curr = argv[i];
        if (curr == "-h") {
            print_help();
            return 1;
        } else if (curr == "-d") {
            debug_mode = true;
        } else if (curr == "-e") {
            if (i + 1 >= argc) {
                std::cout << "Invalid use, argument `-e` without number" << std::endl;
                print_help();
                return 1;
            }
            std::stringstream strVal;
            strVal << argv[i+1];
            strVal >> limit;
            if (strVal.fail()) {
                std::cout << "Invalid use, argument after `-e` isn't unsigned number" << std::endl;
                print_help();
                return 1;
            }
            ++i;
        } else if (curr == "-i") {
            if (i + 1 >= argc) {
                std::cout << "Invalid use, argument `-i` without path" << std::endl;
                print_help();
                return 1;
            }
            file_name = argv[i+1];
            set_filename = true;
            ++i;
        } else {
            std::cout << "Invalid use, unknown argument `" << curr << "`" << std::endl;
            print_help();
            return 1;
        }
    }

    if (!set_filename) {
        std::cout << "Invalid use, no filepath given" << std::endl;
        print_help();
        return 1;
    }

    std::ifstream stream;
    stream.open(file_name);
    if (stream.fail()) {
        std::cout << "Couldn't open file `" << file_name << "`" << std::endl;
        print_help();
        return 1;
    }

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
        std::cout << "Too many errors, compilation stopped" << std::endl;
        return 1;
    } else {
        std::cout << "Compilation was not successful: " << errors << " errors!" << std::endl;
        return 1;
    }

    foc::CodeGenerator code_gen("example.asm");
    code_gen.generate_asm(program);
    std::system("nasm -f elf64 -o example.o example.asm && ld -o example example.o");
}
