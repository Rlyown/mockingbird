//
// Created by ChaosChen on 2021/5/21.
//

#include <list>
#include <mocker/config.h>
#include <mocker/log.h>

namespace mocker {

    ////////////////////////////////////////////////////////////////////
    /// Config
    ///////////////////////////////////////////////////////////////////
    ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
        RWMutexType::ReadLock lock(GetRWMutex());
        auto it = GetData().find(name);
        return it == GetData().end() ? nullptr : it->second;
    }

    static void ListAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node>>& output) {


        if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            MOCKER_LOG_ERROR(MOCKER_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }

        output.push_back(std::make_pair(prefix, node));

        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(),
                              it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node &root) {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);

        for (auto& i : all_nodes) {
            std::string key = i.first;
            if (key.empty()) {
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            if (var) {
                if (i.second.IsScalar()) {
                    var->fromString(i.second.Scalar());
                } else {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    void Config::Visit(const std::function<void(ConfigVarBase::ptr)>& cb) {
        RWMutexType::ReadLock lock(GetRWMutex());
        ConfigVarMap& m = GetData();

        for (auto & it : m) {
            cb(it.second);
        }
    }

}
