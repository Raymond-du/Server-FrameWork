
#include "config.h"

namespace raymond{

        void Config::listAllMember(const std::string & prefix ,const YAML::Node & node,
                                                                std::list<std::pair<std::string,const YAML::Node>> & output){
                if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos && !prefix.empty()){
                        RAYMOND_RLOG_ERROR("Config invalid name: %s : ", prefix.c_str());
                        return ;
                }
                output.push_back(std::make_pair(prefix,node));
                /**
                 *      A
                 *              a = raymond
                 *      prefix 转换为 A.a          
                 * */
                if(node.IsMap()){
                        for(auto it = node.begin();it != node.end(); ++it){
                                listAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(),it->second,output);
                        }
                }
        }

        void Config::loadFromYaml(const YAML::Node & root){
                std::list<std::pair<std::string,const YAML::Node>> all_Nodes;
                listAllMember("",root,all_Nodes);

                for(auto it : all_Nodes){
                        std::string key = it.first;
                        if(key.empty() ){
                                continue;
                        }
                        //将key 小写
                        std::transform(key.begin(),key.end(),key.begin(),::tolower);
                        //获取 ConfigVar的指针
                        ConfigVarBase::ptr var = findConfigVar(key);
                        if(var){
                                if(it.second.IsScalar()){
                                        var->fromString(it.second.Scalar());
                                }else{
                                        std::stringstream ss;
                                        ss<<it.second;
                                        var->fromString(ss.str());
                                }
                        }
                        
                }
        }
}

