
#ifndef _tokenh
#define _tokenh

#include <string>
enum class Symbol : int
{
    LABEL,
    OP_BEGIN,
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
    NUMBER,
    ID,
    COMMA,
    ERRO
};

class Token
{
private:
public:
    Symbol symbol;
    std::string token;
    int linha;
    Token(){};
    Token(Symbol s, std::string t, int l) : symbol(s), token(t), linha(l){};
};

#endif //