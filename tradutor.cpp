#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <map>

#include "tradutor.h"
using std::string;


std::unordered_map<string, int>tabelaSimbolo; // Tabela de simbolos;
std::unordered_map<string, int>tabelaUso; // Tabela de uso; // extern
std::unordered_map<string, int>tabelaDefinicao; // Tabela de uso; // public 

std::pair<Symbol, std::string> getNextToken(string& str) {
    std::smatch m;
    for (auto& [k, v] : patterns) {

        std::regex e(v);

        while (std::regex_search(str, m, e)) {
            // std::cout << m[0].str() << "\n";
            for (auto x : m) {
                if (x.matched) {
                    // std::cout << v << " " << x << "\n";
                    string r = x.str();
                    std::pair<Symbol, string> kr = { k,r };
                    str = m.suffix().str();

                    return kr;
                }
            }
        }

        // std::cout << v << " unmatched " << "\n";
    }
    throw std::logic_error("No match during tokenization");
}
void tokenize(std::string str, std::vector<std::pair<Symbol, string>>& tokens) {
    while (!str.empty()) {
        auto token = getNextToken(str);
        tokens.push_back(token);
    }
}

std::ostream& operator<< (std::ostream& o, Estado e) {
    return o << statenames[e];
}

/// 3 arguemtnos [-p||-o], in, out
/*
    0           1       2                                 4
    Start -> Seção -> Texto -> Comandos     -> Seção -> Dados -> comandos -> fim
                            -> Comandos*    -> Fim
*/
std::unordered_map < Estado, std::function<Estado(Symbol, const string&)>> transicoes = {
    {Estado::Inicial, [](Symbol input, const string&) {
        switch (input) {
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

    {Estado::SecaoT, [](Symbol input, const string& token) {

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
            return Estado::Read1;
            break;
        case Symbol::COPY:
            return Estado::Read2;
            break;
        case Symbol::LABEL:

            // estado que espera estado ou um input e opera na tabela de simbolos;
            break;
        case Symbol::STOP:
        default:
            break;
        }
        return Estado::Erro;
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
        {Estado::Read2, [](Symbol input, const string& token) {
                return input == Symbol::ID ? Estado::Read1 : Estado::Erro;
            }
        },
        {Estado::Read1, [](Symbol input, const string& token) {
            switch (input) {
                case Symbol::ID:
                case Symbol::NUMBER:
                    return Estado::SecaoT;
                default:
                    return Estado::Erro;
            }

            }
        },
        {Estado::Erro, [](Symbol input, const string& token) { return Estado::Erro; }},
        {Estado::Stop, [](Symbol input, const string& token) { return input == Symbol::SECAO_DADOS ? Estado::Erro : Estado::SecaoD; }},
        {Estado::Fim, [](Symbol input, const string& token) { return Estado::Fim; }}
};

// std::unordered_map<string, int>tabelaErro; // Tabela de erros;

void firstPass(std::vector<std::pair<string, int>>& file) {
    string linha;

    Estado estado = Estado::Inicial;
    std::vector<std::pair<Symbol, std::string>> tokens;
    // Contar as linhas, marcar ocorrências de labels com o contador;
    // ao verificar SECAO texto atualizar os valores na tabela de simbolos;
    for (int i = 0; i < file.size(); i++) {
        linha = file[i].first;
        std::stringstream ss(linha);
        tokenize(linha, tokens);
    }

    string token;
    Symbol simbolo;
    // verificar erros;
    for (int i = 0; i < tokens.size(); i++) {
        simbolo = tokens[i].first;
        token = tokens[i].second;
        std::cout << "token: " << token << " simbolo: " << (int)simbolo << " estado atual:" << estado << " proximo estado: ";
        estado = transicoes[estado](simbolo, token);
        std::cout << estado << "\n";
        // if (comandos.find(simbolo) != comandos.end()) {

        // }
    }
}



std::vector<string> traduzir(std::vector<std::pair<string, int>>& processed) {
    std::vector<string> traduzido;
    int i;
    string token;
    string linha;
    firstPass(processed);

    return traduzido;
}

