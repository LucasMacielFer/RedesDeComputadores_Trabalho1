#include "client/client.h"
#include <cstdio>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

/*
    Universidade Tecnológica Federal do Paraná - UTFPR
    Departamento Acadêmico de Informática - DAINF
    ICSR30 - Redes de Computadores

    Trabalho de Programação 1

    Lucas Maciel Ferreira - 2556596
*/ 

namespace
{
    struct RequestUrl
    {
        std::string host;
        uint16_t port;
        std::string filename;
    };

    std::optional<RequestUrl> parseRequestUrl(const std::string& text)
    {
        if (text.empty() || text[0] != '@')
            return std::nullopt;

        size_t hostStart = 1;
        size_t hostEnd;
        size_t colonPos;

        if (hostStart < text.size() && text[hostStart] == '[')
        {
            const size_t closeBracket = text.find(']', hostStart);
            if (closeBracket == std::string::npos || closeBracket + 1 >= text.size() || text[closeBracket + 1] != ':')
                return std::nullopt;

            hostEnd = closeBracket;
            colonPos = closeBracket + 1;
            ++hostStart;
        }
        else
        {
            colonPos = text.find(':', hostStart);
            if (colonPos == std::string::npos)
                return std::nullopt;

            hostEnd = colonPos;
        }

        const size_t slashPos = text.find('/', colonPos);
        if (slashPos == std::string::npos)
            return std::nullopt;

        const std::string portStr = text.substr(colonPos + 1, slashPos - colonPos - 1);
        if (portStr.empty())
            return std::nullopt;

        int portValue = 0;
        try
        {
            portValue = std::stoi(portStr);
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }

        if (portValue <= 0 || portValue > 65535)
            return std::nullopt;

        RequestUrl url;
        url.host = text.substr(hostStart, hostEnd - hostStart);
        url.port = static_cast<uint16_t>(portValue);
        url.filename = text.substr(slashPos + 1);

        if (url.host.empty() || url.filename.empty())
            return std::nullopt;

        return url;
    }

    bool parseAddress(const std::string& host, Network::Endpoint& endpoint)
    {
        endpoint = {};

        if (inet_pton(AF_INET, host.c_str(), endpoint.ip) == 1)
        {
            endpoint.isIpv6 = false;
            return true;
        }

        if (inet_pton(AF_INET6, host.c_str(), endpoint.ip) == 1)
        {
            endpoint.isIpv6 = true;
            return true;
        }

        return false;
    }

    std::vector<std::string> tokenize(const std::string& line)
    {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
            tokens.push_back(token);
        return tokens;
    }

    void handleRequestLine(const std::vector<std::string>& tokens)
    {
        const auto url = parseRequestUrl(tokens[0]);
        if (!url)
        {
            std::cerr << "[CLIENT] Formato invalido. Use: @IP:PORTA/arquivo_remoto ou @[IPv6]:PORTA/arquivo_remoto" << std::endl;
            return;
        }

        bool simulateCorruption = false;
        bool simulatePacketLoss = false;
        std::string localPath;

        for (int i = 1; i < tokens.size(); ++i)
        {
            if (tokens[i] == "-simulate-corruption")
                simulateCorruption = true;
            else if (tokens[i] == "-simulate-packet-loss")
                simulatePacketLoss = true;
            else
                localPath = tokens[i];
        }

        if (localPath.empty())
            localPath = url->filename;

        Network::Endpoint serverEndpoint{};
        if (!parseAddress(url->host, serverEndpoint))
        {
            std::cerr << "[CLIENT] Endereco invalido: " << url->host << std::endl;
            return;
        }
        serverEndpoint.port = url->port;

        FileTransfer::Client client(serverEndpoint, simulatePacketLoss, simulateCorruption);
        client.requestFile(url->filename, localPath);
    }
}

int main()
{
    std::cout << "Cliente de transferencia de arquivos." << std::endl;
    std::cout << "Uso: @IP:PORTA/arquivo_remoto ou @[IPv6]:PORTA/arquivo_remoto [arquivo_local] [-simulate-corruption] [-simulate-packet-loss]" << std::endl;
    std::cout << "Digite \"sair\" para encerrar." << std::endl;

    std::string line;
    std::cout << "> " << std::flush;

    while (std::getline(std::cin, line))
    {
        const std::vector<std::string> tokens = tokenize(line);

        if (tokens.empty())
        {
            std::cout << "> " << std::flush;
            continue;
        }

        if (tokens[0] == "sair" || tokens[0] == "exit" || tokens[0] == "quit")
            break;

        handleRequestLine(tokens);

        std::cout << "> " << std::flush;
    }

    return 0;
}
