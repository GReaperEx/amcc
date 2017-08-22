#include "Biome.h"
#include "Chunk.h"

void fatalError(const std::string& prefix, const std::string& message);

Biome::Biome(std::ifstream& infile, const std::string& name)
{
    this->name = name;

    minHeight = maxHeight = 0;
    minOccurrence = maxOccurrence = 0;

    std::string property;
    while (infile >> property && property != "}") {
        if (property == "minHeight") {
            infile >> minHeight;
        } else if (property == "maxHeight") {
            infile >> maxHeight;
        } else if (property == "minOccurrence") {
            infile >> minOccurrence;
        } else if (property == "maxOccurrence") {
            infile >> maxOccurrence;
        } else if (property == "genOctaves") {
            infile.ignore(100, '{');
            float freq, exp, amp, offset;
            while (infile >> freq >> exp >> amp >> offset) {
                genOctaves.push_back(Wave{ freq, exp, amp, offset });
            }
            if (!infile.eof()) {
                infile.clear();
            }
            infile.ignore(100, '}');
        } else if (property == "layers") {
            infile.ignore(100, '{');
            std::string blockName;
            while (infile >> blockName && blockName != "}") {
                int maxDepth;
                infile >> maxDepth;
                layers.push_back(GenerationLayer{ maxDepth, blockName });
            }
        }
    }

    if (minOccurrence == maxOccurrence) {
        fatalError("Biome_Error: ", "Min and max occurrences can't be the same.");
    }
    if (genOctaves.empty()) {
        fatalError("Biome_Error: ", "Needs at least one octave.");
    }
    if (layers.empty()) {
        fatalError("Biome_Error: ", "Needs at least one layer.");
    }
    if (!infile) {
        fatalError("File_Error: ", "Configuration file is malformed.");
    }

    std::sort(layers.begin(), layers.end(), [](const GenerationLayer& a, const GenerationLayer& b) {
        return a.maxDepth < b.maxDepth;
    });
}

void Biome::genChunkColumn(void *column, const BlockInfo& blockInfo, int surfaceHeight) const
{
    auto it = layers.begin();
    Chunk::SBlock curBlock{ 0, 0 };
    curBlock.id = blockInfo.getBlockID(it->blockName);

    for (int i = surfaceHeight; i >= 0; --i) {
        while (it->maxDepth < (surfaceHeight - i)) {
            curBlock.id = blockInfo.getBlockID((++it)->blockName);
        }
        ((Chunk::SBlock*)column)[i] = curBlock;
    }
}
