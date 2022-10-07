#include "lfast_comms.h"

#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <iterator>
#include <regex>
#include <iostream>
#include <map>


std::string LFAST::MessageGenerator::getMessageStr()
{
    bool objFlag = false;
    std::stringstream ss;
    ss << "{";

    if (this->DestIdStr != "")
    {
        objFlag = true;
        ss << std::quoted(DestIdStr) << ":";
    }

    if (this->argStrings.size() > 0)
    {
        if (objFlag)
            ss << "{";
        auto itr = this->argStrings.begin();
        while (itr != this->argStrings.end())
        {
            ss << *itr;
            if (itr++ < this->argStrings.end() - 1)
            {
                ss << ",";
            }
        }
        if (objFlag)
            ss << "}";
    }
    else
    {
        ss << std::quoted("");
    }
    ss << "}";

    return ss.str();
}
const char* LFAST::MessageGenerator::getMessageCStr()
{
    auto msgStr = this->getMessageStr();
    return msgStr.c_str();
}

bool LFAST::isObject(std::string str)
{
    std::regex rgx(R"(^\{.*\}$)");
    std::smatch m;
    return (std::regex_search(str, m, rgx));
}

void LFAST::MessageParser::parseKeyValuePair(std::string *kvStr)
{
    bool resultFlag = false;
    if(this->parsingStatus == ParsingStatus::PARSING_IN_PROGRESS)
    {
        std::regex r(R"(^\"(\w+)\":(.+)$)");
        std::smatch m;
        if (std::regex_search(*kvStr, m, r))
        {
            while (!m.ready())
            {
            }
            if (m.size() == 3)
            {
                auto keyStr = m[1].str();
                auto valStr = m[2].str();
                this->data[keyStr] = valStr;
#if OUTPUT_DEBUG_INFO
                std::cout << __LINE__ << ":\t\t\tParsed kv pair: " << keyStr << ":::::" << valStr << std::endl;
#endif
                *kvStr = "";
                resultFlag = true;
            }
        }
    }
    if(!resultFlag) this->parsingStatus = ParsingStatus::PARSING_FAILURE;
}

// bool LFAST::MessageParser::parseKeyValuePair(std::string *kvStr, std::string *keybuff, std::string *valbuff)
// {
//     std::pair<std::string, std::string> result;
//     std::regex r(R"(^\"(\w+)\":(.+)$)");
//     std::smatch m;
//     if (std::regex_search(*kvStr, m, r))
//     {
//         while (!m.ready())
//         {
//         }
//         if (m.size() == 3)
//         {
//             *keybuff = m[1].str();
//             *valbuff = m[2].str();
//             *kvStr = "";
//             return true;
//         }
//     }
//     return false;
// }

bool LFAST::isKey(std::string const &str)
{
    auto c = str.back();
    if (c == ':')
        return true;
    else
        return false;
}

std::string LFAST::cleanupKey(std::string const &inStr)
{
    auto outStr = inStr;
    outStr.erase(std::remove(outStr.begin(), outStr.end(), '\"'), outStr.end());
    return outStr;
}

std::string extractLeadingValString(std::string *inBuff)
{
    std::smatch m;
    std::regex re;
    re = (R"((\w+),(.*))");
    if (std::regex_search(*inBuff, m, re))
    {
        while (!m.ready())
            ;
        auto retVal = m[1].str();
        *inBuff = m[2].str();

        std::cout << __LINE__ << ":\t Extracted: " << retVal << std::endl;
        std::cout << __LINE__ << ":\t Remaining: " << *inBuff << std::endl;
        return retVal;
    }
    else
    {
        auto inBuffCopy = *inBuff;
        *inBuff = "";
        return inBuffCopy;
    }
}

void LFAST::MessageParser::parseMessageBuffer(std::string *inBuff)
{
    // if(this->depth > MAX_DEPTH )
    // {
    //     std::cout << __LINE__ << ": MAX DEPTH EXCEEDED(" << depth << ")\n";
    //     this->parsingStatus = ParsingStatus::MAX_DEPTH_EXCEEDED;
    //     return;
    // }


    this->parsingStatus = ParsingStatus::PARSING_IN_PROGRESS;
    std::string keyStr = {0}, valStr = {0};

    // Extract outer-most object in inBuff (anything inside a matched set of curly braces)
    std::smatch m;
    std::regex re;
    re = (R"(^\{\"(\w+)\":(.*)\}$)");
    if (std::regex_search(*inBuff, m, re))
    {
        while (!m.ready())
            ;
#if OUTPUT_DEBUG_INFO
        std::cout << __LINE__ <<  ": Parsing: ";
        for (unsigned ii = 1; ii < m.size(); ii++)
            std::cout << "<" << m[ii] << ">";
        std::cout << std::endl;
#endif
        auto preColonBuff = m[1].str();
        auto postColonBuff = m[2].str();
        keyStr = preColonBuff;

        if (isObject(postColonBuff))
        {
#if OUTPUT_DEBUG_INFO
            std::cout <<  __LINE__ << ": Found an object under parent " << keyStr << ": " << postColonBuff << std::endl;
#endif
            valStr = postColonBuff;
            this->data[keyStr] = valStr;
            this->childNode = new MessageParser(postColonBuff, this);
            if(this->childNode->parsingStatus != ParsingStatus::PARSING_SUCCESS)
            {
                this->parsingStatus = this->childNode->parsingStatus;
                return;
            }
        }
        else
        {
            if (postColonBuff.size() > 0)
            {
#if OUTPUT_DEBUG_INFO
                std::cout << __LINE__ << ": Key: " << keyStr << "Buff contents not an object: "  << ": " << postColonBuff << std::endl;
#endif
                do
                {
                    std::string newValStr1 = extractLeadingValString(&postColonBuff);
                    this->data[keyStr] = newValStr1;
#if OUTPUT_DEBUG_INFO
                    std::cout << __LINE__ << ": Added <" << keyStr << ":" << newValStr1 << ">"
                              << "to key/value pair(s) [" << depth << "]" << std::endl;
#endif
                    if (postColonBuff.size() > 0)
                    {
                        this->parseMessageBuffer(&postColonBuff);
                    }
                    else
                    {
#if OUTPUT_DEBUG_INFO
                        std::cout << __LINE__ <<  ": We're done here.\n";
#endif
                        this->childNode = nullptr;
                    }
                }
                while (postColonBuff.size() > 0);
            }
            else
            {
#if OUTPUT_DEBUG_INFO
                std::cout << __LINE__ << ": Buffer empty" << std::endl;
#endif
            }
        }
    }
    else
    {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string token;
        unsigned count = 0;
        while ((pos = inBuff->find(delimiter)) != std::string::npos)
        {
            token = inBuff->substr(0, pos);
            inBuff->erase(0, pos + delimiter.length());
            parseKeyValuePair(&token);
            if( this->parsingStatus == ParsingStatus::PARSING_FAILURE ) break;
#if OUTPUT_DEBUG_INFO
            else std::cout << __LINE__ << ":\t\t\t" << inBuff->size() << "Chars remaining in buffer: " << *inBuff << std::endl;
#endif
        }
        parseKeyValuePair(inBuff);

#if OUTPUT_DEBUG_INFO
        if(this->parsingStatus == ParsingStatus::PARSING_FAILURE) std::cout << "\t\t\tFAILED TO PARSE." << std::endl;
#endif
    }
    if(this->parsingStatus == ParsingStatus::PARSING_IN_PROGRESS)
    {
        this->parsingStatus =  ParsingStatus::PARSING_SUCCESS;
    }
    this->printParsingStatusInfo();
}


void LFAST::MessageParser::printMessage()
{
    std::cout << std::endl;
}

void LFAST::print_map(std::map<std::string, std::string> const &m)
{
    std::for_each(m.begin(),
                  m.end(),
                  [](const std::pair<std::string, std::string> &p)
    {
        std::cout << "{" << p.first << ": " << p.second << "}\n";
    });
}

void LFAST::MessageParser::printParsingStatusInfo()
{
    switch(this->parsingStatus)
    {
        case ParsingStatus::PARSING_INACTIVE:
            std::cout << "PARSING_INACTIVE" << std::endl;
            break;
        case ParsingStatus::PARSING_IN_PROGRESS:
            std::cout << "PARSING_IN_PROGRESS" << std::endl;
            break;
        case ParsingStatus::PARSING_SUCCESS:
            std::cout << "PARSING_SUCCESS" << std::endl;
            break;
        case ParsingStatus::PARSING_FAILURE:
            std::cout << "PARSING_FAILURE" << std::endl;
            break;
        default:
            std::cout << "Something weird." << std::endl;
            break;
    }
}


std::string LFAST::MessageParser::find(std::string const &keyStr)
{
    auto val = this->data[keyStr];
    if(val.size() == 0)
    {
        if(this->childNode)
        {
            val = this->childNode->find(keyStr);
        }
    }

    // std::string matchValStr = this->data[keyStr];

    // std::cout << "\n\n" <<  __LINE__ << ": Key: " << keyStr << ", Value: " << matchValStr << "\n";
    // std::cout << "size: " << matchValStr.size() << "\n\n";

    return val;
}

LFAST::MessageParser::~MessageParser()
{
    if(this->childNode != nullptr)
    {
        delete this->childNode;
    }
}