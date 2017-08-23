#ifndef STRUCTURE_MANAGER_H
#define STRUCTURE_MANAGER_H

#include "Structure.h"

#include <map>

class StructureManager
{
public:
    void addStructure(const std::string& name) {
        structs[name] = Structure("assets/structures/" + name + ".str");
    }

    const Structure& getStructure(const std::string& name) {
        auto it = structs.find(name);
        if (it == structs.end()) {
            addStructure(name);
            it = structs.find(name);
        }
        return it->second;
    }

private:
    std::map<std::string, Structure> structs;
};

#endif // STRUCTURE_MANAGER_H
