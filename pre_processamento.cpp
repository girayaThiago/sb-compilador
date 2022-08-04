#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include "pre_processamento.h"

using std::pair;
using std::string;
std::unordered_map<string, int> equTable;
// label, EQU, IF

string replaceEqu(string linha)
{
    for (auto &[k, v] : equTable)
    {
        int index = linha.find(k);
        if (index == string::npos)
            continue;
        // std::cout << linha << std::endl;
        string expression = " " + k + "\\s| " + k + "$";
        std::regex e(expression);
        string teste = " " + std::to_string(v);
        linha = std::regex_replace(linha, e, teste);
        // std::cout << "replaced \"" << k << "\" ocurrence with value \"" << v << "\":" << linha << "\n";
    }
    return linha;
}

std::vector<std::pair<string, int>> prepocessamento(std::vector<string> &file)
{
    std::vector<std::pair<std::string, int>> processed;
    std::vector<std::pair<std::string, int>> EQUprocessed;

    int i;
    string token;
    string linha;
    // Popula tabela do EQU
    for (i = 0; i < file.size(); i++)
    {
        linha = file[i];
        std::stringstream ss(linha);
        if (linha.find("EQU") != string::npos)
        {
            ss >> token;                                      // label com ':'
            string label = token.substr(0, token.size() - 1); // label sem ':'
            ss >> token;                                      // consome EQU
            ss >> token;                                      // valor do EQU
            try
            {
                int value = std::stoi(token);
                equTable[label] = value;
                // std::cout << "Setting value: " << value << " for label: " << label << "\n";
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n'; // erro ao processar valor do EQU, léxico? Sintático?
            }
            continue;
        }
        EQUprocessed.push_back(std::make_pair(linha, i));
    }

    // Substitui ocorrências de EQU e trata ocorrências de IF
    for (i = 0; i < EQUprocessed.size(); i++)
    {
        string linha = EQUprocessed[i].first;
        std::stringstream ss(linha);
        if (linha.find("IF") != string::npos)
        {
            ss >> token; // consome IF
            ss >> token; // token para checar
            // std::cout << equTable[token] << " = Valor para o token: " << token << "\n";
            if (equTable.find(token) == equTable.end())
            {
                i++;      // passa para a próxima linha
                continue; // já que não tem if ignorar a nova linha (i++);
            }
            else if (equTable[token] == 0)
            {
                i++;
                continue;
            }
            else
                continue; // senão continua e mantém a próxima linha;
        }
        else
        {
            replaceEqu(linha);
        }
        processed.push_back({linha, i}); // adiciona linha para o arquivo pre-processado com referência para o arquivo original.
    }
    return processed; // retorna linhas pré-processadas
}