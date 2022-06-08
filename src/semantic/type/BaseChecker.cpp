#include "BaseChecker.h"

std::unordered_map<std::string, std::string> BaseChecker::bases;

void BaseChecker::setBase(const std::string &me, const std::string &base) {
    auto iter = bases.find(me);
    if (iter != bases.end()) {
        iter->second = base;
    } else {
        std::pair<std::string, std::string> p = {me, base};
        bases.insert(p);
    }
}

std::string BaseChecker::getBase(const std::string &me)
{
    auto it = bases.find(me);
    if (it != bases.end()) {
        return it->second;
    } else {
        return "";
    }
}

bool BaseChecker::isBase(const std::string &me, const std::string &base) {
    auto it = bases.find(me);

    while (it != bases.end())
    {
        std::string basename = it->second;
        if (basename == base) {
            return true;
        } else {
            it = bases.find(basename);
        }
    }
    return false;
}
std::vector<std::string> BaseChecker::getBaseChain(const std::string &me) {
    std::vector<std::string> v;
    
    for (std::string cur = me; !cur.empty(); cur = getBase(cur)) {
        v.push_back(cur);
    }
    return v;
}
