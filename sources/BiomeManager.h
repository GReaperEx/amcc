#ifndef BIOME_MANAGER_H
#define BIOME_MANAGER_H

#include <vector>
#include <string>
#include <iostream>

#include "Biome.h"
#include "BlockManager.h"
#include "NoiseGenerator.h"

class BiomeManager
{
public:
    const Biome& getBiome(float occurrence) const {
        Biome searchTerm(occurrence, occurrence);
        return *std::lower_bound(biomes.begin(), biomes.end(), searchTerm);
    }

    void loadBiomeInfo() {
        std::ifstream infile("assets/biomes.cfg");
        std::string biomeName;

        std::cout << "Parsing biome config file." << std::endl;
        while (infile >> biomeName) {
            infile.ignore(100, '{');
            biomes.push_back(Biome(infile, biomeName));
        }

        std::sort(biomes.begin(), biomes.end());
    }

private:
    // There should be biomes for all occurrences
    std::vector<Biome> biomes;
};

#endif // BIOME_MANAGER_H
