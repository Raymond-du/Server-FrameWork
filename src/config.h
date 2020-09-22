
/**
 * @file config.h
 * @author raymond-du 
 * @brief 
 * @version 0.1
 * @date 2020-08-01
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>
#include <memory>
#include <yaml-cpp/yaml.h>
//  进行类型之间的转换
#include <boost/lexical_cast.hpp>
#include "Logger.h"
#include "singleton.h"
#include <unordered_map>
#include <unordered_set>
#include "util.h"
#include <functional>

namespace raymond
{
        template<class F,class T>
        class LexicalCast;
        /**
         * @brief 配置变量的基类
         * 
         */
        class ConfigVarBase
        {

        public:
                typedef std::shared_ptr<ConfigVarBase> ptr;
                /**
                 * @brief  构造函数初始化 配置变量の名称
                 * 
                 * @param name 
                 * @param description 
                 */
                ConfigVarBase(const std::string &name, const std::string &description = "")
                    : m_name(name),
                      m_description(description)
                {
                        /**
                         * @brief j transform相当于一个循环 第一个参数是开始位置, 第二个参数结束的位置,地开个结果存放的位置,第四个执行的函数
                         * 
                         */
                        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
                }

                virtual ~ConfigVarBase() {}

                std::string getName() const { return m_name; }
                std::string getDescription() const { return m_description; }

                /**
                 * @brief  转换成字符串
                 * 
                 * @return 
                 */
                virtual std::string toString() = 0;

                /**
                 * @brief 从字符串初始化值
                 * 
                 * @param str 
                 * @return 
                 * @return 
                 */
                virtual bool fromString(const std::string &str) = 0;

                /**
                 * @brief 获取参数类型名
                 * 
                 * @return 
                 */
                virtual std::string getTypeName() = 0;

        protected:
                std::string m_name;
                std::string m_description;
        };

        /**
         * @brief  配置对象和配置信息
         * 
         * @tparam T 
         */
        template <class T>
        class ConfigVar : public ConfigVarBase
        {
        public:
                typedef std::shared_ptr<ConfigVar> ptr;
                typedef std::function<void( const T &,const T &)> on_change_cb;

                ConfigVar(const std::string &name, const T &val, const std::string &description)
                    : ConfigVarBase(name, description),
                      m_val(val) {}

                std::string toString() override
                {
                        try
                        {
                                return LexicalCast<T,std::string>()(m_val);
                        }
                        catch (const std::exception &e)
                        {
                                RAYMOND_RLOG_ERROR("ConfigVar toString() exception %s ,convert:%s toString ",
                                        e.what(),typeToName<T>().c_str());
                        }
                        return "";
                }

                bool fromString(const std::string &str) override
                {
                        try
                        {
                                setValue(LexicalCast<std::string,T>()(str));
                        }
                        catch (const std::exception &e)
                        {
                                RAYMOND_RLOG_ERROR("ConfigVar fromString() exception %s ,convert:%s fromString ",
                                        e.what(),typeToName<T>().c_str());
                                return false;
                        }
                        return true;
                }

                std::string getTypeName() override
                {
                        return typeToName<T>();
                }

                void setValue(const T & val){
                        if(val == m_val){
                                return;
                        }
                        for(auto i : m_changeListener){
                                i.second(m_val,val);
                        }
                        m_val = val;
                }

                const T & getValue()const {return m_val;}

                /**
                 * @brief  添加修改值的listener函数
                 * 
                 * @param callback  listener函数
                 * @return      listener标识 作为删除的listener的key
                 */
                uint32_t addListener(const on_change_cb & callback){
                        //标识 listener的id
                        static uint32_t  s_listener_id = 0;
                        m_changeListener[++s_listener_id] = callback;
                        return s_listener_id;
                }
                /**
                 * @brief 获取回调函数
                 * 
                 * @param key  回调函数标识
                 * @return      回调函数function
                 */
                on_change_cb getListener(uint32_t key){
                        auto it =  m_changeListener.find(key);
                        return (it == m_changeListener.end() ?  nullptr : it->second);
                }
                /**
                 * @brief  清除所有的回调函数
                 * 
                 */
                void clearAllListener(){
                        m_changeListener.clear();
                }
        private:
                T m_val;
                // 变更回调函数组, uint32_t key,要求唯一  ,作为回调函数的标识
                std::map<uint32_t, on_change_cb > m_changeListener;
                
        };
        /**
         * @brief  配置信息
         * 
         */
        class Config
        {

        public:
                typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

                
                static void loadFromConFile(const std::string & path){
                        YAML::Node node = YAML::LoadFile(path);
                        try
                        {
                                loadFromYaml(node);
                        }
                        catch(const std::exception& e)
                        {
                                RAYMOND_RLOG_ERROR("LoadConfFile file failed");
                        }
                        
                }

                /**
                 * @brief 查询配置参数
                 * 
                 * @tparam T  返回参数值的类型
                 * @param name  参数的名字
                 * @return      参数变量的包装 如果没找到 返回NULL
                 */
                template <class T>
                static typename ConfigVar<T>::ptr findConfigVar(const std::string &name)
                {
                        auto it = getAllDatas().find(name);
                        if (it != getAllDatas().end())
                        {
                                return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                        }
                        return NULL;
                }

                /**
                 * @brief 查询name配置
                 *      如果未找到 ,创建一个新的,并返回该新的配置对象
                 * @tparam T    配置数据的类型
                 * @param name  配置的名称
                 * @param defaultValue  配置的默认值
                 * @param description   描述
                 * @return      配置对象
                 */
                template <class T>
                static typename ConfigVar<T>::ptr lookUp(const std::string &name, const T &defaultValue, const std::string &description = "")
                {
                        auto it = getAllDatas().find(name);
                        if (it != getAllDatas().end())
                        {
                                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                                if (tmp)
                                {
                                        return tmp;
                                }
                                else
                                {
                                        RAYMOND_RLOG_ERROR("Lookup name= %s exists but type not %s, real_type= %s  %s",name.c_str(), 
                                                typeToName<T>().c_str(),it->second->getTypeName().c_str() ,  it->second->toString());
                                        return nullptr;
                                }
                        }
                        if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos)
                        {
                                RAYMOND_RLOG_ERROR("Lookup name invalid : " ,name);
                                return nullptr;
                        }
                        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, defaultValue, description));
                        getAllDatas()[name] = v;
                        return v;
                }

                /**
                 * @brief 查询某个配置信息
                 * 
                 * @param name  配置信息的名称
                 * @return  配置信息的指针
                 */
                static ConfigVarBase::ptr findConfigVar(const std::string &name)
                {
                        auto it = getAllDatas().find(name);
                        return it != getAllDatas().end() ? it->second : nullptr;
                }

                /**
                 * @brief i 设置配置配置本地的变量
                 * 
                 * @param root  yaml的根节点
                 */
                static void loadFromYaml(const YAML::Node &root);

                /**
                 * @brief  输出结点的所有成员
                 * 
                 * @param prefix  结点的
                 * @param output  输出所有的借鉴
                 */
                static void listAllMember(const std::string &prefix, const YAML::Node &node,
                                          std::list<std::pair<std::string, const YAML::Node>>& output);

        private:
                //所有的配置变量映射
                static ConfigVarMap &getAllDatas()
                {
                        static ConfigVarMap m_datas;
                        return m_datas;
                }
        };

        /**
         * @brief 类型转换的模板类
         * 
         * @tparam F    原类型
         * @tparam T    转化的类型
         */
        template <class F, class T>
        class LexicalCast
        {
        public:
                T operator()(const F &f)
                {
                        return boost::lexical_cast<T>(f);
                }
        };
        
        /**
         * @brief string 的类型的字符串 转成 yaml::node
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<std::string,YAML::Node>{
        public:
                YAML::Node operator()(const std::string & yamlStr){
                        return YAML::Load(yamlStr);
                }
        };    
        /**
         * @brief yaml::node 的类型的字符串 转成 string
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<YAML::Node,std::string>{
        public:
                std::string operator()(const YAML::Node & node){
                        std::stringstream ss;
                        ss<< node;
                        return ss.str();
                }
        };

	/**
	 * @brief yaml-node转换成T
	 *
	 * @tparam T	泛型
	 */
        template<class T>
        class LexicalCast<YAML::Node,T>
        {
        public:
                T operator()(const YAML::Node & node)
                {
                        std::stringstream ss;
                        ss<<node;
                        return LexicalCast<std::string,T>()(ss.str());
                }
        };

	/**
	 * @brief 泛型T转换成yaml::node
	 *
	 * @tparam T 泛型t
	 */
        template<class T>
        class LexicalCast<T,YAML::Node>
        {
        public:
                YAML::Node operator ()(const T & t){
                        return YAML::Load(boost::lexical_cast<std::string>(t));
                }
        };
        /**
         * @brief string 的类型的字符串 转成 T
         * 
         * @tparam T  模板类型
         */
        template<class T>
        class LexicalCast<T,std::string>{
        public:
                std::string operator ()(const T & val){
                        return LexicalCast<YAML::Node,std::string>()(LexicalCast<T,YAML::Node>()(val));
                }
        };
        /**
         * @brief yaml::node 的类型的字符串 转成 string
         * 
         * @tparam T 模板类型
         */
        template<class T>
        class LexicalCast<std::string,T>{
        public:
                T operator ()(const std::string & str){
                        YAML::Node node = LexicalCast<std::string,YAML::Node>()(str);
                        return LexicalCast<YAML::Node,T>()(node);
                }
        };
        /**
         * @brief yaml 的类型的字符串 转成 vector
         * 
         * @tparam T 
         */
        template <class T>
        class LexicalCast<YAML::Node, std::vector<T>>
        {
        public:
                std::vector<T> operator()(const YAML::Node &node)
                {
                        typename std::vector<T> vec;

                        for (size_t i = 0; i < node.size(); i++)
                        {
                                vec.push_back(LexicalCast<YAML::Node,T>()(node[i]));
                        }

                        return vec;
                }
        };
        /**
         * @brief 将配置变量属性vec 转变成 yaml格式的字符串
         * 
         */
        template <class T>
        class LexicalCast<std::vector<T>, YAML::Node>
        {
        public:
                 YAML::Node operator()(std::vector<T> vec)
                {
                        //生成一个队列的yaml node
                        YAML::Node node(YAML::NodeType::Sequence);
                        for (auto it : vec)
                        {
                                node.push_back(LexicalCast<T,YAML::Node>() (it));
                        }
                        return node;
                }
        };
        /**
         * @brief yaml 的类型的字符串 转成 list
         * 
         * @tparam T 
         */
        template <class T>
        class LexicalCast<YAML::Node, std::list<T>>
        {
        public:
                std::list<T> operator()(const YAML::Node &node)
                {
                        typename std::list<T> list;

                        for (size_t i = 0; i < node.size(); i++)
                        {
                                list.push_back(LexicalCast<YAML::Node,T>(node[i]));
                        }

                        return list;
                }
        };
        /**
         * @brief 将配置变量属性list 转变成 yaml格式的字符串
         * 
         */
        template <class T>
        class LexicalCast<std::list<T>, YAML::Node>
        {
        public:
                YAML::Node operator()(std::list<T> list)
                {
                        //生成一个队列的yaml node
                        YAML::Node node(YAML::NodeType::Sequence);
                        for (auto it : list)
                        {
                                node.push_back(LexicalCast<T,YAML::Node>() (it));
                        }
                        return node;
                }
        };

        /**
         * @brief yaml 的类型的字符串 转成 set
         * 
         * @tparam T 
         */
        template <class T>
        class LexicalCast<YAML::Node, std::set<T>>
        {
        public:
                std::set<T> operator()(const YAML::Node &node)
                {
                        typename std::set<T> set;

                        for (size_t i = 0; i < node.size(); i++)
                        {
                                set.insert(LexicalCast<YAML::Node,T>()(node[i]));
                        }

                        return set;
                }
        };
        /**
         * @brief 将配置变量属性set 转变成 yaml格式的字符串
         * 
         */
        template <class T>
        class LexicalCast<std::set<T>, YAML::Node>
        {
        public:
                YAML::Node operator()(const std::set<T> & set)
                {
                        //生成一个队列的yaml node
                        YAML::Node node(YAML::NodeType::Sequence);
                        for (auto& it : set)
                        {
                                node.push_back(LexicalCast<T,YAML::Node>() (it));
                        }
                        return node;
                }
        };

        /**
         * @brief yaml格式的字符串转变成 unordered_set 
         * 
         */
        template<class T>
        class LexicalCast<YAML::Node,std::unordered_set<T> >{
        public :
                std::unordered_set<T> operator ()(const YAML::Node & node){
                        typename std::unordered_set<T> set;

                        for(size_t i = 0;i<node.size(); ++i ){
                                set.insert(LexicalCast<YAML::Node,T>()(node[i]));
                        }
                        return set;
                }
        };
        /**
         * @brief unordered_set转变成  yaml格式的字符串
         * 
         */
        template<class T>
        class LexicalCast<std::unordered_set<T>,YAML::Node>{
        public:
                YAML::Node operator()(std::unordered_set<T> set){
                        YAML::Node node(YAML::NodeType::Sequence);
                        for(auto& it : set){
                                node.push_back(LexicalCast<T,YAML::Node>()(it));
                        }
                        return node;
                }
        };

        /**
         * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
         */
        template <class T>
        class LexicalCast<YAML::Node, std::map<std::string, T>>
        {
        public:
                std::map<std::string, T> operator()(const YAML::Node &node)
                {
                        typename std::map<std::string, T> map;

                        for (auto it = node.begin();  it != node.end(); ++it)
                        {
                                map.insert(std::make_pair(it->first.Scalar(),
                                                          LexicalCast<YAML::Node, T>()(it->second)));
                        }
                        return map;
                }
        };

        /**
         * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
         */
        template <class T>
        class LexicalCast<std::map<std::string, T>, YAML::Node>
        {
        public:
                YAML::Node operator()(const std::map<std::string, T> &map)
                {
                        YAML::Node node(YAML::NodeType::Map);
                        for (auto &i : map)
                        {
                                node[i.first] = LexicalCast<T, YAML::Node>()(i.second);
                        }
                        return node;
                }
        };

        /**
         * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
         */
        template <class T>
        class LexicalCast<YAML::Node, std::unordered_map<std::string, T>>
        {
        public:
                std::unordered_map<std::string, T> operator()(const YAML::Node &node)
                {
                        typename std::unordered_map<std::string, T> vec;
                        for (auto it = node.begin();
                             it != node.end(); ++it)
                        {
                                vec.insert(std::make_pair(it->first.Scalar(),
                                                          LexicalCast<YAML::Node, T>()(it->second)));
                        }
                        return vec;
                }
        };

        /**
         * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
         */
        template <class T>
        class LexicalCast<std::unordered_map<std::string, T>, YAML::Node>
        {
        public:
                YAML::Node operator()(const std::unordered_map<std::string, T> &v)
                {
                        YAML::Node node(YAML::NodeType::Map);
                        for (auto &i : v)
                        {
                                node[i.first] = LexicalCast<T, YAML::Node>()(i.second);
                        }
                        return node;
                }
        };
} // namespace raymond
#endif // __CONFIG_H
