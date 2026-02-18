// Copyright (c) 2026 Brace Software Co.
// BetterC++/bCXX/bC++ Compiler

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <unordered_map>
// ----------------------------------------- MACROS -----------------------------------------
#define __BCXX_CLASS_ACCESS_MODIFIER(ss, KWORD__, result, lines) if(result.rfind(KWORD__, 0) == 0)\
            {\
                ss.clear();std::string rest = result.substr(KWORD__.size());\
                if(bcxx__::flags::template_found == -1)\
                {\
                    ss << KWORD__;\
                    ss << ":\n";\
                }\
                if(bcxx__::flags::template_found != -1)\
                {\
                    lines[bcxx__::flags::template_found] = (KWORD__ + ":\n") + lines[bcxx__::flags::template_found];\
                    bcxx__::flags::template_found = -1;\
                }\
                ss << rest;\
                result = ss.str();\
                return result;\
            }

// ----------------------------------------- CODE -----------------------------------------

namespace bcxx__
{
    std::vector<std::pair<std::string, int>> line_info;
    std::vector<std::string> lines;
    static inline void print(const std::string& text)
    {
        std::cout << "bC++: " << text << std::endl;
        return;
    }

    namespace flags
    {
        int template_found = (-1);
        unsigned int defer_count = 0;
    }

    std::vector<std::string> DEFER_BLOCK__vec;
    constinit const int DEFER_BLOCK = 1;

    std::vector<int> __BCXX_BRACE_STACK;

    namespace util
    {
        int find(const std::string& s, const std::string& what)
        {
            if(what.empty())
            {
                return 0;
            }
            if(s.empty())
            {
                return -1;
            }
            if(s.size() < what.size())
            {
                return -1;
            }
            if(s == what)
            {
                return 0;
            }
            for(int i = 0; i <= s.size() - what.size(); ++i)
            {
                bool match = true;
                for(int j = 0; j < what.size(); ++j)
                {
                    if(s[i + j] != what[j])
                    {
                        match = false;
                        break;
                    }
                }
                if(match)
                {
                    return i;
                }
            }
            return -1;
        }
        bool isbetween(const std::string& str, const std::string& delim, char what)
        {
            int pos1 = -1,pos2 = -1,delpos = -1;
            delpos = util::find(str, delim);
            for(int i = 0; i < str.size(); ++i)
            {
                #if 0
                if(str.at(i) == delim)
                {
                    if(delpos == -1)
                    {
                        delpos = i;
                        //std::cout << "found delpos " << delpos << std::endl;
                        continue;
                    }
                }
                #endif
                if(str.at(i) == what)
                {
                    if(pos1 == -1)
                    {
                        pos1 = i;
                        //std::cout << "found pos1 " << pos1 << std::endl;
                        continue;
                    }
                    if(pos2 == -1)
                    {
                        pos2 = i;
                        //std::cout << "found pos2 " << pos2 << std::endl;
                        continue;
                    }
                }
            }
            if((delpos != std::string::npos) && (pos1 != -1) && (pos2 != -1))
            {
                //std::cout << "pos1 " << pos1<< "delpos " << delpos << "pos2" << pos2 << std::endl;
                if(pos1 < delpos && delpos < pos2)
                {
                    return true;
                }
            }
            return false;
        }
        inline std::string remc(const std::string& line)
        {
            if(isbetween(line, "//", '"') or isbetween(line, "//", '\''))
            {
                return line;
            }
            std::size_t pos = line.find("//");
            if(pos != std::string::npos)
            {
                return line.substr(0, pos);
            }
            return line;
        }
        std::string trim(const std::string& str);
        [[nodiscard]]
        inline std::vector<std::string> split(const std::string& str, char delimiter)
        {
            std::vector<std::string> tokens;
            std::string token;
            std::stringstream ss(str);

            while(std::getline(ss, token, delimiter))
            {
                if(bcxx__::util::trim(token).empty()) continue;
                tokens.push_back(token);
            }

            return tokens;
        }
        [[nodiscard]]
        inline std::vector<std::string> split_fixed(const std::string &str, char delimiter)
        {
            size_t pos = str.find(delimiter);
            std::vector<std::string> tokens;

            if(pos != std::string::npos)
            {
                tokens.push_back(str.substr(0, pos));
                tokens.push_back(str.substr(pos + 1));
            }
            else
            {
                tokens.push_back(str);
            }
            
            return tokens;
        }
        [[nodiscard]]
        inline std::string trim(const std::string &str)
        {
            auto start = str.begin();
            auto end = str.end();

            while(start != end && std::isspace(static_cast<unsigned char>(*start)))
            {
                ++start;
            }

            if(start == end)
            {
                return static_cast<std::string>("");
            }
            auto it = end;
            do
            {
                --it;
            }
            while(it != start && std::isspace(static_cast<unsigned char>(*it)));

            //returns trimmed string
            return std::string(start, it + 1);
        }
    }
    

    class file_linker
    {
        private:
        static inline void make(const std::string& filename, const std::vector<std::string>& lines)
        {
            std::ofstream file(filename);

            if(!file.is_open())
            {
                bcxx__::print("System has encountered an error while opening `" + filename + "`.");
                return;
            }

            for(const auto& line : lines)
            {
                file << line << "\n";
            }

            file.close();
            return;
        }

        template<typename T>
        static inline void __repl_vec_seg(std::vector<T>& v, const std::vector<T>& n, std::size_t index)
        {
            if(index >= v.size())
            {
                return;
            }

            v.insert(v.erase(v.begin() + index), n.begin(), n.end());
        }

        static inline std::pair<bool, std::vector<std::string>> read(const std::string& filename)
        {
            static const std::vector<std::string> extensions = {
                ".bcpp", ".cpp", ".c", ".h", ".hpp"
            };
            int exists = -1;

            for(int i = 0; i < extensions.size(); ++i)
            {
                if(std::filesystem::exists(filename + extensions.at(i)))
                {
                    exists = i;
                    break;
                }
            }

            if(exists == -1)
            {
                bcxx__::print("No such file or directory -> " + filename);
                return {false, {}};
            }

            std::ifstream file(filename + extensions.at(exists));

            if(!file.is_open())
            {
                bcxx__::print("System has encountered an error while opening `" + filename + "`.");
                return {false, {}};
            }

            std::vector<std::string> lines;

            std::string line;
            while(std::getline(file, line))
            {
                lines.push_back(line);
            }

            file.close();
            return {true, lines};
        }
        static inline std::pair<bool, std::string> __BCXX_SYNTAX_MATCH_link(const std::string& line)
        {
            static const std::regex pattern("^\\s*link\\s+\"([^\"]+)\"\\s*;$");//std::regex pattern(R"(^package\s+(.+);$)");
            std::smatch match;
            if(std::regex_match(line, match, pattern))
            {
                return {true, match[1].str()};
            }

            return {false, ""};
        }

        static inline std::pair<bool, std::string> __BCXX_get_annotation(const std::string& s)
        {
            std::string result = "\0";
            if(s.at(0) == '@')
            {
                result = bcxx__::util::trim(s.substr(1));
                return {true, result};
            }
            return {false, result};
        }

        static inline std::string __BCXX_PARSE(int idx, decltype(lines)& v)
        {
            auto tokenize = [](std::string str) -> std::vector<std::string> {
                std::vector<std::string> tokens;
                std::string token;
                std::istringstream stream(str);

                while(stream >> token)
                {
                    tokens.push_back(token);
                }

                return tokens;
            };
            using namespace bcxx__;
            std::stringstream ss;
            std::string result = v[idx];
            result = util::remc(result);
            //replace public:, protected:, and private: in standard C++
            result = bcxx__::util::trim(result);
            if(result.empty())
            {
                return result;
            }
            static const std::string KWORD_DEFER = "defer";
            static const std::string KWORD_FINAL = "final";
            //custom BC++ decorators
            if(result == "$")
            {
                if(__BCXX_BRACE_STACK.empty())
                {
                    bcxx__::print("ERROR -> UNEXPECTED `$`");
                    return std::string("_______BCXX_ERROR_(`$` unexpected)");
                }
                int block = 0;
                block = __BCXX_BRACE_STACK.back();
                __BCXX_BRACE_STACK.pop_back();
                if(block == bcxx__::DEFER_BLOCK)
                {
                    if(DEFER_BLOCK__vec.empty())
                    {
                        bcxx__::print("ERROR -> UNEXPECTED `$`; defer block is empty!");
                        return std::string("_______BCXX_ERROR_(empty defer block)");
                    }
                    ss.clear();
                    ss.str("");

                    ss << "struct __BCXX_DEFER_" << bcxx__::flags::defer_count << " " << KWORD_FINAL << "{\n";
                    ss << "inline ~__BCXX_DEFER_" << bcxx__::flags::defer_count << "(){\n";
                    for(int i = 0; i < DEFER_BLOCK__vec.size(); ++i)
                    {
                        ss << DEFER_BLOCK__vec[i] << "\n";
                    }
                    ss << "}};\n__BCXX_DEFER_" << bcxx__::flags::defer_count << " __BCXX_DEFEROBJECT" << bcxx__::flags::defer_count << ';';
                    DEFER_BLOCK__vec.clear();
                    bcxx__::flags::defer_count ++;
                    return ss.str();
                }
                return std::string("//__BCXX_DECORATOR_END;");
            }
            if(!__BCXX_BRACE_STACK.empty()) if(__BCXX_BRACE_STACK.back() == bcxx__::DEFER_BLOCK)
            {
                DEFER_BLOCK__vec.push_back(result);
                return std::string("//__BCXX_DECORATOR_END->defer-block!");
            }
            if(result[0] == '$' and result.size() >= 3)
            {
                std::string token = result.substr(1);
                token = util::trim(token);
                if(token == KWORD_DEFER)
                {
                    __BCXX_BRACE_STACK.push_back(bcxx__::DEFER_BLOCK);
                    return std::string("//__BCXX_DEFER_BLOCK;");
                }
            }
            //annotations @ -> [[]]
            auto ann = __BCXX_get_annotation(result);
            if(ann.first)
            {
                result = "[[" + ann.second + "]]";
                if(bcxx__::flags::template_found == -1)
                {
                    bcxx__::flags::template_found = idx;
                }
                return result;
            }
            //class access modifiers
            static const std::string KWORD_PUBLIC = "public";
            static const std::string KWORD_PRIVATE = "private";
            static const std::string KWORD_PROTECTED = "protected";
            static const std::string KWORD_TEMPLATE = "template";

            if(result.rfind(KWORD_PUBLIC, 0) == 0 or result.rfind(KWORD_PRIVATE, 0) == 0 or result.rfind(KWORD_PROTECTED, 0) == 0)
            if(result.back() == ':' and bcxx__::flags::template_found != -1)
            {
                bcxx__::flags::template_found = -1;
            }
            if(result.back() != ':')
            {
                __BCXX_CLASS_ACCESS_MODIFIER(ss,KWORD_PRIVATE, result, v)
                __BCXX_CLASS_ACCESS_MODIFIER(ss,KWORD_PUBLIC, result, v)
                __BCXX_CLASS_ACCESS_MODIFIER(ss,KWORD_PROTECTED, result, v)
            }
            //detect template
            if(result.rfind(KWORD_TEMPLATE, 0) == 0)
            {
                //bcxx__::print("Found template at " + std::to_string(idx));
                if(bcxx__::flags::template_found == -1)
                {
                    bcxx__::flags::template_found = idx;
                }
                return result;
            }
            //class inheritance
            static const std::string KWORD_EXTENDS = "extends";
            static const std::string KWORD_CLASS = "class";
            static const std::string KWORD_STRUCT = "struct";
            auto vec = tokenize(result);
            if(!vec.empty()) if(vec.at(0) == KWORD_CLASS or vec.at(0) == KWORD_STRUCT)
            {
                bcxx__::flags::template_found = -1;
            }
            auto is_access_modifier = [](std::string s) -> bool {
                if(s == KWORD_PROTECTED or s == KWORD_PRIVATE or s == KWORD_PUBLIC)
                {
                    return true;
                }
                return false;
            };

            if(vec.size() >= 4)
            {
                if(vec.at(0) == KWORD_CLASS and vec.at(2) == KWORD_EXTENDS and not is_access_modifier(vec.at(3)))
                {
                    //result = KWORD_CLASS + vec.at(1) + ":" + KWORD_PUBLIC + vec.at(3);
                    result.clear();
                    for(int i = 0; i < vec.size(); ++i)
                    {
                        if(i == 0)
                        {
                            result.append(KWORD_CLASS + " ");
                            continue;
                        }
                        if(i == 2)
                        {
                            result.append(KWORD_PUBLIC + " ");
                            continue;
                        }
                        result.append(vec.at(i) + " ");
                        if(i == 1)
                        {
                            result.append(":");
                        }
                    }
                    return result;
                }
                if(vec.at(0) == KWORD_CLASS and vec.at(2) == KWORD_EXTENDS and is_access_modifier(vec.at(3)))
                {
                    //result = KWORD_CLASS + vec.at(1) + ":" + KWORD_PUBLIC + vec.at(3);
                    result.clear();
                    for(int i = 0; i < vec.size(); ++i)
                    {
                        if(i == 0)
                        {
                            result.append(KWORD_CLASS + " ");
                            continue;
                        }
                        if(i == 2)
                        {
                            result.append(" ");
                            continue;
                        }
                        result.append(vec.at(i) + " ");
                        if(i == 1)
                        {
                            result.append(":");
                        }
                    }
                    return result;
                }
            }
            if(vec.size() >= 4)
            {
                if(vec.at(0) == KWORD_STRUCT and vec.at(2) == KWORD_EXTENDS and not is_access_modifier(vec.at(3)))
                {
                    //result = KWORD_CLASS + vec.at(1) + ":" + KWORD_PUBLIC + vec.at(3);
                    result.clear();
                    for(int i = 0; i < vec.size(); ++i)
                    {
                        if(i == 0)
                        {
                            result.append(KWORD_STRUCT + " ");
                            continue;
                        }
                        if(i == 2)
                        {
                            result.append(KWORD_PUBLIC + " ");
                            continue;
                        }
                        result.append(vec.at(i) + " ");
                        if(i == 1)
                        {
                            result.append(":");
                        }
                    }
                    return result;
                }
                if(vec.at(0) == KWORD_STRUCT and vec.at(2) == KWORD_EXTENDS and is_access_modifier(vec.at(3)))
                {
                    //result = KWORD_CLASS + vec.at(1) + ":" + KWORD_PUBLIC + vec.at(3);
                    result.clear();
                    for(int i = 0; i < vec.size(); ++i)
                    {
                        if(i == 0)
                        {
                            result.append(KWORD_STRUCT + " ");
                            continue;
                        }
                        if(i == 2)
                        {
                            result.append(" ");
                            continue;
                        }
                        result.append(vec.at(i) + " ");
                        if(i == 1)
                        {
                            result.append(":");
                        }
                    }
                    return result;
                }
            }
            return result;
        }

        public:
        static inline void COMPILE__(const std::string& filename, const std::string& outputfilename)
        {
            bcxx__::print("Processing entry file `" + filename + "`...");
            auto P = bcxx__::file_linker::read(filename);
            if(!P.first)
            {
                bcxx__::print("fatal error");
                return;
            }
            auto v = P.second;
            int files = 0;

            for(int i = 0; i < v.size(); ++i)
            {
                bcxx__::line_info.push_back({filename, i + 1});
            }

            int linetemp;
            std::string temp;
            std::vector<std::string> v2;
            std::vector<std::pair<std::string, int>> line_data_local;
            for(int i = 0; i < v.size(); ++i)
            {
                temp.clear();
                v2.clear();
                line_data_local.clear();
                auto p = bcxx__::file_linker::__BCXX_SYNTAX_MATCH_link(v[i]);
                if(p.first)
                {
                    std::string module_path = p.second;
                    module_path = bcxx__::util::trim(module_path);
                    //std::cout << "module_path -> " << module_path << std::endl;
                    auto c = bcxx__::file_linker::read(module_path);
                    if(!c.first)
                    {
                        continue;
                    }
                    v2 = c.second;
                    bcxx__::print("Linking package/app module `" + module_path + "`...");
                    bcxx__::file_linker::__repl_vec_seg(v, v2, i);

                    for(int j = 0; j < v2.size(); ++j)
                    {
                        line_data_local.push_back({p.second, j + 1});
                    }

                    bcxx__::file_linker::__repl_vec_seg(bcxx__::line_info, line_data_local, i);

                    files++;
                }
            }

            bcxx__::print("Successfully linked " + std::to_string(files) + " files.");
            //bcxx__::lines = v;
            for(int i = 0; i < v.size(); ++i)
            {
                //std::cout << v.size() << " DEBUG -> " << i << " : " << v.at(i) << std::endl;
                v[i] = bcxx__::file_linker::__BCXX_PARSE(i, v);
            }
            if(false) for(int i = 0; i < v.size(); ++i)
            {
                v[i] = "\t" + v[i] + "\t";
            }
            bcxx__::file_linker::make(outputfilename, v);
            return;
        }
    };

}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        bcxx__::print("only 2 arguments are needed -> filename output name\n\t\tbc++ my_code.cpp outputcode.cpp\n\n\t\tMAKE SURE NOT TO ADD ANY FILE EXTENSIONS! COMPILER USES THE *.bcpp EXTENSION!!!");
        return 1;
    }

    std::string file = argv[1];
    std::string out = argv[2];
    
    bcxx__::file_linker::COMPILE__(file,out);
    bcxx__::print("Compiled...");
    return 1;
}
