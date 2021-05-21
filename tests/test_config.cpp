//
// Created by ChaosChen on 2021/5/21.
//


#include <vector>
#include <yaml-cpp/yaml.h>
#include <mocker/config.h>
#include <mocker/log.h>

mocker::ConfigVar<int>::ptr g_int_value_config =
        mocker::Config::lookup("system.port", (int)8080, "system port");

mocker::ConfigVar<float>::ptr g_float_value_config =
        mocker::Config::lookup("system.value", (float)10.12f, "system port");

mocker::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config =
        mocker::Config::lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

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
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << root.Scalar();

}

void test_config() {
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "Before: " << g_int_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "Before: " << g_float_value_config->getValue();
    for (auto& i : g_int_vec_value_config->getValue()) {
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "before int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    mocker::Config::loadFromYaml(root);

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After: " << g_int_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After: " << g_float_value_config->getValue();
    for (auto& i : g_int_vec_value_config->getValue()) {
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "before int_vec: " << i;
    }
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << g_int_vec_value_config->toString();
}

int main(int argc, char *argv[]) {
    test_config();

    return 0;
}