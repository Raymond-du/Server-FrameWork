#include "Logger.h"
#include "config.h"

namespace raymond{

        const char * LogLevel::toString(LogLevel::Level level){
                switch (level)
                {
                #define XX(name)  \
                        case LogLevel::name: \
                                return #name;
                // case LogLevel::DEBUG:
                //         return "DEBUG";
                XX(DEBUG);
                XX(INFO);
                XX(WARN);
                XX(ERROR);
                XX(FATAL);
                #undef XX 
                default:
                        return "UNKNOW";
                }
        }

        LogLevel::Level LogLevel::toLevel(const std::string & str){
                std::string tmp = str;
                std::map<std::string,LogLevel::Level> levelMap = {
                #define XX(str) \
                        {#str,LogLevel::Level::str}
                         XX(DEBUG),
                        XX(INFO),
                        XX(WARN),
                        XX(ERROR),
                        XX(FATAL),
                #undef XX
                };
                std::transform(tmp.begin(),tmp.end(),tmp.begin(),::toupper);
                auto it = levelMap.find(tmp);
                if(it == levelMap.end()){
                        return LogLevel::Level::NOKNOW;
                }
                return it->second;
        }

        void Logger::addAppender(LogAppender::ptr appender){
                std::unique_lock<std::mutex> lock(m_mutex); 
                m_appenders.push_back(appender);
        }

        void Logger::delAppender(LogAppender::ptr appender){
                std::unique_lock<std::mutex> lock(m_mutex); 
                for(auto it = m_appenders.begin();it != m_appenders.end(); ++it){
                        if(*it == appender){
                                m_appenders.erase(it);
                                break;
                        }
                }
        }

        void Logger::log(LogLevel::Level level,const LogEvent & event){
                if(m_appenders.empty()){
                        m_defaultAppender->log(*this,level,event);
                        return;
                }
                if(level >= m_level){
                        for(auto& it : m_appenders){
                                std::unique_lock<std::mutex> lock(m_mutex);                                
                                it->log(*this,level,event);
                        }
                }
        }

        void Logger::debug(const LogEvent & event){
                log(LogLevel::Level::DEBUG,event);
        }

        void Logger::info(const LogEvent & event){
                log(LogLevel::Level::INFO,event);
        }

        void Logger::warn(const LogEvent & event){
                log(LogLevel::Level::WARN,event);    
        }

        void Logger::error(const LogEvent & event){
                log(LogLevel::Level::ERROR,event);    
        }

        void Logger::fatal(const LogEvent & event){
                log(LogLevel::Level::FATAL,event);    
        }

        void Logger::debug(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName){
                LogEvent event(str,line,fileName,funcName);
                log(LogLevel::Level::DEBUG,event);
        }

        void Logger::info(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName){
                LogEvent event(str,line,fileName,funcName);
                log(LogLevel::Level::INFO,event);
        }

        void Logger::warn(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName){
                LogEvent event(str,line,fileName,funcName);
                log(LogLevel::Level::WARN,event);    
        }

        void Logger::error(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName){
                LogEvent event(str,line,fileName,funcName);
                log(LogLevel::Level::ERROR,event);    
        }

        void Logger::fatal(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName){
                LogEvent event(str,line,fileName,funcName);
                log(LogLevel::Level::FATAL,event);    
        }

        YAML::Node Logger::toYamlNode (){
                std::unique_lock<std::mutex> lock(m_mutex); 
                YAML::Node node (YAML::NodeType::Map);
                node["name"] = m_name;
                node["level"] = LogLevel::toString(m_level);
                for(auto  a : m_appenders){
                        node["appenders"].push_back(a->toYamlNode());
                }
                return node;
        }

        void StdoutLogAppender::log(const Logger & logger,LogLevel::Level level, const LogEvent & event){
                if(level >= logger.getLevel() && level >= m_level){
                        std::cout<<m_formatter->format(logger,level,event);
                }
        }

        YAML::Node StdoutLogAppender::toYamlNode(){
                YAML::Node node(YAML::NodeType::Map);
                node["type"] = "StdoutLogAppender";
                node["formatter"] = m_formatter->getPattern();
                node["level"] = LogLevel::toString(m_level);
                return node;
        }

        void FileLogAppender::log(const Logger & logger,LogLevel::Level level, const LogEvent & event){
                if(!reopen()){
                        std::cout<<m_filename<<"  文件打开异常"<<std::endl;
                }
                if(level >= logger.getLevel() && level >= m_level){
                        m_fileOfstream<<m_formatter->format(logger,level,event);
                }
        }

        YAML::Node FileLogAppender::toYamlNode(){
                YAML::Node node(YAML::NodeType::Map);
                node["type"] = "StdoutLogAppender";
                node["formatter"] = m_formatter->getPattern();
                node["level"] = LogLevel::toString(m_level);
                node["file"] = m_filename;
                return node;
        }

        bool FileLogAppender::reopen(){
                if(m_fileOfstream){
                        m_fileOfstream.close();
                }
                m_fileOfstream.open(m_filename,std::ofstream::app | std::ofstream::out);
                return !!m_fileOfstream;
        }

        std::string LogFormatter::format(const Logger & logger,LogLevel::Level level,const LogEvent & event){
                std::stringstream ss;
                for(auto it : m_items){
                        it->format(ss,logger,level,event);
                }
                return ss.str();
        }

        void LogFormatter::init(){
                //pair是将string和int说和在一起,类似于 map
                //tuple 是一个元素组合,把不同类型的组成一个组;
                // std::vector<std::pair<std::string,int>> vec;
                // str(打印字符) ,format(打印对应的格式) ,type(字符还是特殊类型)
                std::vector<std::tuple<std::string, std::string,int>> vec;
                //打印的字符串
                std::string nstr;
                for (size_t i = 0; i < m_pattern.size(); i++)
                {
                        //字符串流
                        if(m_pattern[i] != '%'){
                                nstr.append(1,m_pattern[i]);
                                continue;
                        }
                        //  转移符 打印%
                        if((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%'){
                                nstr.append(1,'%');
                                continue;
                        }
                        //格式标识
                        std::string markstr;
                        //格式字符串
                        std::string fmtstr;
                        size_t fmt_begin = 0; //{}开始的位置
                        bool fmtResolve = false; //是否在解析 样式
                        size_t pos = i + 1;//解析到pattern字符串的位置
                        //存在格式字符流
                        if((i + 1) < m_pattern.size()){
                                while(pos < m_pattern.size()){
                                        //%xxx{xx}    解析非括号内的字符   %sdsa /  -->   sdsa
                                        if(!fmtResolve && !isalpha(m_pattern[pos]) && m_pattern[pos] != '{' && m_pattern[pos] != '}'){
                                                markstr = m_pattern.substr(i+1,pos - i - 1);
                                                break;
                                        }else if(!fmtResolve){
                                                if(m_pattern[pos] == '{'){
                                                        markstr = m_pattern.substr(pos,pos - i - 1);
                                                        fmt_begin = pos + 1;
                                                        fmtResolve = true;
                                                        ++ pos;
                                                        continue;
                                                }
                                        }else if(fmtResolve && m_pattern[pos] == '}'){ //大括号里面的内容
                                                fmtstr = m_pattern.substr(fmt_begin,pos - fmt_begin);
                                                fmtResolve = false;
                                                ++pos;
                                                continue;
                                        }
                                ++pos;
                                if(pos == m_pattern.size()){
                                        if(markstr.empty()){
                                                markstr = m_pattern.substr(i + 1);
                                        }
                                }                                
                                }
                                //标示解析未完成  存在语法错误
                                if(fmtResolve){
                                        std::cout<<"patter parse error:"<<m_pattern<<"-->"<<m_pattern.substr(i)<<std::endl;
                                        vec.push_back(std::make_tuple("<<patter_error>>",fmtstr,0));
                                        m_error = true;
                                }else{
                                        if(!nstr.empty()){
                                                vec.push_back(std::make_tuple(nstr,"",0));
                                                nstr.clear();
                                        }
                                        vec.push_back(std::make_tuple(markstr,fmtstr,1));
                                        i = pos - 1;
                                }
                        }
                }
                //出现 错误和没出现%的情况
                if(!nstr.empty()) {
                        vec.push_back(std::make_tuple(nstr, "", 0));
                }
                
                //下面进行   每个属性对应不同的方法或类
                // 
                //  %m 消息提
                //  %p输出优先级
                //  %r 启动后的时间
                //  %c 日志名称
                //  %t 线程id
                //  %n 回车换行符
                //  %d 时间
                //  %f 文件名
                //  %l 行号
                // %X:协程id
                //  %T tab
                //% F :function
                
                //这里是 特殊字符流和对应formatitem类的映射
                std::map<std::string,std::function<LogFormatter::FormatItem::ptr(const std::string &)>> fmtItemsMap = {
                #define XX(str,C) \
                        {#str, [](const std::string& fmt) { return LogFormatter::FormatItem::ptr(new C(fmt));}}
                        
                        XX(m, MessageFormatItem),           //m:消息
                        XX(p, LevelFormatItem),             //p:日志级别
                        XX(r, ElapseFormatItem),            //r:累计毫秒数
                        XX(c, NameFormatItem),              //c:日志名称
                        XX(t, ThreadIdFormatItem),          //t:线程id
                        XX(n, EnterFormatItem),           //n:换行
                        XX(d, DateFormatItem),          //d:时间
                        XX(f, FileNameFormatItem),          //f:文件名
                        XX(l, LineNumFormatItem),              //l:行号
                        XX(T, TabFormatItem),               //T:Tab
                        XX(X, FiberIdFormatItem),           //X:协程id
                        XX(F,FuncFormatItem),           //方法名
                        //XX(N, ThreadNameFormatItem),        //N:线程名称
                
                #undef XX
                };
                for(auto& t : vec){
                        //get 获取s元素组中的第2个元素
                        if(std::get<2>(t) == 0){
                                m_items.push_back(LogFormatter::FormatItem::ptr(new StringFormatItem(std::get<0>(t))));
                        }else{
                                //获得参数串  对应的iterator
                                auto it = fmtItemsMap.find(std::get<0>(t));
                                if(it == fmtItemsMap.end()){
                                        //表示没有查到该参数
                                        m_items.push_back(LogFormatter::FormatItem::ptr(new StringFormatItem("<<pattern error %" +std::get<0>(t) + ">>")));
                                }else{
                                        m_items.push_back(it->second(std::get<1>(t)));
                                }
                        }
                }
        }

        void StringFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<m_str;
        }

        void MessageFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<event.getContent();
        }

        void LevelFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<LogLevel::toString(level);
        }

        void ElapseFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                clock_t elapse =  clock();
                os<< ((double)elapse/CLOCKS_PER_SEC) << "s";
        }

        void NameFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<logger.getName();
        }        

        void ThreadIdFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<gettid();
        }

        void EnterFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<"\r\n";
        }

        void DateFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                tm tm;
                time_t cur_time = time(NULL);
                localtime_r(&cur_time,&tm);
                char buf[64];
                strftime(buf,64,m_format.c_str(),&tm);
                os<<buf;
        }

        void FileNameFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<event.getFileName();
        }

        void LineNumFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<event.getLineNum();
        }

        void FiberIdFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                
        }

        void TabFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<"\t";
        }

        void FuncFormatItem::format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event){
                os<<event.getFuncName();
        }

        struct LogAppenderDefine
        {
                int type = 0; //1.file  2 stdout
                LogLevel::Level level = LogLevel::Level::NOKNOW;
                std::string formatter;
                std::string file;
                bool m_valid = false;

                bool operator ==( const LogAppenderDefine & oth) const{
                        return type == oth.type  &&
                                        level ==oth.level &&
                                        formatter == oth.formatter &&
                                        file == oth.file;
                }

                bool isValid(){
                        return m_valid;
                }
        };
        /**
         * @brief YAML::Node转变成  LogAppenderDefine
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<YAML::Node,LogAppenderDefine>{
        public:
                LogAppenderDefine operator ()(const YAML::Node & node){
                        LogAppenderDefine appDefine;
                        if( node["type"].IsDefined() ){
                                std::string type = node["type"].as<std::string>();
                                if(type == "FileLogAppender"){
                                        appDefine.type = 1;
                                        if(!node["file"].IsDefined()){
                                                RAYMOND_RLOG_ERROR("log config error: fileAppender file is null");
                                                return appDefine;
                                        }
                                        appDefine.file = node["file"].as<std::string>();
                                        appDefine.m_valid = true;
                                }else if(type == "StdoutLogAppender"){
                                        appDefine.type = 2;
                                        appDefine.m_valid = true;
                                }else{
                                        return appDefine;
                                }
                                if(node["level"].IsDefined()){
                                        appDefine.level = LogLevel::toLevel(node["level"].as<std::string>());
                                }
                                if(node["formatter"].IsDefined()){
                                        appDefine.formatter = node["formatter"].as<std::string>();
                                }
                                return appDefine;
                        }
                        RAYMOND_RLOG_ERROR("log config error : logAppend type is null");
                        return appDefine;
                }
        };
        /**
         * @brief YAML::LogAppenderDefine转变成 Node
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<LogAppenderDefine ,YAML::Node>{
        public:
                YAML::Node operator ()(const LogAppenderDefine & def){
                        YAML::Node node(YAML::NodeType::Map);
                        node["type"] = (def.type == 1 ? "FileLogAppender" : "StdoutLogAppender");
                        node["level"] = LogLevel::toString(def.level);
                        if(def.formatter.empty()){
                                node["formatter"] = def.formatter;      
                        }
                        if(!def.file.empty()){
                                node["file"] = def.file;
                        }
                        return node;
                }
        };

        struct LogDefine
        {
                std::string name;
                LogLevel::Level level;
                std::vector<LogAppenderDefine> appenders;

                bool operator== (const LogDefine & oth) const {
                        return name == oth.name &&
                                        level == oth.level &&
                                        appenders == oth.appenders;
                }

                bool operator < (const LogDefine & oth) const {
                        return name < oth.name;
                }
        };
        /**
         * @brief YAML::Node转变成  LogDefine
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<YAML::Node,LogDefine>{
        public:
                LogDefine operator ()(const YAML::Node & node){
                        LogDefine logDef;
                        if(!node["name"].IsDefined()){
                                RAYMOND_RLOG_ERROR("Log config error : name is null");
                                return logDef;
                        }
                        logDef.name = node["name"].as<std::string>();
                        if(node["level"].IsDefined()){
                                logDef.level = LogLevel::toLevel(node["level"].as<std::string>());
                        }
                        if(node["appenders"].IsDefined()){
                                auto appenders = LexicalCast<YAML::Node,std::vector<LogAppenderDefine> >()(node["appenders"]);
                                for(auto it = appenders.begin(); it != appenders.end();++it){
                                        if(!it->isValid()){
                                                appenders.erase(it);
                                        }
                                }
                                logDef.appenders = appenders;
                        }
                        return logDef;
                }
        };
        /**
         * @brief YAML::LogDefine转变成 Node
         * 
         * @tparam  
         */
        template<>
        class LexicalCast<LogDefine,YAML::Node>{
        public:
                YAML::Node operator()(const LogDefine & logDef){
                        YAML::Node node(YAML::NodeType::Map);
                        node["name"] = logDef.name;
                        node["level"] = LogLevel::toString(logDef.level);
                        node["appenders"] = LexicalCast<std::vector<LogAppenderDefine>, YAML::Node>()(logDef.appenders);
                        return node;
                }
        };

}
//全局配置log对象
raymond::ConfigVar<std::set<raymond::LogDefine> >::ptr g_logs_define =
        raymond::Config::lookUp("logs",std::set<raymond::LogDefine>(),"logs config");

//添加日志配置文件修改监听器
struct LogIniter
{
        LogIniter(){
                g_logs_define->addListener([](const std::set<raymond::LogDefine> & oldVal,
                                const std::set<raymond::LogDefine> & newVal){
                        RAYMOND_RLOG_INFO("log config has  changed \n");
                        for(auto & i : newVal){
                                //查看oldval里面是否有i ,如果没有则创建
                                auto it = oldVal.find(i);
                                raymond::Logger::ptr logger;

                                if(it != oldVal.end() && i == *it){
                                        continue;
                                }

                                logger = RAYMOND_LOG_BYNAME(i.name);

                                logger->setLevel(i.level);

                                logger->clearAppender();
                                for(auto& a : i.appenders){
                                        raymond::LogAppender::ptr ap;
                                        if(a.type == 1){
                                                ap.reset(new raymond::FileLogAppender(a.file));
                                        }else if(a.type == 2){
                                                ap.reset(new raymond::StdoutLogAppender());
                                        }
                                        ap->setLevel(a.level);
                                        if(!a.formatter.empty()){
                                                auto formatter = raymond::LogFormatter::ptr(new raymond::LogFormatter(a.formatter));
                                                if(!formatter->isError()){
                                                        ap->setFormatter(formatter);
                                                }else{
                                                        RAYMOND_RLOG_ERROR("log name : %s , format_pattern is error : <%s>",
                                                                i.name,a.formatter.c_str());
                                                }                                                
                                        }
                                        logger->addAppender(ap);
                                }
                        }
                        //删除多余的logger
                        for(auto & i : oldVal){
                                auto it = newVal.find(i);
                                if(it == newVal.end()){
                                        RAYMOND_LOGMANAGE().delLogger(i.name);
                                }
                        }
                });
        }
};

static LogIniter s_logIniter;



