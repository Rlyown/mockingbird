//
// Created by ChaosChen on 2021/5/21.
//

#ifndef MOCKER_CONFIG_H
#define MOCKER_CONFIG_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <mocker/log.h>
#include <mocker/mutex.h>

namespace mocker {
    ////////////////////////////////////////////////////////////////////
    /// ConfigVarBase
    ////////////////////////////////////////////////////////////////////
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        ConfigVarBase(const std::string &name, const std::string &description = "")
                : m_name(name),
                  m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }

        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;

        virtual bool fromString(const std::string &val) = 0;

        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };


    ////////////////////////////////////////////////////////////////////
    /// type cast
    ////////////////////////////////////////////////////////////////////
    /**
     * Normal type cast
     * @tparam F from_type
     * @tparam T to_type
     */
    template<class F, class T>
    class LexicalCast {
    public:
        T operator()(const F &v) {
            return boost::lexical_cast<T>(v);
        }
    };


    /**
     * For str cast to vector
     * @tparam T vector inner type
     */
    template<class T>
    class LexicalCast<std::string, std::vector<T>> {
    public:
        std::vector<T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::vector<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };


    /**
     * vector cast to str
     * @tparam T vector inner type
     */
    template<class T>
    class LexicalCast<std::vector<T>, std::string> {
    public:
        std::string operator()(const std::vector<T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
     * str cast to list
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::string, std::list<T>> {
    public:
        std::list<T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::list<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };


    /**
     * list cast to str
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::list<T>, std::string> {
    public:
        std::string operator()(const std::list<T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
     * str cast to set
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::string, std::set<T>> {
    public:
        std::set<T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::set<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };


    /**
     * set cast to str
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::set<T>, std::string> {
    public:
        std::string operator()(const std::set<T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
     * str cast to unordered_set
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::string, std::unordered_set<T>> {
    public:
        std::unordered_set<T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::unordered_set<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };


    /**
     * unordered_set cast to str
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string> {
    public:
        std::string operator()(const std::unordered_set<T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
     * str cast to unordered_set
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::string, std::map<std::string, T>> {
    public:
        std::map<std::string, T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::map<std::string, T> vec;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };


    /**
     * unordered_set cast to str
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::map<std::string, T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    /**
     * str cast to unordered_set
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>> {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            typename std::unordered_map<std::string, T> vec;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };


    /**
     * unordered_set cast to str
     * @tparam T
     */
    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &v) {
            YAML::Node node;
            for (auto &i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    ////////////////////////////////////////////////////////////////////
    /// ConfigVar
    ////////////////////////////////////////////////////////////////////

    // FromStr T operator() (const std::string&)
    // FromStr std::string operator() (T&)
    template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        typedef std::function<void(const T &old_value, const T &new_value)> on_change_cb;
        typedef RWMutex RWMutexType;

        ConfigVar(const std::string &name, const T &default_value, const std::string &description = "")
                : ConfigVarBase(name, description),
                  m_val(default_value) {

        }

        std::string toString() override {
            try {
                // return boost::lexical_cast<std::string>(m_val);
                return ToStr()(getValue());
            } catch (std::exception &e) {
                MOCKER_LOG_ERROR(MOCKER_LOG_ROOT()) << "ConfigVar::toString exception"
                                                    << e.what() << " convert: " << typeid(m_val).name() << " to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override {
            try {
                // m_val = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
                return true;
            } catch (std::exception &e) {
                MOCKER_LOG_ERROR(MOCKER_LOG_ROOT()) << "ConfigVar::fromString exception"
                                                    << e.what() << " convert: string to " << typeid(m_val).name();
                return false;
            }
        }

        T getValue() {
            RWMutexType::ReadLock lock(m_rwmutex);
            return m_val;
        }

        void setValue(const T &v) {
            {
                RWMutexType::ReadLock lock(m_rwmutex);
                if (v == m_val) {
                    return;
                }
                for (auto &callback : m_cbs) {
                    callback.second(m_val, v);
                }
            }
            RWMutexType::WriteLock lock(m_rwmutex);
            m_val = v;
        }

        std::string getTypeName() const override { return typeid(T).name(); }

        uint64_t addListener(on_change_cb cb) {
            static uint64_t s_fun_id = 0;
            RWMutexType::WriteLock lock(m_rwmutex);
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }

        void delListener(uint64_t key) {
            RWMutexType::WriteLock lock(m_rwmutex);
            m_cbs.erase(key);
        }

        on_change_cb getListener(uint64_t key) {
            RWMutexType::ReadLock lock(m_rwmutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

        void clearListener() {
            RWMutexType::WriteLock lock(m_rwmutex);
            m_cbs.clear();
        }

    private:
        T m_val;
        // Change the callback function group, the uint64_t key must be
        // unique, generally hash can be used
        std::map<uint64_t, on_change_cb> m_cbs;

        RWMutexType m_rwmutex;
    };

    ////////////////////////////////////////////////////////////////////
    /// Config
    ////////////////////////////////////////////////////////////////////
    class Config {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
        typedef RWMutex RWMutexType;

        /**
         * Look up a config variable. If it doesn't exist, lookup will create it.
         * @tparam T
         * @param name
         * @param default_value
         * @param description
         * @return
         */
        template<class T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name,
                                                 const T &default_value,
                                                 const std::string &description = "") {
            {
                RWMutexType::ReadLock lock(getRWMutex());
                auto it = getData().find(name);
                if (it != getData().end()) {
                    auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                    if (tmp) {
                        MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "Lookup name=" << name << " exists";
                        return tmp;
                    } else {
                        MOCKER_LOG_ERROR(MOCKER_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                                                            << typeid(T).name() << " real_type="
                                                            << it->second->getTypeName()
                                                            << " " << it->second->toString();
                        return nullptr;
                    }
                }
            }
            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
                MOCKER_LOG_ERROR(MOCKER_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            {
                RWMutexType::WriteLock lock(getRWMutex());
                getData()[name] = v;
            }
            return v;
        }

        /**
         * Just look up the config variable, without creating it.
         * @tparam T
         * @param name
         * @return
         */
        template<class T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name) {
            RWMutexType::ReadLock lock(getRWMutex());
            auto it = getData().find(name);
            if (it == getData().end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        static void loadFromYaml(const YAML::Node &root);
        static ConfigVarBase::ptr lookupBase(const std::string &name);

        static void visit(const std::function<void(ConfigVarBase::ptr)>& cb);

    private:
        /**
         * The reason why the static member s_data is not directly defined
         * here is because the order of initialization cannot be guaranteed
         * when the memory is initialized. Therefore, when other static
         * members call lookup for initialization, there may be cases where
         * s_data has not been initialized. So use a static function to
         * encapsulate to ensure that s_data has been initialized when calling.
         * @return s_data
         */
        static ConfigVarMap &getData() {
            static ConfigVarMap s_data;
            return s_data;
        }

        static RWMutexType &getRWMutex() {
            static RWMutexType s_rwmutex;
            return s_rwmutex;
        }

    };
}

#endif //MOCKER_CONFIG_H
