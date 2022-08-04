#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <map>
#include <iomanip>
#include "tradutor.h"
#include "token.h"
#include "trim.cpp"
using std::string;
int errorCount = 0;
class SymbolRecord
{
public:
    Token token;

    int posicao;
    bool definido = false;
    bool externo = false;
    bool publico = false;
    SymbolRecord(){};
    SymbolRecord(const Token &t, int pos) : token(t), posicao(pos){};
    friend std::ostream &operator<<(std::ostream &os, const SymbolRecord &s);
};
std::ostream &operator<<(std::ostream &os, const SymbolRecord &s)
{
    return os << std::boolalpha << "Name " << s.token.token
              << " Posicao:" << s.posicao
              << " Externo " << s.externo
              << " Publico " << s.publico;
}
bool hasBegin;
std::unordered_map<string, SymbolRecord> tabelaSimbolo; // Tabela de simbolos;
std::unordered_map<string, std::vector<int>> tabelaUso; // Tabela de uso;
std::vector<Token> labelStack;
int contador = 0;

std::pair<Symbol, std::string> getNextToken(string &str)
{
    std::smatch m;
    // bool verbose = true;
    for (auto &[k, v] : patterns)
    {

        std::regex e(v);

        while (std::regex_search(str, m, e, std::regex_constants::match_continuous))
        {
            // std::cout << m[0].str() << "\n";
            // std::vector<std::pair<Symbol, string>> vec;

            for (auto x : m)
            {
                if (x.matched)
                {
                    string r = x.str();
                    std::pair<Symbol, string> kr = {k, r};
                    str = m.suffix().str();
                    str;
                    return kr;
                }
            }
        }

        // std::cout << v << " unmatched " << "\n";
    }
    throw std::logic_error("No match during tokenization");
}
void tokenize(std::pair<std::string, int> linha, std::vector<Token> &tokens)
{
    std::string trimmed = trim_copy(linha.first);
    while (!trimmed.empty())
    {
        auto token = getNextToken(trimmed);
        if ((int)token.first == 30)
        {
            std::cout << "\n";
            errorCount++;
            std::cout << "Erro léxico " << linha.first << "\n";
        }
        tokens.push_back({token.first, token.second, linha.second});
    }
}

std::ostream &operator<<(std::ostream &o, Estado e)
{
    return o << statenames[e];
}

/// 3 arguemtnos [-p||-o], in, out
/*
0           1       2                                 4
Start -> Seção -> Texto -> Comandos     -> Seção -> Dados -> comandos -> fim
-> Comandos*    -> Fim
*/

void addLabelStack(const Token &t)
{
    labelStack.push_back(t);
}
void clearLabelStack(int posicao, bool definido, bool externo = false, bool publico = false)
{
    for (auto l : labelStack)
    {
        if (tabelaSimbolo[l.token].definido == false)
        {
            tabelaSimbolo[l.token].posicao = posicao;
            tabelaSimbolo[l.token].definido = definido;
            tabelaSimbolo[l.token].externo = externo;
            if (!tabelaSimbolo[l.token].publico)
                tabelaSimbolo[l.token].publico = publico;
            tabelaSimbolo[l.token].token = l;
        }
        else
        {
            std::stringstream ss;
            errorCount++;
            ss << "Erro Semântico: Redefinição de " << l.token << "na linha: " << l.linha + 1 << "\n";
            std::cout << ss.str();
            // throw std::runtime_error();
        }
    }
    labelStack.clear();
}
std::unordered_map<Estado, std::function<Estado(Token)>> transicoes = {
    {Estado::Inicial, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::SECAO_TEXTO:
             return Estado::SecaoT;

         case Symbol::LABEL:
             addLabelStack(t);
             return Estado::TabelaSD;

         case Symbol::SECAO_DADOS:
             return Estado::SecaoD;

         case Symbol::PUBLIC:
             return Estado::TabelaUso;

         default:
             return Estado::Erro;
         }
     }},
    {Estado::TabelaSD, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::LABEL:
             addLabelStack(t);
             return Estado::TabelaSD;
         case Symbol::EXTERN:
             clearLabelStack(-1, true, true, false);
             return Estado::Inicial;
         case Symbol::OP_BEGIN:
             clearLabelStack(0, true, false, true);
             if (!hasBegin)
                 hasBegin = true;
             else
                 throw std::runtime_error("BEGIN redefinido");
             return Estado::Inicial;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::TabelaUso, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::ID:
             addLabelStack(t);
             clearLabelStack(-1, false, false, true);
             return Estado::Inicial;
         default:
             return Estado::Erro;
         }
     }},
    {Estado::SecaoT, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::INPUT:
         case Symbol::ADD:
         case Symbol::SUB:
         case Symbol::MUL:
         case Symbol::MULT:
         case Symbol::DIV:
         case Symbol::JMP:
         case Symbol::JMPN:
         case Symbol::JMPP:
         case Symbol::JMPZ:
         case Symbol::LOAD:
         case Symbol::STORE:
         case Symbol::OUTPUT:
             contador++;
             return Estado::Read1;
         case Symbol::COPY:
             contador++;
             return Estado::Read2;
         case Symbol::LABEL:
             addLabelStack(t);
             return Estado::LabelT;
         // estado que espera estado ou um t.symbol e opera na tabela de simbolos;
         case Symbol::STOP:
             contador++;
             return Estado::WaitDataOrEnd;
         default:

             return Estado::Erro;
             break;
         }
     }},
    {Estado::SecaoD, [](const Token &t)
     {
         switch (t.symbol)
         {

         case Symbol::LABEL:
             addLabelStack(t);
             return Estado::LabelD;
         case Symbol::END:
             if (hasBegin)
                 return Estado::Fim;
             else
                 return Estado::Erro;
         // estado que espera estado ou um t.symbol e opera na tabela de simbolos;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::WaitDataOrEnd, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::SECAO_DADOS:
             return Estado::SecaoD;
         case Symbol::END:
             if (hasBegin)
                 return Estado::Fim;
             else
                 return Estado::Erro;
         default:
             return Estado::Erro; // Comando ou label definido fora de seção;
             break;
         }
     }},
    // {Estado::ComandosT, [](Symbol t.symbol) {
    //     try
    //     {
    //         return comandos[t.symbol].e;
    //     } catch (const std::exception& e)
    //     {
    //         return Estado::Erro;
    //     }
    //     // return t.symbol == Symbol::SECAO") || t.symbol == Symbol::FIM") ? Estado::Erro : Estado::SecaoD;
    //     // return Estado::Inicial; // TODO: CORRIGIR ESTADO DE RETORNO>
    //     }
    // },
    // {Estado::ComandosD, [](Symbol t.symbol) {
    //     if (t.symbol == Symbol::SECAO_DADOS == 0) {
    //         return Estado::Fim;
    //     } else {
    //         return Estado::ComandosD;
    //     }
    // }},
    {Estado::Read2, [](const Token &t)
     {
         contador++;
         return t.symbol == Symbol::ID ? Estado::ReadCOMMA : Estado::Erro;
     }},
    {Estado::ReadCOMMA, [](const Token &t)
     {
         return t.symbol == Symbol::COMMA ? Estado::Read1 : Estado::Erro;
     }},
    {Estado::Read1, [](const Token &t)
     {
         contador++;
         switch (t.symbol)
         {
         case Symbol::ID:
         case Symbol::NUMBER:
             return Estado::SecaoT;
         default:
             return Estado::Erro;
         }
     }},
    {Estado::LabelT, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::INPUT:
         case Symbol::ADD:
         case Symbol::SUB:
         case Symbol::MUL:
         case Symbol::MULT:
         case Symbol::DIV:
         case Symbol::JMP:
         case Symbol::JMPN:
         case Symbol::JMPP:
         case Symbol::JMPZ:
         case Symbol::LOAD:
         case Symbol::STORE:
         case Symbol::OUTPUT:
             clearLabelStack(contador++, true, false, false);
             return Estado::Read1;
         case Symbol::COPY:
             clearLabelStack(contador++, true, false, false);
             return Estado::Read2;
         case Symbol::LABEL:
             labelStack.push_back(t); // enquanto encontrar labels, empilhá-las pois todas apontam para a próxima referencia;
             return Estado::LabelT;
         // estado que espera estado ou um t.symbol e opera na tabela de simbolos;
         case Symbol::STOP:
             clearLabelStack(contador++, true, false, false);
             return Estado::WaitDataOrEnd;
         default:
             return Estado::Erro;
             break;
         }
         return Estado::Erro;
     }},
    {Estado::LabelD, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::LABEL:
             labelStack.push_back(t);
             return Estado::LabelD;
         case Symbol::CONST:
             // popar toda a stack de labels e fazer referencia ao endereço atual.
             clearLabelStack(contador, true, false, false);
             return Estado::ReadImmediate;
         case Symbol::SPACE:
             clearLabelStack(contador++, true, false, false);
             return Estado::SecaoD;
         // tem que cuidar da tabela de definição tb;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::ReadImmediate, [](const Token &t)
     {
         switch (t.symbol)
         {
         case Symbol::NUMBER:
             contador++;
             return Estado::SecaoD;
         default:
             return Estado::Erro;
         }
     }},
    {Estado::Erro, [](const Token &t)
     { return Estado::Erro; }},
    {Estado::Stop, [](const Token &t)
     { return t.symbol == Symbol::SECAO_DADOS ? Estado::Erro : Estado::SecaoD; }},
    {Estado::Fim, [](const Token &t)
     { return Estado::Fim; }}};

// std::unordered_map<string, int>tabelaErro; // Tabela de erros;

std::vector<Token> firstPass(std::vector<std::pair<string, int>> &file)
{
    std::pair<std::string, int> linha;

    Estado estado = Estado::Inicial;
    std::vector<Token> tokens;
    // Contar as linhas, marcar ocorrências de labels com o contador;
    // ao verificar SECAO texto atualizar os valores na tabela de simbolos;
    for (int i = 0; i < file.size(); i++)
    {
        linha = file[i];
        tokenize(linha, tokens);
    }

    string token;
    Symbol simbolo;
    // verificar erros;
    for (int i = 0; i < tokens.size(); i++)
    {
        simbolo = tokens[i].symbol;
        token = tokens[i].token;
        std::cout << "token: " << token << " simbolo: " << (int)simbolo << " estado atual:" << estado << "\n";
        estado = transicoes[estado](tokens[i]);
        if (estado == Estado::Erro)
        {
            errorCount++;
            std::cout << "Erro sintático na linha " << tokens[i].linha << "\n";
        }
    }
    return tokens;
}

void printSymbolTable(bool onlyPublico = false)
{
    for (auto &[k, v] : tabelaSimbolo)
    {
        if (!onlyPublico)
            std::cout << v << "\n";
        else if (v.publico)
            std::cout << v << "\n";
    }
}

std::vector<string> secondPass(std::vector<Token> tokens)
{
    // if (hasBegin)
    // {
    // }
    std::cout << "second pass\n";
    std::vector<string> processed;
    for (auto t : tokens)
    {
        if (comandos.find(t.symbol) != comandos.end())
        {
            int code = comandos[t.symbol].OPCODE;
            processed.push_back(std::to_string(code));
        }
        else if (tabelaSimbolo.find(t.token) != tabelaSimbolo.end())
        {
            if (tabelaSimbolo[t.token].definido)
            {
                processed.push_back(std::to_string(tabelaSimbolo[t.token].posicao));
            }
            else
            {
                processed.push_back("xx");
            }
        }
        else
        {
            std::cout << "Token não definido: " << t.token << "na linha " << t.linha << std::endl;
        }
    }
    return processed;
}

std::vector<string> traduzir(std::vector<std::pair<string, int>> &processed)
{
    std::vector<string> traduzido;
    int i;
    string token;
    string linha;
    std::vector<Token> tokens = firstPass(processed);
    if (errorCount)
    {
        std::cout << "contagem de erros superior a 1 interrompendo execução antes da segunda passagem\n";
        exit(-1);
    }
    traduzido = secondPass(tokens);
    std::stringstream ss;
    // escrever pro arquivo;
    printSymbolTable();
    std::cout << "-=-=-=-=-=-=--=-=-=-=-=-\n";
    printSymbolTable(true);
    for (auto num : traduzido)
    {
        ss << num << " ";
    }
    std::cout << ss.str();
    return traduzido;
}
