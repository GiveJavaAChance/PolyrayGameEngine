#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

enum JSONType : uint8_t {
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_INT,
    JSON_FLOAT,
    JSON_STRING,
    JSON_BOOL,
    JSON_NULL,
    JSON_INVALID
};

struct JSONValue {
    JSONType type;
    union {
        std::vector<JSONValue> arr;
        std::unordered_map<std::string, JSONValue> obj;
        int i;
        float f;
        std::string str;
        bool b;
    };

    JSONValue() : type(JSON_INVALID) {
    }

    JSONValue(const std::vector<JSONValue>& a) : type(JSON_ARRAY) {
        new (&arr) std::vector<JSONValue>(a);
    }

    JSONValue(const std::unordered_map<std::string, JSONValue>& o) : type(JSON_OBJECT) {
        new (&obj) std::unordered_map<std::string, JSONValue>(o);
    }

    JSONValue(int i) : type(JSON_INT), i(i) {
    }

    JSONValue(float f) : type(JSON_FLOAT), f(f) {
    }

    JSONValue(const std::string& s) : type(JSON_STRING) {
        new (&str) std::string(s);
    }

    JSONValue(bool b) : type(JSON_BOOL), b(b) {
    }

    JSONValue(JSONType type) : type(type) {
    }

    JSONValue(const JSONValue& other) : type(other.type) {
        switch (type) {
            case JSON_ARRAY:
                new (&arr) std::vector<JSONValue>(other.arr);
                break;
            case JSON_OBJECT:
                new (&obj) std::unordered_map<std::string, JSONValue>(other.obj);
                break;
            case JSON_STRING:
                new (&str) std::string(other.str);
                break;
            case JSON_INT:
                i = other.i;
                break;
            case JSON_FLOAT:
                f = other.f;
                break;
            case JSON_BOOL:
                b = other.b;
                break;
            default:
                break;
        }
    }

    ~JSONValue() {
        destroy();
    }

    JSONValue& operator=(const JSONValue& other) {
        if (this == &other) {
            return *this;
        }
        destroy();
        type = other.type;
        switch (type) {
            case JSON_ARRAY:
                new (&arr) std::vector<JSONValue>(other.arr);
                break;
            case JSON_OBJECT:
                new (&obj) std::unordered_map<std::string, JSONValue>(other.obj);
                break;
            case JSON_STRING:
                new (&str) std::string(other.str);
                break;
            case JSON_INT:
                i = other.i;
                break;
            case JSON_FLOAT:
                f = other.f;
                break;
            case JSON_BOOL:
                b = other.b;
                break;
            default:
                break;
        }
        return *this;
    }

private:
    void destroy() {
        switch (type) {
            case JSON_ARRAY:
                arr.~vector<JSONValue>();
                break;
            case JSON_OBJECT:
                obj.~unordered_map<std::string, JSONValue>();
                break;
            case JSON_STRING:
                str.~basic_string();
                break;
            default:
                break;
        }
    }
};

using JSONArray = std::vector<JSONValue>;
using JSONObject = std::unordered_map<std::string, JSONValue>;

namespace JSON {
    namespace {
        static bool findNext(uint32_t& pos, const char* json) {
            while(char c = json[pos]) {
                if (!(c == ' ' || c == '\n' || c == '\t' || c == '\r')) {
                    return false;
                }
                pos++;
            }
            return true;
        }

        static uint32_t matchNumber(uint32_t pos, const char* json, bool& isFloat) {
            uint32_t start = pos;
            isFloat = false;
            char c = json[pos];
            if (c == '-' || c == '+') {
                pos++;
                c = json[pos];
            }
            if (c == '0') {
                pos++;
            } else if (c >= '1' && c <= '9') {
                pos++;
                while (json[pos] >= '0' && json[pos] <= '9') {
                    pos++;
                }
            } else {
                return start;
            }
            if (json[pos] == '.') {
                isFloat = true;
                pos++;
                if (!(json[pos] >= '0' && json[pos] <= '9')) {
                    return start;
                }
                while (json[pos] >= '0' && json[pos] <= '9') {
                    pos++;
                }
            }
            if (json[pos] == 'e' || json[pos] == 'E') {
                isFloat = true;
                pos++;
                if (json[pos] == '+' || json[pos] == '-') {
                    pos++;
                }
                if (!(json[pos] >= '0' && json[pos] <= '9')) {
                    return start;
                }
                while (json[pos] >= '0' && json[pos] <= '9') {
                    pos++;
                }
            }
            return pos;
        }

        static uint32_t matchString(uint32_t pos, const char* json) {
            char c = json[pos];
            if(c != '\"') {
                return pos;
            }
            pos++;
            uint32_t backslashes = 0u;
            while((c = json[pos])) {
                if(c == '\\') {
                    backslashes++;
                } else {
                    backslashes = 0u;
                }
                pos++;
                if(c == '\"' && (backslashes & 1u) == 0u) {
                    return pos;
                }
            }
            return pos;
        }

        static uint32_t matchLiteral(uint32_t pos, const char* json, const char* literal, uint32_t length) {
            for(uint32_t i = 0u; i < length; i++) {
                if(json[pos + i] != literal[i]) {
                    return pos;
                }
            }
            return pos + length;
        }

        static std::string extractString(uint32_t start, uint32_t end, const char* str) {
            if(start == end) {
                return "";
            }
            char* buffer = new char[end - start + 1u];
            uint32_t pos = 0u;
            for (uint32_t i = start; i < end; i++) {
                char c = str[i];
                if (c == '\\' && i + 1u < end) {
                    char next = str[i + 1u];
                    switch (next) {
                        case 'n' : c = '\n'; break;
                        case 'r' : c = '\r'; break;
                        case 't' : c = '\t'; break;
                        case 'b' : c = '\b'; break;
                        case 'f' : c = '\f'; break;
                        case '\\': c = '\\'; break;
                        case '"' : c =  '"'; break;
                        case '\'': c = '\''; break;
                        default  : c = next; break;
                    }
                    buffer[pos++] = c;
                    i++;
                } else {
                    buffer[pos++] = c;
                }
            }
            buffer[pos] = 0;
            return std::string(buffer);
        }

        static JSONValue parseValue(uint32_t& pos, const char* json) {
            char firstChar = json[pos];
            if(firstChar == '[') {
                pos++;
                JSONArray array;
                if(findNext(pos, json)) {
                    return JSONValue();
                }
                if(json[pos] == ']') {
                    pos++;
                    return JSONValue(array);
                }
                while(true) {
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    array.push_back(parseValue(pos, json));
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    char next = json[pos++];
                    if(next == ']') {
                        return JSONValue(array);
                    }
                    if(next != ',') {
                        return JSONValue();
                    }
                }
            }
            if(firstChar == '{') {
                pos++;
                JSONObject object;
                if(findNext(pos, json)) {
                    return JSONValue();
                }
                if(json[pos] == '}') {
                    pos++;
                    return JSONValue(object);
                }
                while(true) {
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    uint32_t stringStart = pos;
                    uint32_t stringMatch = matchString(pos, json);
                    if(stringMatch == pos) {
                        return JSONValue();
                    }
                    pos = stringMatch;
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    char next = json[pos++];
                    if(next != ':') {
                        return JSONValue();
                    }
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    object[extractString(stringStart + 1u, stringMatch - 1u, json)] = parseValue(pos, json);
                    if(findNext(pos, json)) {
                        return JSONValue();
                    }
                    next = json[pos++];
                    if(next == '}') {
                        return JSONValue(object);
                    }
                    if(next != ',') {
                        return JSONValue();
                    }
                }
            }
            uint32_t literalEnd = matchLiteral(pos, json, "null", 4u);
            if(literalEnd != pos) {
                pos = literalEnd;
                return JSONValue(JSON_NULL);
            }
            literalEnd = matchLiteral(pos, json, "true", 4u);
            if(literalEnd != pos) {
                pos = literalEnd;
                return JSONValue(true);
            }
            literalEnd = matchLiteral(pos, json, "false", 5u);
            if(literalEnd != pos) {
                pos = literalEnd;
                return JSONValue(false);
            }
            bool numberIsFloat = false;
            uint32_t numberMatch = matchNumber(pos, json, numberIsFloat);
            uint32_t stringMatch = matchString(pos, json);
            uint32_t maxEnd = numberMatch > stringMatch ? numberMatch : stringMatch;
            if(maxEnd == pos) {
                return JSONValue{};
            }
            if(numberMatch == maxEnd) {
                if (!numberIsFloat) {
                    int value = 0;
                    int sign = 1;
                    if (json[pos] == '-') {
                        sign = -1;
                        pos++;
                    }
                    while (pos < maxEnd) {
                        value = value * 10 + (json[pos++] - '0');
                    }
                    return JSONValue(value * sign);
                } else {
                    double value = 0.0;
                    double sign = 1.0;
                    if (json[pos] == '-') {
                        sign = -1.0;
                        pos++;
                    }
                    while (pos < maxEnd && json[pos] >= '0' && json[pos] <= '9') {
                        value = value * 10.0 + (json[pos++] - '0');
                    }
                    if (pos < maxEnd && json[pos] == '.') {
                        pos++;
                        double scale = 0.1;
                        while (pos < maxEnd && json[pos] >= '0' && json[pos] <= '9') {
                            value += (json[pos++] - '0') * scale;
                            scale *= 0.1;
                        }
                    }
                    if (pos < maxEnd && (json[pos] == 'e' || json[pos] == 'E')) {
                        pos++;
                        bool expSign = true;
                        if (json[pos] == '+') {
                            pos++;
                        }
                        else if (json[pos] == '-') {
                            expSign = false;
                            pos++;
                        }
                        int exponent = 0;
                        while (pos < maxEnd) {
                            exponent = exponent * 10 + (json[pos++] - '0');
                        }
                        double pow10 = 1.0;
                        for (int i = 0; i < exponent; i++) {
                            pow10 *= 10.0;
                        }
                        value = expSign ? value * pow10 : value / pow10;
                    }
                    return JSONValue(static_cast<float>(value * sign));
                }
            }
            std::string str = extractString(pos + 1u, maxEnd - 1u, json);
            pos = maxEnd;
            return JSONValue(str);
        }
    }

    static JSONValue parseJSON(const char* json) {
        uint32_t pos = 0u;
        if(findNext(pos, json)) {
            return JSONValue{};
        }
        return parseValue(pos, json);
    }
};

#endif