#include <nlohmann/json.hpp>
#include <unordered_map>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string>
#include <pwd.h>

using json = nlohmann::json;

struct Server
{
    std::string host;
};

struct Organization
{
    std::string keyFile;
    std::string bastion;
    std::unordered_map<std::string, Server> servers;
};

const std::string getConfigFile();
const std::string getLoginCmd(const std::string &key, const std::string &bastion, const std::string &server);

const std::string getConfigFile()
{
    std::ostringstream oss;

    struct passwd *pw = getpwuid(getuid());

    oss << pw->pw_dir << "/.config/c/config.json";

    return oss.str();
}

const std::string getLoginCmd(const std::string &key, const std::string &bastion, const std::string &server)
{
    std::ostringstream oss;

    oss << "#!/bin/bash\n";
    oss << "eval $(ssh-agent -s) > /dev/null\n";
    oss << "ssh-add \"" << key << "\" > /dev/null 2>&1\n";
    oss << "ssh -At " << bastion << " ssh \"" << server << "\"\n";

    return oss.str();
}

int main(int argc, char **argv)
{
    auto configFile = getConfigFile();

    std::ifstream is(configFile);

    if (!is.good()) {
        std::cerr << argv[0] << ": Cannot load config file " << configFile << std::endl;
        return 1;
    }

    std::unordered_map<std::string, Organization> map;

    auto json = json::parse(is);

    auto &organizations = json["organizations"];

    if (organizations.is_null()) {
        std::cerr << argv[0] << ": \"organizations\" must be set in config file." << std::endl;
        return 1;
    }

    for (auto &orig : organizations.get<json::object_t>()) {
        auto &keyFile = orig.second["key"];
        auto &bastion = orig.second["bastion"];
        auto &servers = orig.second["servers"];

        if (keyFile.is_null() || !keyFile.is_string()) {
            std::cerr << argv[0] << ": \"key\" must be set in " << orig.first << std::endl;
            return 1;
        }

        if (bastion.is_null() || !bastion.is_string()) {
            std::cerr << argv[0] << ": \"bastion\" must be set in " << orig.first << std::endl;
            return 1;
        }

        if (servers.is_null() || !servers.is_object()) {
            std::cerr << argv[0] << ": \"servers\" must be set in " << orig.first << "." << std::endl;
            return 1;
        }

        Organization organization;
        organization.keyFile = keyFile;
        organization.bastion = bastion;

        for (auto &s : servers.get<json::object_t>()) {
            Server server;
            server.host = s.second;
            organization.servers.insert({s.first, server});
        }

        map.insert({orig.first, organization});
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <orginization> <server>" << std::endl;
        for (auto &[k, o] : map) {
            for (auto &[j, s] : o.servers) {
                std::cout << "\t\t" << k << "\t\t" << j << std::endl;
            }
            std::cout << "\t\t";
            for (int i = 0; i < w.ws_col - (8 * 4); ++i) {
                std::cout << "-";
            }
            std::cout << std::endl;
        }
        return 1;
    }

    auto organization = map.find(argv[1]);

    if (organization == map.end()) {
        std::cerr << argv[0] << ": organization \"" << argv[1] << "\" not found." << std::endl;
        return 1;
    }

    auto server = organization->second.servers.find(argv[2]);

    if (server == organization->second.servers.end()) {
        std::cerr << argv[0] << ": server \"" << argv[2] << "\" not found in \"" << organization->first << "\" organization." << std::endl;
        return 1;
    }

    system(getLoginCmd(organization->second.keyFile, organization->second.bastion, server->second.host).c_str());

    return 0;
}