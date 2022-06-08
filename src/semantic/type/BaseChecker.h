#ifndef _SUB_CHECKER_H_
#define _SUB_CHECKER_H_

#include <unordered_map>
#include <vector>

class BaseChecker {
public:
    BaseChecker() = delete;
    static void setBase(const std::string &me, const std::string &base);
    static std::string getBase(const std::string &me);
    static bool isBase(const std::string &me, const std::string &base);
    static std::vector<std::string> getBaseChain(const std::string &me);

private:
    static std::unordered_map<std::string, std::string> bases;
};
#endif