#include <iostream>
#include <fstream>
#include <string>
#include "parser.h"
#include <regex>
#include <algorithm>
#include "utils.h"
#include "token.h"
#include "file.h"

Parser::Parser(std::string vmfile, Token &token)
{
    //ifstream fin;
    fin.open(vmfile);
    tk = token;
    input_filename_stem = getNameStem(vmfile);
}

void Parser::parse()
{
    std::istringstream line(currentCmdLine);
    str_frags = {std::istream_iterator<std::string>{line},
                 std::istream_iterator<std::string>{}};
    // Test
    // for (auto &str : str_frags)
    //     std::cout << str + " ";
    // std::cout << std::endl;

    // Need error handler here to check whehter srt_frags[0] is in the map.
    cmd = tk.getToken(str_frags[0]);
    command_type = tk.getType(cmd);
    switch (command_type)
    {
    case Token::C_ARITHMETIC:
        break;
    case Token::C_PUSH:
        arg1 = tk.getToken(str_frags[1]);
        arg2 = stoi((str_frags[2]));
        break;
    case Token::C_POP:
        arg1 = tk.getToken(str_frags[1]);
        arg2 = stoi((str_frags[2]));
        break;
    case Token::C_LABEL:
        label = str_frags[1];
        break;
    case Token::C_GOTO:
        label = str_frags[1];
        break;
    case Token::C_IF:
        label = str_frags[1];
        break;
    case Token::C_FUNCTION:
        label = str_frags[1];
        arg2 = stoi((str_frags[2]));
        break;
    case Token::C_RETURN:
        break;
    case Token::C_CALL:
        label = str_frags[1];
        arg2 = stoi((str_frags[2]));
        break;
    default:
        break;
    }
}

bool Parser::hasMoreCommands()
{
    if (fin.peek() != EOF)
        return true;
    else
        return false;
}

// Variables for advance()
void Parser::advance()
{
    std::string line;
    getline(fin, line);
    line = removeComments(line);
    line = trimOuterSpaces(line);
    // Skip empty line(s) after removing comments and triming.
    while (line.empty())
    {
        getline(fin, line);
        line = removeComments(line);
        line = trimOuterSpaces(line);
    }
    currentCmdLine = line;
}

int Parser::commandType()
{
    // if (!tk.hasKey(cmd))
    //     return -1;
    // else
    // {
    int token = cmd;
    //cout << tk.getType(token) << " " << tk.getName(token) << endl;
    // command_type = static_cast<Token::COMMAND_TYPE>(tk.getType(token));
    return tk.getType(token);
    // }
}

void Parser::closeFstream()
{
    fin.close();
}

std::string Parser::getInputFileNameStem()
{
    return input_filename_stem;
}

std::string Parser::getCmdLine()
{
    return currentCmdLine;
}

int Parser::getCommandType()
{
    return command_type;
}

int Parser::getCommand()
{
    return cmd;
}

int Parser::getArg1()
{
    return arg1;
}

int Parser::getArg2()
{
    return arg2;
}

std::string Parser::getLabel()
{
    return label;
}