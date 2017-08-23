#ifndef BIOME_H
#define BIOME_H

#include "BlockManager.h"
#include "NoiseGenerator.h"

#include <fstream>
#include <string>

class Biome
{
public:
    Biome(std::ifstream& infile, const std::string& name);
    Biome(float minOccur, float maxOccur) {
        minOccurrence = minOccur;
        maxOccurrence = maxOccur;
    }

    // @column: Should actually be Chunk::SBlock
    // TODO: Find a more elegant way to circumvent that circular dependency
    void genChunkColumn(void *column, const BlockManager& blockManager, int surfaceHeight) const;

    bool hasDesiredOccurrence(float occurrence) const {
        return minOccurrence <= occurrence && occurrence < maxOccurrence;
    }

    int calcSurfaceHeight(int x, int z, const NoiseGenerator& noiseGen) const {
        float genNoise = calcNoise(x, z, genOctaves, noiseGen);
        return (int)glm::clamp((int)(minHeight + genNoise), minHeight, maxHeight);
    }

    const std::string getName() const {
        return name;
    }

    // Needed for sorting and binary search
    bool operator< (const Biome& other) const {
        return maxOccurrence <= other.minOccurrence;
    }

private:
    std::string name;
    int minHeight;     // Default: 0
    int maxHeight;     // Default: 0

    struct Wave
    {
        float frequency;
        float exponent;
        float amplitude;
        float offset;
    };
    std::vector<Wave> genOctaves;

    // Layers all-together must span the whole available depth
    struct GenerationLayer
    {
        int maxDepth;
        std::string blockName;
    };
    std::vector<GenerationLayer> layers;

    // These shouldn't overlap with any other biomes
    float minOccurrence;
    float maxOccurrence;

    float calcNoise(int x, int z, const std::vector<Wave>& waves, const NoiseGenerator& noiseGen) const {
        float result = 0.0f;

        for (Wave curWave : waves) {
            result += glm::pow(noiseGen.noise(x*curWave.frequency + curWave.offset,
                                              0.0f,
                                              z*curWave.frequency + curWave.offset), curWave.exponent)*curWave.amplitude;
        }

        return result;
    }
};

#endif // BIOME_H