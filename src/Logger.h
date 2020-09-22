/**
 * @file Logger.h
 * @author raymond-du
 * @brief       日志模板的分装
 * @version 0.1
 * @date 2020-08-01
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <memory>
#include <list>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <map>
#include <functional>
#include <stdarg.h>
#include <yaml-cpp/yaml.h>
#include "singleton.h"
#include <mutex>

#ifndef __linux__
#include "windows.h"
#else
#include <sys/syscall.h>
#define gettid() syscall(186)
#endif

#define __LOGPARAMS__ __LINE__,__FILE__,__FUNCTION__


//获取 全局 logManage
#define RAYMOND_LOGMANAGE() \
        raymond::SingleTon<raymond::LogManage>::getInstance()
//获取root 日志器
#define RAYMOND_LOG_ROOT() \
        RAYMOND_LOGMANAGE().getRoot()
//获取name的日志器
#define RAYMOND_LOG_BYNAME(name) \
        RAYMOND_LOGMANAGE().getLogger(name)
/**
 * @brief  root 日志打印
 * 
 */
#define RAYMOND_RLOG_ERROR(fmt,...) \
        RAYMOND_LOG_ROOT()->error(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_RLOG_INFO(fmt,...) \
        RAYMOND_LOG_ROOT()->info(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_RLOG_WARN(fmt,...) \
        RAYMOND_LOG_ROOT()->warn(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_RLOG_FATAL(fmt,...) \
        RAYMOND_LOG_ROOT()->fatal(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_RLOG_DEBUG(fmt,...) \
        RAYMOND_LOG_ROOT()->debug(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)

#define RAYMOND_LOG_FMT_ERROR(logger,fmt,...) \
        logger->error(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_LOG_FMT_DEBUG(logger,fmt,...) \
        logger->debug(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_LOG_FMT_FATAL(logger,fmt,...) \
        logger->fatal(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_LOG_FMT_WARN(logger,fmt,...) \
        logger->warn(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)
#define RAYMOND_LOG_FMT_INFO(logger,fmt,...) \
        logger->info(raymond::getFmtStr(fmt,##__VA_ARGS__),__LOGPARAMS__)

namespace raymond
{
        static std::string getFmtStr(const char * fmt,...){
                va_list args;
                char buf[1024*4];
                va_start(args,fmt);
                vsnprintf(buf,1024*4,fmt,args);
                va_end(args);
                return buf;
        }

        class LogEvent;
        class Logger;
        class LogFormatter;
        class LogAppender;
        class StdoutLogAppender;

        /**
         * @brief 日志的事件
         * 
         */
        class LogEvent
        {
        public:
                //用这个来表示 ptr对空间的对象
                typedef std::shared_ptr<LogEvent> ptr;

                LogEvent(){}
                /**
                 * @brief  否早函数
                 * 
                 * @param content  打印日志的内容
                 * @param line          当前行号
                 * @param fileName      当前的文件名
                 * @param funcName      当前的func
                 */
                LogEvent(const std::string & content,uint32_t  line,std::string  fileName,const std::string & funcName) {
                       //分割出文件名,去除路径
                        std::size_t lastPos = fileName.find_last_of('/');
                       m_fileName =  fileName.substr(lastPos + 1);
                       m_line = line;
                       m_content = content;
                       m_funcName = funcName;
                };

                LogEvent(const std::string & filename,uint32_t line,const std::string & content):m_fileName(filename),
                        m_line(line),m_content(content){}

                /**
                 * @brief 获取 相关的属性方法
                 * 
                 * @return  放回属性的值
                 */
                const std::string & getFileName() const { return m_fileName; }
                uint32_t getLineNum() const { return m_line; }
                const std::string &getContent() const { return m_content; }    
                const std::string &getFuncName() const { return m_funcName; }                

                /**
                 * @brief 设置属性的值
                 * 
                 * @param  设置参数的值
                 * @return      返回LogEvent的引用 ,可重复的设置;
                 */
                LogEvent & setLine(uint32_t line){m_line = line; return *this;}
                LogEvent &  setContent(const std::string & content){m_content = content;return *this;}
                LogEvent &  setFuncName(const std::string & funcName){m_funcName = funcName;return *this;}
        private:
                std::string m_fileName;
                std::string m_funcName;
                uint32_t m_line = 0;
                std::string m_content;
        };

        /**
         * @brief 日志级别
         * 
         */
        class LogLevel
        {
        public:
                /**
                 * @brief 日志级别的枚举
                 * 
                 */
                enum Level
                {
                        NOKNOW = 0,
                        DEBUG = 1,
                        INFO = 2,
                        WARN = 3,
                        ERROR = 4,
                        FATAL = 5
                };
                /**
                 * @brief  日志等级转换成文本
                 * 
                 * @param 枚举  等级 
                 * @return 返回的字符串文本 
                 */
                static const char * toString(LogLevel::Level level);
               
                /**
                 * @brief  将文本转换成日志等级
                 * 
                 * @param str  文本字符串
                 * @return      日志等级
                 */
                static LogLevel::Level toLevel(const std::string & str);
        };

        /**
         * @brief  日志格式化
         * 
         */
        class LogFormatter
        {
        public:
                typedef std::shared_ptr<LogFormatter> ptr;
                /**
                 *  
                * %m 消息提
                * %p输出优先级
                * %r 启动后的时间
                * %t 线程id
                * %n 回车换行符
                * %d 时间
                * %f 文件名
                * %l 行号
                * %X:协程id
                * %F :function
                 * 
                 */

                LogFormatter(){}
                LogFormatter(const std::string &pattern) : m_pattern(pattern) { init(); }
                std::string format(const Logger & logger,LogLevel::Level level,const LogEvent & event);

                bool isError(){return m_error;}

        public:
                //日志解析的子模块
                class FormatItem
                {
                public:
                        typedef std::shared_ptr<FormatItem> ptr;
                        virtual ~FormatItem() {}
                        /**
                         * @brief  以流的方式输出
                         * 
                         * @param os            输出流
                         * @param logger     日志器
                         * @param level         日志等级
                         * @param event         日志事件
                         */
                        virtual void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) = 0;
                };
                //做pattern的解析;
                void init();

                std::string getPattern(){return m_pattern;}
                void setPattern(const std::string & pattern){m_pattern = pattern;}

        private:
                std::string m_pattern;
                //打印输出的子项
                std::vector<FormatItem::ptr> m_items;
                //pattern语法是否正确
                bool m_error = false;
        };
        class StringFormatItem: public LogFormatter::FormatItem{
        public:
                StringFormatItem(const std::string & str = ""):m_str(str){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        private:
                //输出普通的字符流
                std::string m_str;
        };
        /**
         * @brief  
         * @note    日志内容项格式化
         */
        class MessageFormatItem : public LogFormatter::FormatItem
        {
        public:
                MessageFormatItem(const std::string &str = "") {}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  
         * @note   日志输出等级的 famat 类
         */
        class LevelFormatItem : public LogFormatter::FormatItem
        {
        public:
                LevelFormatItem(const std::string &str = "") {}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  
         * @note    运行时长
         * @retval None
         */
        class ElapseFormatItem : public LogFormatter::FormatItem
        {
        public:
                ElapseFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  输出日志的名称
         * @note   
         * @retval None
         */
        class NameFormatItem : public LogFormatter::FormatItem
        {
        public:
                NameFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  打印线程的id
         * @note   
         * @retval None
         */
       class ThreadIdFormatItem : public LogFormatter::FormatItem
        {
        public:
                ThreadIdFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief   换行符
         * @note   
         * @retval None
         */
        class EnterFormatItem : public LogFormatter::FormatItem
        {
        public:
                EnterFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  当前时间
         * @note   
         * @retval None
         */
        class DateFormatItem : public LogFormatter::FormatItem
        {
        private:
                std::string m_format;
        public:
                DateFormatItem(const std::string & str = "%Y-%m-%d %H:%M:%S"){
                       m_format = str;
                       if(m_format.empty()){
                                m_format = "%Y-%m-%d %H:%M:%S";
                        }
                }
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  打印文件名
         * @note   
         * @retval None
         */
         class FileNameFormatItem : public LogFormatter::FormatItem
        {
        public:
                FileNameFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  行号
         * @note   
         * @retval None
         */
        class LineNumFormatItem : public LogFormatter::FormatItem
        {
        public:
                LineNumFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
                /**
         * @brief  协程ID
         * @note   
         * @retval None
         */
        class FiberIdFormatItem : public LogFormatter::FormatItem
        {
        public:
                FiberIdFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
                        /**
         * @brief  输出tab
         * @note   
         * @retval None
         */
        class TabFormatItem : public LogFormatter::FormatItem
        {
        public:
                TabFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief  输出function的名字
         * @note   
         * @retval None
         */
        class FuncFormatItem : public LogFormatter::FormatItem
        {
        public:
                FuncFormatItem(const std::string & str = ""){}
                void format(std::ostream &os,const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
        };
        /**
         * @brief 日志输出目标
         * 
         */
        class LogAppender
        {
        public:
                typedef std::shared_ptr<LogAppender> ptr;

                /**
                 * @brief 输出的的默认格式
                 * 
                 */
                LogAppender(const std::string & pattern = "[%p]%T%d%T%t%T%F(%f : %l)%n%T%m %n"){
                        m_formatter.reset(new LogFormatter(pattern));
                        m_level = LogLevel::Level::NOKNOW;
                }
                virtual ~LogAppender(){}
                virtual void log(const Logger & logger,LogLevel::Level level, const LogEvent & event) = 0;
                virtual YAML::Node toYamlNode() = 0;
                
                void setFormat(const std::string  &pattern) { m_formatter = LogFormatter::ptr(new LogFormatter(pattern)); }
                void setFormatter(LogFormatter::ptr  &formatter) { m_formatter = formatter;}
                LogFormatter::ptr  getFormatter() { return m_formatter; }

                LogLevel::Level  getLevel(){return m_level;}
                void setLevel(LogLevel::Level level){m_level = level;}

        protected:
                LogFormatter::ptr m_formatter;
                LogLevel::Level m_level;
        };
        
        class StdoutLogAppender : public LogAppender
        {
        public:
                ~StdoutLogAppender() override {}
                void log(const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
                YAML::Node toYamlNode() override ;
        };

        class FileLogAppender : public LogAppender
        {
        private:
                std::string m_filename;
                std::ofstream m_fileOfstream;

        public:
                FileLogAppender(const std::string &filename)
                {
                        m_filename = filename;
                }
                ~FileLogAppender()
                {
                }

                void log(const Logger & logger,LogLevel::Level level, const LogEvent & event) override;
                YAML::Node toYamlNode() override;
                //如果文件一打开则 关闭后在重新打开
                bool reopen();
        };

        /**
         * @brief 日志器
         * 
         */
        class Logger
        {
        public:
                typedef std::shared_ptr<Logger> ptr;

                Logger(const std::string & name = "",LogLevel::Level level = LogLevel::Level::DEBUG)
                        :m_name(name),
                        m_level(LogLevel::Level::DEBUG){
                                m_defaultAppender = StdoutLogAppender::ptr(new StdoutLogAppender()); 
                        }

                void log(LogLevel::Level level,const LogEvent & event);

                void debug(const LogEvent & event);
                void info(const LogEvent & event);
                void warn(const LogEvent & event);
                void error(const LogEvent & event);
                void fatal(const LogEvent & event);

                void debug(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName);
                void info(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName);
                void warn(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName);
                void error(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName);
                void fatal(const std::string & str,uint32_t  line,std::string  fileName,const std::string & funcName);

                void addAppender(LogAppender::ptr appender);
                void delAppender(LogAppender::ptr appender);
                void clearAppender(){
                        std::unique_lock<std::mutex> lock(m_mutex); 
                        m_appenders.clear();
                }

                LogLevel::Level getLevel() const{ return m_level; }
                void setLevel(LogLevel::Level level) { this->m_level = level; }

                void setName(const std::string & name){m_name = name;}
                std::string getName()const {return m_name;}

                YAML::Node toYamlNode();
        private:
                std::string m_name;
                LogLevel::Level m_level;
                std::list<LogAppender::ptr> m_appenders; //appender的一个集合
                // 默认的输出的 appender
                LogAppender::ptr m_defaultAppender;
                std::mutex m_mutex; //写到文件和控制台时 防止错乱
        };
        /**
         * @brief 日志管理工具类
         * 
         */

        class LogManage {

        public:

                /**
                 * @brief  查找获取对应名字的Logger
                 *      如果没有则创建一个新的logger 并返回
                 * @param name  日志名
                 * @return      日志器
                 */
                Logger::ptr getLogger(const std::string& name){
                        std::unique_lock<std::mutex> lock(m_mutex); 
                        auto it  = m_logsMap.find(name);
                        if(it != m_logsMap.end()){
                                return it->second;
                        }
                        Logger::ptr log(new Logger(name));
                        m_logsMap.insert(std::pair<std::string,Logger::ptr>(name,log));
                        return log;
                }
                void delLogger(const std::string & name){
                        std::unique_lock<std::mutex> lock(m_mutex); 
                        auto it = m_logsMap.find(name);
                        if(it != m_logsMap.end()){
                                m_logsMap.erase(it);
                        }
                }
                /**
                 * @brief 获取根日志器
                 * 
                 * @return  日志器
                 */
                Logger::ptr getRoot(){return getLogger("root");}

                YAML::Node toYamlNode(){
                        YAML::Node node(YAML::NodeType::Map);
                        for(auto l : m_logsMap){
                                node["logs"].push_back(l.second->toYamlNode());
                        }
                        return node;
                }
        private:
                // 日志名称-->日志器
                std::map<std::string,Logger::ptr> m_logsMap;       
                std::mutex m_mutex;         
        };

} // namespace raymond

#endif // __LOG_H
