#include <iostream>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <vector>
#include "tradutor.h"
#include "pre_processamento.h"

using std::string;
/*
    0           1       2                                 4
    Start -> Seção -> Texto -> Comandos     -> Seção -> Dados -> comandos -> fim
                            -> Comandos*    -> Fim
    OPCODE  Num     Tamanho     Ação
    ADD     01      2           ACC ß ACC + mem(OP)
    SUB     02      2           ACC ß ACC - mem(OP)
    MUL     03      2           ACC ß ACC × mem(OP)
    DIV     04      2           ACC ß ACC ÷ mem(OP)
    JMP     05      2           PC ß OP
    JMPN    06      2           Se ACC<0 então PC ß OP
    JMPP    07      2           Se ACC>0 então PC ß OP
    JMPZ    08      2           Se ACC=0 então PC ß OP
    LOAD    10      2           ACC ß mem(OP)
    STORE   11      2           mem(OP) ß ACC
    INPUT   12      2           mem(OP) ß entrada
    OUTPUT  13      2           saída ß mem(OP)
    COPY    09      3           mem(OP2) ß mem(OP1)
    STOP    14      1           Suspende a execução
*/

// std::unordered_map<string, void(*)(const string&)> mdt;

string toUpper(string s)
{
    string upper = "";
    for (int i = 0; i < s.size(); i++)
    {
        char c = s[i];
        if (c >= 'a' && c <= 'z')
            c -= 32;
        upper += c;
    }
    return upper;
}

std::map<string, string> flags = {
    {"preprocess", "(-p|-P)"},
    {"translate", "(-o|-O)"},
    {"file", "([a-zA-Z0-9_]*.asm)"}

};

int main(int argc, char const *argv[])
{
    std::vector<string> file;
    Estado e = Estado::Inicial;
    char const *flag = nullptr;
    char const *input = nullptr;
    char const *output = nullptr;
    if (argc >= 4)
    {
        flag = argv[1];
        input = argv[2];
        output = argv[3];
    }
    if (!flag || !input || !output)
    {
        throw std::logic_error("Você deve informar a flag e os arquivos de saída -o|-p input.asm output.asm");
    }
    std::ifstream fs(input);
    // lê o arquivo todo e armazena num array, linha a linha;
    //  TODO: ler uma label e a linha de uma só vez, bem como ler uma label e o código que pode estar na próxima linha;
    if (fs.good())
    {
        string linha;
        string palavra;
        int contador_linha = 0;
        int contador_posicao = 0;
        while (std::getline(fs, linha))
        {
            contador_linha++;
            linha = toUpper(linha);
            file.push_back(linha);
        }
    }
    else
    {
        std::cout << "Erro ao abrir o arquivo\n";
        return -1;
    }
    std::vector<std::pair<string, int>> processed = prepocessamento(file);
    if (flag[1] == 'p')
    {
        std::ofstream myfile;
        myfile.open(output, std::ios::trunc);
        for (auto par : processed)
        {
            myfile << par.first << "\n"; // realmente faltava pouco, mas as tabelas ainda estão meio incompletas
        }
    }
    else
    {
        std::vector<string> traduzido = traduzir(processed);

        std::ofstream myfile;
        myfile.open(output);
        std::stringstream ss;
        for (auto num : traduzido)
        {
            ss << num << " "; // realmente faltava pouco, mas as tabelas ainda estão meio incompletas
        }
        // std::cout << ss.str();
        myfile << ss.str();
    }
    // for (auto l : processed)
    // {
    //     std::cout << l.first << " linha no arquivo original = " << l.second << "\n";
    // }
    return 0;
}
