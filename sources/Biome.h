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
    //! \return Potential structure to be generated on-top. Empty string if none.
    const std::string genChunkColumn(void *column, const BlockManager& blockManager, int surfaceHeight, int x, int z, const NoiseGenerator& noiseGen) const;

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

    struct GenerationStructs
    {
        float frequency;
        float offset;
        std::string structName;
    };
    std::vector<GenerationStructs> genStructs;

    // These shouldn't overlap with any other biomes
    float minOccurrence;
    float maxOccurrence;

    float calcNoise(int x, int z, const std::vector<Wave>& waves, const NoiseGenerator& noiseGen) const {
        float result = 0.0f;

        for (Wave curWave : waves) {
            result += glm::pow(noiseGen.noise((x + curWave.offset)*curWave.frequency,
                                              0.0f,
                                              (z + curWave.offset)*curWave.frequency), curWave.exponent)*curWave.amplitude;
        }

        return result;
    }

    bool isAtPeak(int x, int z, const GenerationStructs& genStruct, const NoiseGenerator& noiseGen) const {
        float baseNoise = noiseGen.noise((x + genStruct.offset)*genStruct.frequency, 0.0f, (z + genStruct.offset)*genStruct.frequency);
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (i != 0 || j != 0) {
                    if (baseNoise <= noiseGen.noise((x + i + genStruct.offset)*genStruct.frequency, 0.0f, (z + j + genStruct.offset)*genStruct.frequency)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

#endif // BIOME_H
