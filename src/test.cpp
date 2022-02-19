
#include "Logger.h"
#include "config.h"
#include <memory>
#include <cxxabi.h>
#include <stdarg.h>
#include "util.h"
#include "macro.h"

using namespace std;

class Person{
public:
        string name;
        int age;
        bool sex;      

        string toString(){
                stringstream ss;
                ss<<"name : "<< name << "age : "<< age << "sex : "<<sex <<endl;
                return ss.str();
        }
        bool operator==(const Person& oth) const {
                return name == oth.name
                && age == oth.age
                && sex == oth.sex;
        }
};
namespace raymond{

        template<>
        class LexicalCast<YAML::Node,Person> {
        public:
                Person operator() (const YAML::Node & node){
                        
                        Person p;
                        p.name = node["name"].as<string>();
                        p.age = node["age"].as<int> ();
                        p.sex = node["sex"].as<bool>();
                        return p;
                }
        };
        template<>
        class LexicalCast<Person, YAML::Node> {
        public:
                YAML::Node operator()(const Person& p) {
                        YAML::Node node;
                        node["name"] = p.name;
                        node["age"] = p.age;
                        node["sex"] = p.sex;
                        return node;
                }
        };
}

raymond::ConfigVar<Person>::ptr con_per = raymond::Config::lookUp("person",Person(),"person info");
raymond::ConfigVar<std::vector<Person> >::ptr con_vec= raymond::Config::lookUp("persons",std::vector<Person>(),"persons info");

/**
int main(){
        using namespace raymond;

        // LogEvent event;
        // event.setContent("nihk").setTime(0).setLine(2);
        // LogEvent e = event.setContent("sad");

        // cout<<&event;
        // string type = typeToName<double>();
        // student s;
        // s.name = "nihk";
        // s.id =10 ;
        // try
        // {
        //         cout<<boost::lexical_cast<string>(s)<<endl;
        // }
        // catch(const std::exception& e)
        // {
        //         std::cerr << e.what() << '\n';
        // }
        // student s1 = boost::lexical_cast<student>("nihk@10");

        // YAML::Node node;
        // node.push_back("test");
        // YAML::Node node1;
        // node1.push_back("raymond-du");
        // node.push_back(node1);

        // ofstream ofs("test.txt");
        // ofs<<node<<endl;

        Config::loadFromConFile("test.txt");

        RAYMOND_RLOG_DEBUG("test_debug");
        RAYMOND_LOG_BYNAME("system")->debug("nihk",__LOGPARAMS__);

        cout<<LexicalCast<YAML::Node,std::string>()(RAYMOND_LOGMANAGE().toYamlNode());


        // auto p = con_vec->getValue();
        // cout<<p[0].name<<endl;

        // std::map<std::string,int> map;
        // // map["nihk"] = 100;
        // // map["buhk"] = 200;
        // ifstream is("test.txt");
        // stringstream ss;
        // ss << is.rdbuf();
        // string s = ss.str();
        // map = LexicalCast<string,std::map<std::string,int>>()(ss.str());

        // cout<<""<<endl;
        // cout<<i->toString()<<endl;
        // RAYMOND_LOG_FMT_ERROR(RAYMOND_LOG_ROOT(),"name : %s age:%d ","raymond ",10);
        // const char * name = LogLevel::toString( LogLevel::Level::DEBUG);
        // cout<< name;
}

*/

int main(){
        //RAYMOND_RLOG_ERROR(raymond::backTraceToString().c_str());
        RAYMOND_ASSERT(0);
}