#include "code_gen.h"
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <initializer_list>
#include <bitset>
#include <cstring>
#include "parser.h"
#include "file.h"
#include "utils.h"
#include "symbol_table.h"
#include "token.h"
#include <stdio.h>

CodeGenerator::CodeGenerator(FILE *file, Parser *parser)
{
    _hackfile = file;
    _parser = parser;
}

void CodeGenerator::writeHack()
{
    if (generate_comments)
        fprintf(_hackfile, "//%s\n", _parser->currentCmdLine.c_str());

    switch (_parser->command_type)
    {
    case Token::C_PUSH:
        writePush(_parser->cmd, _parser->arg1, _parser->arg2);
        break;
    case Token::C_POP:
        writePop(_parser->cmd, _parser->arg1, _parser->arg2);
        break;
    case Token::C_ARITHMETIC:
        writeArithmetic(_parser->cmd);
        break;
    default:
        break;
    }
}

const char *pop_stem = "@SP\n"
                       "AM=M-1\n"
                       "D=M\n"
                       "A=A-1\n";

const char *stack_dec = "@SP\n"
                        "AM=M-1\n";

const char *pointer_inc = "@SP\n"
                          "M=M+1\n";

void CodeGenerator::writeCompa(std::string compa_predicate)
{
    fprintf(_hackfile, "@SP\n"
                       "AM=M-1\n"
                       "D=M\n"
                       "A=A-1\n"
                       "D=M-D\n"
                       "@TRUE%d\n" // index
                       "D;%s\n"    // compa_predicate
                       "@SP\n"
                       "AM=M-1\n"
                       "M=0\n"
                       "@SP\n"
                       "M=M+1\n"
                       "@FALSE%d\n" // index
                       "0;JMP\n"
                       "(TRUE%d)\n" // index
                       "@SP\n"
                       "AM=M-1\n"
                       "M=-1\n"
                       "@SP\n"
                       "M=M+1\n"
                       "(FALSE%d)\n", // index
            index, compa_predicate.c_str(), index, index, index);
    index++;
}

void CodeGenerator::writeArithmetic(int command)
{
    switch (command)
    {
    case Token::ADD:
        fprintf(_hackfile, pop_stem);
        fprintf(_hackfile, "M=M+D\n");
        break;
    case Token::SUB:
        fprintf(_hackfile, pop_stem);
        fprintf(_hackfile, "M=M-D\n");
        break;
    case Token::NEG:
        fprintf(_hackfile, stack_dec);
        fprintf(_hackfile, "M=-M\n");
        fprintf(_hackfile, pointer_inc);
        break;
    case Token::EQ:
        writeCompa("JEQ");
        break;
    case Token::GT:
        writeCompa("JGT");
        break;
    case Token::LT:
        writeCompa("JLT");
        break;
    case Token::AND:
        fprintf(_hackfile, pop_stem);
        fprintf(_hackfile, "M=M&D\n");
        break;
    case Token::OR:
        fprintf(_hackfile, pop_stem);
        fprintf(_hackfile, "M=M|D\n");
        break;
    case Token::NOT:
        fprintf(_hackfile, stack_dec);
        fprintf(_hackfile, "M=!M\n");
        fprintf(_hackfile, pointer_inc);
        break;
    default:
        break;
    }
}

// TEMP segment maps on RAM[5] to RAM[12]
const char *temp_segment = "5";
const char *pointer_segment = "THIS";
const char *push_template1 = "@%d\n"
                             "D=A\n"
                             "@%s\n"
                             "A=M\n"
                             "A=A+D\n"
                             "D=M\n"
                             "@SP\n"
                             "A=M\n"
                             "M=D\n"
                             "@SP\n"
                             "M=M+1\n";

const char *push_template2 = "@%s\n"
                             "A=M\n"
                             "D=M\n"
                             "@SP\n"
                             "A=M\n"
                             "M=D\n"
                             "@SP\n"
                             "M=M+1\n";

void CodeGenerator::writePushTemplate(const char *segment, int arg2)
{
    if (arg2 == 0)
        fprintf(_hackfile, push_template2, segment);
    else
        fprintf(_hackfile, push_template1, arg2, segment);
}

void CodeGenerator::writePush(int command, int arg1, int arg2)
{
    switch (arg1)
    {
    case Token::LOCAL:
        writePushTemplate("LCL", arg2);
        break;

    case Token::ARGUMENT:
        writePushTemplate("ARG", arg2);
        break;

    case Token::THIS:
        writePushTemplate("THIS", arg2);
        break;

    case Token::THAT:
        writePushTemplate("THAT", arg2);
        break;

    case Token::CONSTANT:
        fprintf(_hackfile, "@%d\n", arg2);
        fprintf(_hackfile, "D=A\n"
                           "@SP\n"
                           "A=M\n"
                           "M=D\n"
                           "@SP\n"
                           "M=M+1\n");
        break;

    case Token::TEMP:
        writePushTemplate(temp_segment, arg2);
        break;

    case Token::POINTER:
        if (arg2 == 1)
            pointer_segment = "THAT";
        else if (arg2 == 0)
            pointer_segment = "THIS";
        fprintf(_hackfile, "@%s\n"
                           "A=M\n"
                           "D=M\n"
                           "@SP\n"
                           "A=M\n"
                           "M=D\n"
                           "@SP\n"
                           "M=M+1\n",
                pointer_segment);
        break;
    default:
        break;
    }
}

// Normal template
const char *pop_template1 = "@%s\n"
                            "D=M\n"
                            "@%d\n"
                            "D=D+A\n"
                            "@R15\n"
                            "M=D\n"
                            "@SP\n"
                            "AM=M-1\n"
                            "D=M\n"
                            "@R15\n"
                            "A=M\n"
                            "M=D\n";

// Template when arg2=0 since "A=A+0" is illegal
const char *pop_template2 = "@%s\n"
                            "D=M\n"
                            "@R15\n"
                            "M=D\n"
                            "@SP\n"
                            "AM=M-1\n"
                            "D=M\n"
                            "@R15\n"
                            "A=M\n"
                            "M=D\n";

const char *pop_template_temp = "@%s\n"
                                "D=A\n"
                                "@%d\n"
                                "D=D+A\n"
                                "@R15\n"
                                "M=D\n"
                                "@SP\n"
                                "AM=M-1\n"
                                "D=M\n"
                                "@R15\n"
                                "A=M\n"
                                "M=D\n";

void CodeGenerator::writePopTemplate(const char *segment, int arg2)
{
    if (arg2 == 0)
        fprintf(_hackfile, pop_template2, segment);
    else
        fprintf(_hackfile, pop_template1, segment, arg2);
}

void CodeGenerator::writePop(int command, int arg1, int arg2)
{
    switch (arg1)
    {
    case Token::LOCAL:
        writePopTemplate("LCL", arg2);
        break;

    case Token::ARGUMENT:
        writePopTemplate("ARG", arg2);
        break;

    case Token::THIS:
        writePopTemplate("THIS", arg2);
        break;

    case Token::THAT:
        writePopTemplate("THAT", arg2);
        break;

    case Token::TEMP:
        writePopTemplate(temp_segment, arg2);
        break;

    case Token::POINTER:
        if (arg2 == 1)
            pointer_segment = "THAT";
        else if (arg2 == 0)
            pointer_segment = "THIS";
        fprintf(_hackfile, "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@%s\n"
                           "M=D\n",
                pointer_segment);
        break;

    default:
        break;
    }
}
