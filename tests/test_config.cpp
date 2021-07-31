//
// Created by ChaosChen on 2021/5/21.
//


#include <iostream>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>
#include <mocker/config.h>
#include <mocker/log.h>


mocker::ConfigVar<int>::ptr g_int_value_config =
        mocker::Config::Lookup("system.port", (int) 8080, "system port");

// type error
//mocker::ConfigVar<float>::ptr g_int_value_error_config =
//        mocker::Config::Lookup("system.port", (float)8080, "system port");

mocker::ConfigVar<float>::ptr g_float_value_config =
        mocker::Config::Lookup("system.value", (float) 10.12f, "system port");

mocker::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config =
        mocker::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

mocker::ConfigVar<std::list<int>>::ptr g_int_list_value_config =
        mocker::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

mocker::ConfigVar<std::set<int>>::ptr g_int_set_value_config =
        mocker::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");

mocker::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config =
        mocker::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int uset");

mocker::ConfigVar<std::map<std::string, int>>::ptr g_int_map_value_config =
        mocker::Config::Lookup("system.int_map", std::map<std::string, int>{{"a", 2},
                                                                            {"b", 3}}, "system int map");

mocker::ConfigVar<std::unordered_map<std::string, int>>::ptr g_int_umap_value_config =
        mocker::Config::Lookup("system.int_umap", std::unordered_map<std::string, int>{{"a", 2},
                                                                                       {"b", 3}}, "system int umap");


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
    YAML::Node root = YAML::LoadFile("../conf/test.yml");
    print_yaml(root, 0);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << root.Scalar();

}

void test_config() {
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "Before: " << g_int_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "Before: " << g_float_value_config->getValue();
#define XX(var, name, prefix) \
    { \
        for (auto& i : (var)->getValue()) { \
            MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << #prefix " " #name " : " << i; \
        } \
    }

#define XX_M(var, name, prefix) \
    { \
        for (auto& i : (var)->getValue()) { \
            MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << #prefix " " #name " : " << "{" << i.first << " - " << i.second << "}"; \
        } \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_int_map_value_config, int_map, brefore);
    XX_M(g_int_umap_value_config, int_umap, brefore);

    YAML::Node root = YAML::LoadFile("../conf/test.yml");
    mocker::Config::LoadFromYaml(root);

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After: " << g_int_value_config->getValue();
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After: " << g_float_value_config->getValue();
    XX(g_int_vec_value_config, int_vec, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_vec_value_config->toString();
    XX(g_int_list_value_config, int_list, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_list_value_config->toString();
    XX(g_int_set_value_config, int_set, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_set_value_config->toString();
    XX(g_int_uset_value_config, int_uset, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_uset_value_config->toString();
    XX_M(g_int_map_value_config, int_map, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_map_value_config->toString();
    XX_M(g_int_umap_value_config, int_umap, after);
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "After YAML: " << g_int_umap_value_config->toString();
#undef XX
}

class Person {
public:
    std::string m_name = "null";
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
            << " age=" << m_age
            << " sex=" << m_sex
            << "]";
        return ss.str();
    }

    bool operator== (const Person& oth) const {
        return m_name == oth.m_name
               && m_age == oth.m_age
               && m_sex == oth.m_sex;
    }
};

namespace mocker {
    template<>
    class LexicalCast<std::string, Person> {
    public:
        Person operator() (const std::string& v) {
            YAML::Node node = YAML::Load(v);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex= node["sex"].as<bool>();
            return p;
        }
    };

    template<>
    class LexicalCast<Person, std::string> {
    public:
        std::string operator()(const Person &p) {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

mocker::ConfigVar<Person>::ptr g_person =
        mocker::Config::Lookup("class.person", Person(), "system person");

mocker::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
        mocker::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

mocker::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_map_vec =
        mocker::Config::Lookup("class.map_vec", std::map<std::string, std::vector<Person>>(), "system person");

void test_class() {
    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for (auto& i : m) { \
            MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }

    g_person->addListener([](const Person& old_value, const Person& new_value){
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "old_value=" << old_value.toString()
            << " new_value=" << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "before: " << g_person_map_vec->toString();

    YAML::Node root = YAML::LoadFile("../conf/test.yml");
    mocker::Config::LoadFromYaml(root);

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");

    MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "after: " << g_person_map_vec->toString();
#undef XX_PM
}

void test_log() {
    std::cout << mocker::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("../conf/log.yml");
    mocker::Config::LoadFromYaml(root);
    std::cout << "===================================" << std::endl;
    std::cout << mocker::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "===================================" << std::endl;
    MOCKER_LOG_INFO(MOCKER_LOG_NAME("system")) << "Hello system log";
    MOCKER_LOG_NAME("system")->setFormatter("%d - %f - %m%n");
    MOCKER_LOG_WARN(MOCKER_LOG_NAME("system")) << "system formatter changed";
}


int main(int argc, char *argv[]) {
//    test_yaml();
//    test_config();
//    test_class();
    test_log();

    mocker::Config::Visit([](mocker::ConfigVarBase::ptr var) {
        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "name=" << var->getName()
                                           << " description=" << var->getDescription()
                                           << " typename=" << var->getTypeName()
                                           << " value=" << var->toString();
    });
    return 0;
}