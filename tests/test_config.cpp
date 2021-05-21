//
// Created by ChaosChen on 2021/5/21.
//


#include <yaml-cpp/yaml.h>
#include "mocker/config.h"
#include "mocker/log.h"

mocker::ConfigVar<int>::ptr g_int_value_config =
        mocker::Config::lookup("system.port", (int)8080, "system port");

mocker::ConfigVar<float>::ptr g_float_value_config =
        mocker::Config::lookup("system.value", (float)10.12f, "system port");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()){
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << std::string(level * 4, ' ') <<  "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    print_yaml(root, 0);
//    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << root;

}

int main(int argc, char *argv[]) {
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << g_int_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << g_int_value_config->toString();

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << g_float_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << g_float_value_config->toString();

    test_yaml();
    return 0;
}