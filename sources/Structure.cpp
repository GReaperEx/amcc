#include "Structure.h"

#include <fstream>

void fatalError(const std::string& prefix, const std::string& message);

Structure::Structure(const std::string& filename)
{
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        fatalError("File_Error: ", "Unable to open \'" + filename + "\'.");
    }

    std::string blockName;
    while (infile >> blockName) {
        int x, y, z;
        if (infile >> x >> y >> z) {
            blocks.push_back(Structure::Data{ glm::vec3(x, y, z), blockName });
        } else {
            fatalError("File_Error: ", "\'" + filename + "\' appears to contain invalid data.");
        }
    }
}
