#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <map>

#include "tradutor.h"
using std::string;

std::unordered_map<string, int> tabelaSimbolo;   // Tabela de simbolos;
std::unordered_map<string, int> tabelaUso;       // Tabela de uso; // extern
std::unordered_map<string, int> tabelaDefinicao; // Tabela de uso; // public
std::vector<string> labelStack;
int contador = 0;

std::pair<Symbol, std::string> getNextToken(string &str)
{
    std::smatch m;
    for (auto &[k, v] : patterns)
    {

        std::regex e(v);

        while (std::regex_search(str, m, e))
        {
            // std::cout << m[0].str() << "\n";
            for (auto x : m)
            {
                if (x.matched)
                {
                    // std::cout << v << " " << x << "\n";
                    string r = x.str();
                    std::pair<Symbol, string> kr = {k, r};
                    str = m.suffix().str();

                    return kr;
                }
            }
        }

        // std::cout << v << " unmatched " << "\n";
    }
    throw std::logic_error("No match during tokenization");
}
void tokenize(std::string str, std::vector<std::pair<Symbol, string>> &tokens)
{
    while (!str.empty())
    {
        auto token = getNextToken(str);
        tokens.push_back(token);
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

void popLabelStack()
{
    for (auto l : labelStack)
        tabelaSimbolo[l] = contador;
    labelStack.clear();
}
std::unordered_map<Estado, std::function<Estado(Symbol, const string &)>> transicoes = {
    {Estado::Inicial, [](Symbol input, const string &)
     {
         switch (input)
         {
         case Symbol::SECAO_TEXTO:
             return Estado::SecaoT;

         case Symbol::LABEL:
             return Estado::TabelaSD;

         case Symbol::SECAO_DADOS:
             return Estado::SecaoD;

         case Symbol::PUBLIC:
             return Estado::TabelaUso;

         default:
             return Estado::Erro;
         }
     }},

    {Estado::SecaoT, [](Symbol input, const string &token)
     {
         switch (input)
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
             return Estado::LabelT;
             // estado que espera estado ou um input e opera na tabela de simbolos;
         case Symbol::STOP:
             contador++;
             return Estado::WaitDataOrEnd;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::SecaoD, [](Symbol input, const string &token)
     {
         switch (input)
         {

         case Symbol::LABEL:
             labelStack.push_back(token);
             return Estado::LabelD;
             // estado que espera estado ou um input e opera na tabela de simbolos;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::WaitDataOrEnd, [](Symbol input, const string &token)
     {
         switch (input)
         {
         case Symbol::SECAO_DADOS:
             return Estado::SecaoD;
         case Symbol::END:
             return Estado::Fim;
         default:
             return Estado::Erro; // Comando ou label definido fora de seção;
             break;
         }
     }},
    // {Estado::ComandosT, [](Symbol input) {
    //     try
    //     {
    //         return comandos[input].e;
    //     } catch (const std::exception& e)
    //     {
    //         return Estado::Erro;
    //     }
    //     // return input == Symbol::SECAO") || input == Symbol::FIM") ? Estado::Erro : Estado::SecaoD;
    //     // return Estado::Inicial; // TODO: CORRIGIR ESTADO DE RETORNO>
    //     }
    // },
    // {Estado::ComandosD, [](Symbol input) {
    //     if (input == Symbol::SECAO_DADOS == 0) {
    //         return Estado::Fim;
    //     } else {
    //         return Estado::ComandosD;
    //     }
    // }},
    {Estado::Read2, [](Symbol input, const string &token)
     {
         return input == Symbol::ID ? Estado::Read1 : Estado::Erro;
     }},
    {Estado::Read1, [](Symbol input, const string &token)
     {
         switch (input)
         {
         case Symbol::ID:
         case Symbol::NUMBER:
             return Estado::SecaoT;
         default:
             return Estado::Erro;
         }
     }},
    {Estado::LabelT, [](Symbol input, const string &token)
     {
         switch (input)
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
             for (auto l : labelStack)
                 tabelaSimbolo[l] = contador;
             labelStack.clear();
             contador++;
             return Estado::Read1;
         case Symbol::COPY:
             for (auto l : labelStack)
                 tabelaSimbolo[l] = contador;
             labelStack.clear();
             contador++;
             return Estado::Read2;
         case Symbol::LABEL:
             labelStack.push_back(token); // enquanto encontrar labels, empilhá-las pois todas apontam para a próxima referencia;
             return Estado::LabelT;
             // estado que espera estado ou um input e opera na tabela de simbolos;
         case Symbol::STOP:
             for (auto l : labelStack)
                 tabelaSimbolo[l] = contador;
             labelStack.clear();
             contador++;
         default:
             return Estado::Erro;
             break;
         }
         return Estado::Erro;
     }},
    {Estado::LabelD, [](Symbol input, const string &token)
     {
         switch (input)
         {
         case Symbol::LABEL:
             labelStack.push_back(token);
             return Estado::LabelD;
         case Symbol::CONST:
             // popar toda a stack de labels e fazer referencia ao endereço atual.
             popLabelStack();
             contador++;
             return Estado::ReadImmediate;
         case Symbol::SPACE:
             popLabelStack();
             contador++;
             return Estado::SecaoD;
             // tem que cuidar da tabela de definição tb;
         default:
             return Estado::Erro;
             break;
         }
     }},
    {Estado::ReadImmediate, [](Symbol input, const string &token)
     {
         switch (input)
         {
         case Symbol::NUMBER:
             contador++;
             return Estado::SecaoD;
         default:
             return Estado::Erro;
         }
     }},
    {Estado::Erro, [](Symbol input, const string &token)
     { return Estado::Erro; }},
    {Estado::Stop, [](Symbol input, const string &token)
     { return input == Symbol::SECAO_DADOS ? Estado::Erro : Estado::SecaoD; }},
    {Estado::Fim, [](Symbol input, const string &token)
     { return Estado::Fim; }}};

// std::unordered_map<string, int>tabelaErro; // Tabela de erros;

void firstPass(std::vector<std::pair<string, int>> &file)
{
    string linha;

    Estado estado = Estado::Inicial;
    std::vector<std::pair<Symbol, std::string>> tokens;
    // Contar as linhas, marcar ocorrências de labels com o contador;
    // ao verificar SECAO texto atualizar os valores na tabela de simbolos;
    for (int i = 0; i < file.size(); i++)
    {
        linha = file[i].first;
        tokenize(linha, tokens);
    }

    string token;
    Symbol simbolo;
    // verificar erros;
    for (int i = 0; i < tokens.size(); i++)
    {
        simbolo = tokens[i].first;
        token = tokens[i].second;
        std::cout << "token: " << token << " simbolo: " << (int)simbolo << " estado atual:" << estado << " proximo estado: ";
        estado = transicoes[estado](simbolo, token);
        std::cout << estado << "\n";
    }
    // terminei de navegar na maquina de estados;
    //  TODO: popular as tabelas de uso, definição e simbolos;
    //  na teoria tamo contando corretamente os tokens de verdade, falta, navegar no código novamente, e substituir ocorrências de ID's pelos valores nas tabelas;
    //  se um valor não existir, throw error;gi
}

std::vector<string> traduzir(std::vector<std::pair<string, int>> &processed)
{
    std::vector<string> traduzido;
    int i;
    string token;
    string linha;
    firstPass(processed);

    return traduzido;
}
