#ifndef C_BIOME_H
#define C_BIOME_H

#include "CBlockInfo.h"
#include "CNoiseGenerator.h"

#include <fstream>
#include <string>

class CBiome
{
public:
    CBiome(std::ifstream& infile, const std::string& name);

    // @column: Should actually be CChunk::SBlock
    // TODO: Find a more elegant way to circumvent that circular dependency
    void genChunkColumn(void *column, int x, int z, const CBlockInfo& blockInfo, const CNoiseGenerator& noiseGen) const;

    bool hasDesiredOccurrence(float occurrence) const {
        return minOccurrence <= occurrence && occurrence < maxOccurrence;
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

    float calcNoise(int x, int z, const std::vector<Wave>& waves, const CNoiseGenerator& noiseGen) const {
        float result = 0.0f;

        for (Wave curWave : waves) {
            result += glm::pow(noiseGen.noise(x*curWave.frequency + curWave.offset,
                                              0.0f,
                                              z*curWave.frequency + curWave.offset), curWave.exponent)*curWave.amplitude;
        }

        return result;
    }
};

#endif // C_BIOME_H
