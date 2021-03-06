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
#include "token.h"
#include <stdio.h>

// Constructor without refering to a parser. Used for write initial codes.
CodeGenerator::CodeGenerator(FILE *file)
{
    _hackfile = file;
}

CodeGenerator::CodeGenerator(FILE *file, Parser *parser)
{
    _hackfile = file;
    _parser = parser;
}

void CodeGenerator::writeHack()
{
    if (generate_comments)
        fprintf(_hackfile, "// %s\n", _parser->getCmdLine().c_str());

    switch (_parser->getCommandType())
    {
    case Token::C_PUSH:
        writePush(_parser->getArg1(), _parser->getArg2());
        break;
    case Token::C_POP:
        writePop(_parser->getCommand(), _parser->getArg1(), _parser->getArg2());
        break;
    case Token::C_ARITHMETIC:
        writeArithmetic(_parser->getCommand());
        break;
    case Token::C_GOTO:
        writeGoto(_parser->getLabel());
        break;
    case Token::C_LABEL:
        writeLabel(_parser->getLabel());
        break;
    case Token::C_IF:
        writeIf(_parser->getLabel());
        break;
    case Token::C_FUNCTION:
        writeFunction(_parser->getLabel(), _parser->getArg2());
        break;
    case Token::C_RETURN:
        writeReturn();
        break;
    case Token::C_CALL:
        writeCall(_parser->getLabel(), _parser->getArg2());
        break;
    default:
        break;
    }
}

const char *pop_stem = "@SP\n"
                       "AM=M-1\n"
                       "D=M\n"
                       "A=A-1\n";

const char *AddSubAndOr_stem = "@SP\n"
                               "AM=M-1\n"
                               "D=M\n"
                               "@SP\n"
                               "A=M-1\n";

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
                       "@_TRUE%d\n" // index
                       "D;%s\n"     // compa_predicate
                       "D=0\n"
                       "@_FALSE%d\n"
                       "0;JMP\n"
                       "(_TRUE%d)\n"
                       "D=-1\n"
                       "(_FALSE%d)\n"
                       "@SP\n"
                       "A=M-1\n"
                       "M=D\n", // index
            index, compa_predicate.c_str(), index, index, index);
    index++;
}

void CodeGenerator::writeArithmetic(int command)
{
    switch (command)
    {
    case Token::ADD:
        fprintf(_hackfile, AddSubAndOr_stem);
        fprintf(_hackfile, "M=M+D\n");
        break;
    case Token::SUB:
        fprintf(_hackfile, AddSubAndOr_stem);
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
        fprintf(_hackfile, AddSubAndOr_stem);
        fprintf(_hackfile, "M=M&D\n");
        break;
    case Token::OR:
        fprintf(_hackfile, AddSubAndOr_stem);
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
const char *push_template1 = "@%s\n"
                             "D=M\n"
                             "@%d\n"
                             "D=D+A\n"
                             "A=D\n"
                             "D=M\n"
                             "@SP\n"
                             "AM=M+1\n"
                             "A=A-1\n"
                             "M=D\n";

const char *push_template2 = "@%s\n"
                             "A=M\n"
                             "D=M\n"
                             "@SP\n"
                             "A=M\n"
                             "M=D\n"
                             "@SP\n"
                             "M=M+1\n";

const char *push_template_temp = "@%d\n"
                                 "D=A\n"
                                 "@%s\n"
                                 "D=D+A\n"
                                 "A=D\n"
                                 "D=M\n"
                                 "@SP\n"
                                 "A=M\n"
                                 "M=D\n"
                                 "@SP\n"
                                 "M=M+1\n";

void CodeGenerator::writePushTemplate(const char *segment, int arg2)
{
    fprintf(_hackfile, push_template1, segment, arg2);
}

void CodeGenerator::writePush(int arg1, int arg2)
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
                           "AM=M+1\n"
                           "A=A-1\n"
                           "M=D\n");
        break;

    case Token::TEMP:
        fprintf(_hackfile, push_template_temp, arg2, temp_segment);
        break;

    case Token::POINTER:
        if (arg2 == 1)
            pointer_segment = "THAT";
        else if (arg2 == 0)
            pointer_segment = "THIS";
        fprintf(_hackfile, "@%s\n"
                           "D=M\n"
                           "@SP\n"
                           "A=M\n"
                           "M=D\n"
                           "@SP\n"
                           "M=M+1\n",
                pointer_segment);
        break;

    case Token::STATIC:
        fprintf(_hackfile, "@%s.%d\n"
                           "D=M\n"
                           "@SP\n"
                           "A=M\n"
                           "M=D\n"
                           "@SP\n"
                           "M=M+1\n",
                _parser->getInputFileNameStem().c_str(), arg2);
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

// _debug: the first arg can be deleted.
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
        fprintf(_hackfile, pop_template_temp, temp_segment, arg2);
        // writePopTemplate(temp_segment, arg2);
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

    case Token::STATIC:
        fprintf(_hackfile, "@SP\n"
                           "AM=M-1\n"
                           "D=M\n"
                           "@%s.%d\n"
                           "M=D\n",
                _parser->getInputFileNameStem().c_str(), arg2);
        break;

    default:
        break;
    }
}

void CodeGenerator::writeInit()
{
    fprintf(_hackfile, "// SP=256\n");
    fprintf(_hackfile, "@256\n"
                       "D=A\n"
                       "@SP\n"
                       "M=D\n"
                       "// call Sys.init\n");
    writeCall("Sys.init", 0);
}

void CodeGenerator::writeLabel(std::string label)
{
    std::string prefix_label = _parser->getInputFileNameStem() + "." + label;
    std::transform(prefix_label.begin(), prefix_label.end(), prefix_label.begin(), ::toupper);
    fprintf(_hackfile, "(%s)\n", prefix_label.c_str());
}

void CodeGenerator::writeGoto(std::string label)
{
    std::string prefix_label = _parser->getInputFileNameStem() + "." + label;
    std::transform(prefix_label.begin(), prefix_label.end(), prefix_label.begin(), ::toupper);
    fprintf(_hackfile, "@%s\n"
                       "0;JMP\n",
            prefix_label.c_str());
}

void CodeGenerator::writeIf(std::string label)
{
    std::string prefix_label = _parser->getInputFileNameStem() + "." + label;
    std::transform(prefix_label.begin(), prefix_label.end(), prefix_label.begin(), ::toupper);
    fprintf(_hackfile, "@SP\n"
                       "AM=M-1\n"
                       "D=M\n"
                       "@%s\n"
                       "D;JNE\n", // True branching(D=111...111).
            prefix_label.c_str());
}

void CodeGenerator::writeFunction(std::string label, int num_locals)
{
    // Convert to upper case.
    std::transform(label.begin(), label.end(), label.begin(), ::toupper);
    fprintf(_hackfile, "(%s)\n", label.c_str());
    // Initialize
    while (num_locals != 0)
    {
        writePush(Token::CONSTANT, 0);
        num_locals--;
    }
}

int call_index = 0;
const std::string pointer_array_call[] = {"LCL", "ARG", "THIS", "THAT"};
std::string push_template_call = "";
std::string pointer_template_call = "";
void CodeGenerator::writeCall(std::string func_name, int num_args)
{
    // Convert to upper case.
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::toupper);
    std::string ret_label;
    ret_label = "RETADDR." + func_name + "." + std::to_string(call_index);

    // for (const std::string &pointer : pointer_array_call)
    // {
    //     pointer_template_call += std::string("@") + pointer + "\n" +
    //                              //  "A=M\n" +
    //                              "D=M\n" +
    //                              "@SP\n" +
    //                              "A=M\n" +
    //                              "M=D\n" +
    //                              "@SP\n" +
    //                              "M=M+1\n";
    // }

    // push returnAddress
    fprintf(_hackfile, "@%s\n"
                       "D=A\n"
                       "@SP\n"
                       "AM=M+1\n"
                       "A=A-1\n"
                       "M=D\n",
            ret_label.c_str());

    // push LCL, ARG, THIS, THAT
    // fprintf(_hackfile, pointer_template_call.c_str());
    writePush("LCL");
    writePush("ARG");
    writePush("THIS");
    writePush("THAT");

    // ARG = SP - 5 -nArgs
    fprintf(_hackfile, "@SP\n"
                       "D=M\n"
                       "@5\n"
                       "D=D-A\n"
                       "@%d\n"
                       "D=D-A\n"
                       "@ARG\n"
                       "M=D\n"
                       "@SP\n"
                       "D=M\n"
                       "@LCL\n"
                       "M=D\n",
            num_args);

    // LCL = SP
    // fprintf(_hackfile, "@SP\n"
    //                    "A=M\n"
    //                    "D=M\n"
    //                    "@LCL\n"
    //                    "A=M\n"
    //                    "M=D\n");

    // goto functionName
    fprintf(_hackfile, "@%s\n"
                       "0;JMP\n",
            func_name.c_str());

    // write label for return address
    fprintf(_hackfile, "(%s)\n", ret_label.c_str());
    call_index++;
}

void CodeGenerator::writePush(std::string segment)
{
    fprintf(_hackfile, "@%s\n"
                       "D=M\n"
                       "@SP\n"
                       "AM=M+1\n"
                       "A=A-1\n"
                       "M=D\n",
            segment.c_str());
}

const std::string pointer_array[] = {"THAT", "THIS", "ARG", "LCL"};
std::string pointer_template = "";
void CodeGenerator::writeReturn()
{
    // for (const std::string &pointer : pointer_array)
    // {
    //     pointer_template += std::string("@endFrame\n") +
    //                         "D=M-1\n" +
    //                         "AM=D\n" +
    //                         "D=M\n" +
    //                         "@" +
    //                         pointer +
    //                         "\n" +
    //                         "M=D\n";
    // }
    fprintf(_hackfile, "@LCL\n"
                       "D=M\n"
                       "@endFrame\n"
                       "M=D\n" // endFrame = LCL
                       "@5\n"
                       "A=D-A\n"
                       "D=M\n"
                       "@retAddr\n"
                       "M=D\n" // retAddr = *(endFrame - 5)
    );

    // writePop(Token::POP, Token::ARGUMENT, 0); // *ARG = pop()
    // fprintf(_hackfile, "@SP\n"
    //                    "AM=M-1\n"
    //                    "D=M\n"
    //                    "@ARG\n"
    //                    "A=M\n"
    //                    "M=D\n" // *ARG = pop()
    //                    "@ARG\n"
    //                    "D=M+1\n"
    //                    "@SP\n"
    //                    "M=D\n" // SP = ARG + 1
    // );
    writePop("ARG");
    fprintf(_hackfile, "@ARG\n"
                       "D=M\n"
                       "@SP\n"
                       "M=D+1\n" // SP = ARG + 1
    );

    // fprintf(_hackfile, pointer_template.c_str());

    writeFrameRestore("THAT", 1);
    writeFrameRestore("THIS", 2);
    writeFrameRestore("ARG", 3);
    writeFrameRestore("LCL", 4);

    fprintf(_hackfile, "@retAddr\n"
                       "A=M\n"
                       "0;JMP\n" // goto retAddr
    );
}

void CodeGenerator::writePop(std::string segment)
{
    fprintf(_hackfile, "@SP\n"
                       "A=M-1\n"
                       "D=M\n"
                       "@%s\n"
                       "A=M\n"
                       "M=D\n",
            segment.c_str());
}

void CodeGenerator::writeFrameRestore(std::string segment, int minus)
{
    fprintf(_hackfile, "@endFrame\n"
                       "D=M\n"
                       "@%d\n"
                       "A=D-A\n"
                       "D=M\n"
                       "@%s\n"
                       "M=D\n",
            minus, segment.c_str());
}