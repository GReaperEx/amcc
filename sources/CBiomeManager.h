#ifndef C_BIOME_MANAGER_H
#define C_BIOME_MANAGER_H

#include <vector>
#include <string>
#include <iostream>

#include "CBiome.h"
#include "CBlockInfo.h"
#include "CNoiseGenerator.h"

class CBiomeManager
{
public:
    const CBiome& getBiome(float occurrence) const {
        auto it = biomes.begin();
        for (; it != biomes.end(); ++it) {
            if (it->hasDesiredOccurrence(occurrence)) {
                break;
            }
        }
        return *it;
    }

    void loadBiomeInfo() {
        std::ifstream infile("assets/biomes/biomes.cfg");
        std::string biomeName;

        std::cout << "Parsing biome config file." << std::endl;
        while (infile >> biomeName) {
            infile.ignore(100, '{');
            biomes.push_back(CBiome(infile, biomeName));
        }
    }

private:
    // There should be biomes for all occurrences
    std::vector<CBiome> biomes;
};

#endif // C_BIOME_MANAGER_H
