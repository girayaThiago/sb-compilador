#ifndef _tradutorH
#define _tradutorH

#include <functional>
#include <unordered_map>
#include <utility>
#include <map>
#include <stdexcept>
#include <string>

enum class Estado : int {
    Inicial,
    SecaoT,
    ComandosT,
    Read2,
    Read1,
    Stop,
    SecaoD,
    ComandosD,
    Erro,
    Fim,
    TabelaSD,
    TabelaUso
};

class Command {
    std::string OPNAME;
    int OPCODE;
    int size;

public:
    Estado e;
    Command() {
        throw std::logic_error("Tentando instanciar um comando sem argumentos");
    }
    Command(std::string name, int opcode, int size) :OPNAME(name), OPCODE(opcode), size(size) {
        if (name.compare("STOP") == 0) {
            e = Estado::Stop;
        } else if (name.compare("COPY") == 0) {
            e = Estado::Read2;
        } else {
            e = Estado::Read1;
        }
    }
};

enum class Symbol : int {
    OP_BEGIN,
    LABEL,
    COMMENT,
    EQU,
    IF,
    SECAO_TEXTO,
    SECAO_DADOS,
    EXTERN,
    PUBLIC,
    ADD,
    SUB,
    MULT,
    MUL,
    DIV,
    JMPN,
    JMPP,
    JMPZ,
    JMP,
    COPY,
    LOAD,
    STORE,
    INPUT,
    OUTPUT,
    STOP,
    SPACE,
    CONST,
    END,
    ID,
    NUMBER,
    ERRO
};

static std::map<Symbol, std::string> patterns = {
        {Symbol::OP_BEGIN, "(BEGIN)"},
        {Symbol::LABEL, "([a-zA-Z_][a-zA-Z0-9_]*[\\:])"},
        {Symbol::COMMENT, "(;.*)"},
        {Symbol::EQU, "(EQU )"},
        {Symbol::IF, "(IF )"},
        {Symbol::SECAO_TEXTO, "(SECAO TEXTO$)"},
        {Symbol::SECAO_DADOS, "(SECAO DADOS$)"},
        {Symbol::EXTERN, "(EXTERN )"},
        {Symbol::PUBLIC, "(PUBLIC )"},
        {Symbol::ADD, "(ADD )"},
        {Symbol::SUB, "(SUB )"},
        {Symbol::MULT, "(MULT )"},
        {Symbol::MUL, "(MUL )"},
        {Symbol::DIV, "(DIV )"},
        {Symbol::JMPN, "(JMPN )"},
        {Symbol::JMPP, "(JMPP )"},
        {Symbol::JMPZ, "(JMPZ )"},
        {Symbol::JMP, "(JMP )"},
        {Symbol::COPY, "(COPY )"},
        {Symbol::LOAD, "(LOAD )"},
        {Symbol::STORE, "(STORE )"},
        {Symbol::INPUT, "(INPUT )"},
        {Symbol::OUTPUT, "(OUTPUT )"},
        {Symbol::STOP, "(STOP$)"},
        {Symbol::SPACE, "(SPACE$)"},
        {Symbol::CONST, "(CONST )"},
        {Symbol::END, "(END$)"},
        {Symbol::ID, "([a-zA-Z_][a-zA-Z0-9_]*)"},
        {Symbol::NUMBER, "([0-9]+)"},
        {Symbol::ERRO, "(.)"}
};

static std::unordered_map<Symbol, Command> comandos = {
    {Symbol::ADD,Command("ADD", 1, 2)},
    {Symbol::SUB,Command("SUB", 2, 2)},
    {Symbol::MUL,Command("MUL", 3, 2)},
    {Symbol::MULT,Command("MUL", 3, 2)},
    {Symbol::DIV,Command("DIV", 4, 2)},
    {Symbol::JMP,Command("JMP", 5, 2)},
    {Symbol::JMPN,Command("JMPN", 6, 2)},
    {Symbol::JMPP,Command("JMPP", 7, 2)},
    {Symbol::JMPZ,Command("JMPZ", 8, 2)},
    {Symbol::COPY,Command("COPY", 9, 3)},
    {Symbol::LOAD,Command("LOAD", 10, 2)},
    {Symbol::STORE,Command("STORE", 11, 2)},
    {Symbol::INPUT,Command("INPUT", 12, 2)},
    {Symbol::OUTPUT,Command("OUTPUT", 13, 2)},
    {Symbol::STOP,Command("STOP", 14, 1)}
};

static std::unordered_map<Estado, std::string> statenames = {
    {Estado::Inicial,"Inicial"},
    {Estado::SecaoT,"SecaoT"},
    {Estado::ComandosT,"ComandosT"},
    {Estado::ComandosT,"ComandosT"},
    {Estado::Read2,"Read2"},
    {Estado::Read1,"Read1"},
    {Estado::Stop,"Stop"},
    {Estado::SecaoD,"SecaoD"},
    {Estado::ComandosD,"ComandosD"},
    {Estado::Erro,"Erro Sintático"},
    {Estado::Fim,"Fim"}
};

std::vector<std::string> traduzir(std::vector<std::pair<std::string, int>>& processed);

#endif