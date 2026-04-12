#ifndef SHADERPREPROCESSOR_H_INCLUDED
#define SHADERPREPROCESSOR_H_INCLUDED

#pragma once

#include <string>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <ResourceManager.h>

struct ShaderPreprocessor {
private:
    std::unordered_map<std::string, std::string> values;

    void replaceAll(std::string& str) {
        for(std::pair<std::string, std::string> value : values) {
            size_t pos = 0u;
            while ((pos = str.find(value.first, pos)) != std::string::npos) {
                str.replace(pos, value.first.size(), value.second);
                pos += value.second.size();
            }
        }
    }

    void appendAll(std::string& str, std::unordered_set<std::string>& active) {
        /*size_t pos = 0;
        while(true) {
            size_t start = 0;
            size_t end = 0;
            std::string payload = findDirective(str, "append", pos, start, end);
            if(start == 0 && end == 0) {
                break;
            }
            if(!payload.empty()) {

            }
        }*/
        size_t pos = 0;
        while ((pos = str.find("#append", pos)) != std::string::npos) {
            size_t directiveStart = pos;
            pos += 7;
            while (pos < str.size() && std::isspace((unsigned char) str[pos])) {
                pos++;
            }
            if (pos >= str.size() || str[pos] != '"') {
                ++pos;
                continue;
            }
            size_t nameStart = ++pos;
            size_t nameEnd = str.find('"', nameStart);
            if (nameEnd == std::string::npos) {
                break;
            }
            std::string filename = str.substr(nameStart, nameEnd - nameStart);
            auto path = ResourceManager::getResourcePath(filename.c_str());
            if (path.empty()) {
                std::cerr << "Shader append file not found: " << filename << "\n";
                pos = nameEnd;
                continue;
            }
            if (active.contains(filename)) {
                std::cerr << "Circular shader append detected: " << filename << "\n";
                pos = nameEnd;
                continue;
            }
            active.insert(filename);
            pos = nameEnd + 1;
            std::string appendSource = ResourceManager::getResourceAsString(filename.c_str());
            clean(appendSource);
            while (pos < str.size() && str[pos] != '\n') {
                if (str[pos] != '[') {
                    pos++;
                    continue;
                }
                size_t end = str.find(']', pos);
                if (end == std::string::npos) {
                    break;
                }
                std::string tokenList = str.substr(pos + 1, end - pos - 1);
                size_t tokenStart = 0;
                size_t tokenEnd = 0;
                while(tokenEnd != std::string::npos) {
                    tokenEnd = tokenList.find(',', tokenStart);
                    std::string token;
                    if(tokenEnd == std::string::npos) {
                        token = tokenList.substr(tokenStart);
                    } else {
                        token = tokenList.substr(tokenStart, tokenEnd);
                    }
                    tokenStart = tokenEnd + 1;
                    size_t colon = token.find(':');
                    if (colon != std::string::npos) {
                        std::string find = token.substr(0, colon);
                        trim(find);
                        std::string repl = token.substr(colon + 1);
                        trim(repl);
                        replaceAll(appendSource, find, repl);
                    }
                }
                pos = end + 1;
            }
            appendAll(appendSource, active);
            active.erase(filename);
            size_t directiveEnd = str.find('\n', directiveStart);
            if (directiveEnd == std::string::npos) {
                directiveEnd = str.size();
            }
            str.replace(directiveStart, directiveEnd - directiveStart, appendSource);
            pos = directiveStart + appendSource.size();
        }
    }
public:
    void setValue(const char* name, const char* value) {
        values[name] = value;
    }

    template<typename T>
    void setValue(const char* name, const T& value) {
        values[name] = std::to_string(value).c_str();
    }

    void removeValue(const char* name) {
        values.erase(name);
    }

    void process(std::string& source) {
        std::unordered_set<std::string> active;
        appendAll(source, active);
        replaceAll(source);
    }

    inline static void trim(std::string& str) {
        size_t start = 0;
        while (start < str.size() && std::isspace((unsigned char) str[start])) {
            start++;
        }
        size_t end = str.size();
        while (end > start && std::isspace((unsigned char) str[end - 1])) {
            end--;
        }
        str = str.substr(start, end - start);
    }

    inline static void replaceAll(std::string& str, const std::string find, const std::string replace) {
        size_t pos = 0u;
        while ((pos = str.find(find, pos)) != std::string::npos) {
            str.replace(pos, find.size(), replace);
            pos += replace.size();
        }
    }

    inline static bool findDirective(std::string& str, const char* dirName, std::string& payload) {
        std::string name = '#' + std::string(dirName);
        size_t pos = str.find(name, 0);
        if(pos == std::string::npos) {
            return false;
        }
        size_t start = pos + name.size();
        size_t end = str.find('\n', start);
        payload = str.substr(start, end - start);
        trim(payload);
        if(end != std::string::npos) {
            str.erase(pos, end - pos + 1);
        } else {
            str.erase(pos);
        }
        return true;
    }

    inline static void extractDirectiveList(const std::string& str, std::vector<std::pair<std::string, std::string>>& list) {
        if(str[0] != '[' || str[str.size() - 1] != ']') {
            std::cerr << "Error parsing directive list: " << str << std::endl;
            return;
        }
        size_t tokenStart = 0;
        size_t tokenEnd = 0;
        while(tokenEnd != std::string::npos) {
            tokenEnd = str.find(',', tokenStart);
            std::string token;
            if(tokenEnd == std::string::npos) {
                token = str.substr(tokenStart);
            } else {
                token = str.substr(tokenStart, tokenEnd);
            }
            tokenStart = tokenEnd + 1;
            size_t colon = token.find(':');
            if (colon != std::string::npos) {
                std::string a = token.substr(0, colon);
                std::string b = token.substr(colon + 1);
                trim(a);
                trim(b);
                list.emplace_back(a, b);
            }
        }
    }

    inline static void clean(std::string& str) {
        size_t pos = 0;
        while((pos = str.find("//", pos)) != std::string::npos) {
            size_t end = str.find('\n', pos);
            if(end != std::string::npos) {
                str.erase(pos, end - pos);
            } else {
                str.erase(pos);
            }
        }
        pos = 0;
        while((pos = str.find("/*", pos)) != std::string::npos) {
            size_t end = str.find("*/", pos);
            size_t c = 0;
            for(size_t i = pos; i < end; i++) {
                if(str[i] == '\n') {
                    c++;
                }
            }
            if(end != std::string::npos) {
                str.erase(pos, end - pos + 2);
            } else {
                str.erase(pos);
            }
            if(c > 0) {
                char ch[c];
                std::memset(ch, '\n', c);
                str.insert(pos, ch);
            }
        }
        pos = 0;
        while((pos = str.find('\r', pos)) != std::string::npos) {
            str.erase(pos, 1);
        }
    }
};

#endif
