#include "ChunkTree.h"

#include <sstream>

#include <cstdio>

#include <zlib.h>

void ChunkTree::addChunk(const glm::vec3& position)
{
    std::lock_guard<std::mutex> lck(rootBeingModified);
    if (root == nullptr) {
        root = new TreeLeafNode;
        memset(root, 0, sizeof(TreeLeafNode));
        root->node.center = position;
        root->node.boundaries = utils3d::AABBox(position, position + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));
    }

    addLeaf(root, position);
}

void ChunkTree::remChunk(const glm::vec3& position)
{
    std::lock_guard<std::mutex> lck(rootBeingModified);
    if (root != nullptr) {
        remLeaf(root, position);

        // Re-balancing tree
        while (true) {
            int subCount = 0;
            int lastX = -1, lastY = -1, lastZ = -1;
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    for (int k = 0; k < 2; ++k) {
                        if (root->node.subdivisions[i][j][k]) {
                            ++subCount;
                            if (!root->node.subdivisions[i][j][k]->isLeaf) {
                                lastX = i;
                                lastY = j;
                                lastZ = k;
                            }
                        }
                    }
                }
            }

            if (subCount == 1 && lastX >= 0) {
                TreeLeafNode *temp = root->node.subdivisions[lastX][lastY][lastZ];
                delete root;
                root = temp;
            } else {
                break;
            }
        }

        // Deleting last empty node
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    if (root->node.subdivisions[i][j][k] != nullptr) {
                        return;
                    }
                }
            }
        }

        delete root;
        root = nullptr;
    }
}

void ChunkTree::genNewChunks(const utils3d::AABBox& activeArea)
{
    utils3d::AABBox treeArea;
    if (root) {
        treeArea = root->node.boundaries;
    }

    for (int i = activeArea.minVec.x; i < activeArea.maxVec.x; i += Chunk::CHUNK_WIDTH) {
        for (int j = activeArea.minVec.y; j < activeArea.maxVec.y; j += Chunk::CHUNK_HEIGHT) {
            for (int k = activeArea.minVec.z; k < activeArea.maxVec.z; k += Chunk::CHUNK_DEPTH) {
                utils3d::AABBox chunkBox(glm::vec3(i, j, k), glm::vec3(i+Chunk::CHUNK_WIDTH, j+(int)Chunk::CHUNK_HEIGHT, k+Chunk::CHUNK_DEPTH));
                if (!utils3d::AABBcollision(chunkBox, treeArea) || getChunk(chunkBox.minVec, ALL) == nullptr) {
                    if (!loadChunk(chunkBox.minVec)) {
                        addChunk(chunkBox.minVec);
                    }
                }
            }
        }
    }
}

void ChunkTree::eraseOldChunks(const utils3d::AABBox& activeArea)
{
    utils3d::AABBox treeArea;
    if (root) {
        treeArea = root->node.boundaries;
    }

    for (int i = treeArea.minVec.x; i < treeArea.maxVec.x; i += Chunk::CHUNK_WIDTH) {
        for (int j = treeArea.minVec.y; j < treeArea.maxVec.y; j += Chunk::CHUNK_HEIGHT) {
            for (int k = treeArea.minVec.z; k < treeArea.maxVec.z; k += Chunk::CHUNK_DEPTH) {
                utils3d::AABBox chunkBox(glm::vec3(i, j, k), glm::vec3(i+Chunk::CHUNK_WIDTH, j+Chunk::CHUNK_HEIGHT, k+Chunk::CHUNK_DEPTH));
                if (!utils3d::AABBcollision(chunkBox, activeArea)) {
                    Chunk* oldChunk = getChunk(chunkBox.minVec);
                    if (oldChunk && oldChunk->wasChunkEdited()) {
                        saveChunk(chunkBox.minVec);
                    }
                    remChunk(chunkBox.minVec);
                }
            }
        }
    }
}

void ChunkTree::addLeaf(TreeLeafNode* node, const glm::vec3& position, TreeLeafNode* leaf)
{
    glm::vec3 nodeCenter = node->node.center;
    int indX, indY, indZ;

    if (position.x >= nodeCenter.x) {
        indX = 1;
    } else {
        indX = 0;
    }
    if (position.y >= nodeCenter.y) {
        indY = 1;
    } else {
        indY = 0;
    }
    if (position.z >= nodeCenter.z) {
        indZ = 1;
    } else {
        indZ = 0;
    }

    node->node.boundaries.addPoint(position);
    node->node.boundaries.addPoint(position + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));

    if (node->node.subdivisions[indX][indY][indZ] == nullptr) {
        if (leaf != nullptr) {
            node->node.subdivisions[indX][indY][indZ] = leaf;
        } else {
            TreeLeafNode *newNode = new TreeLeafNode;
            newNode->isLeaf = true;
            newNode->leaf.chunk = new Chunk(position);
            node->node.subdivisions[indX][indY][indZ] = newNode;

            std::lock_guard<std::mutex> lck(initedBeingModified);
            chunksToInit.push_back(newNode->leaf.chunk);
        }
    } else if (node->node.subdivisions[indX][indY][indZ]->isLeaf) {
        if (node->node.subdivisions[indX][indY][indZ]->leaf.chunk->getPosition() == position) {
            return;
        }

        TreeLeafNode *newNode = new TreeLeafNode;
        memset(newNode, 0, sizeof(TreeLeafNode));
        newNode->node.center = position;
        newNode->node.boundaries = utils3d::AABBox(position, position);
        addLeaf(newNode, position, leaf);
        addLeaf(newNode, node->node.subdivisions[indX][indY][indZ]->leaf.chunk->getPosition(), node->node.subdivisions[indX][indY][indZ]);
        node->node.subdivisions[indX][indY][indZ] = newNode;

    } else {
        addLeaf(node->node.subdivisions[indX][indY][indZ], position);
    }
}

void ChunkTree::remLeaf(TreeLeafNode* node, const glm::vec3& position)
{
    glm::vec3 nodeCenter = node->node.center;
    int indX, indY, indZ;

    if (position.x >= nodeCenter.x) {
        indX = 1;
    } else {
        indX = 0;
    }
    if (position.y >= nodeCenter.y) {
        indY = 1;
    } else {
        indY = 0;
    }
    if (position.z >= nodeCenter.z) {
        indZ = 1;
    } else {
        indZ = 0;
    }

    TreeLeafNode* temp = node->node.subdivisions[indX][indY][indZ];
    if (temp != nullptr) {
        if (temp->isLeaf) {
            if (temp->leaf.chunk->getPosition() == position) {
                std::lock_guard<std::mutex> lck(erasedBeingModified);
                chunksToErase.push_back(ChunkTimeBundle(temp->leaf.chunk));
                delete temp;
                node->node.subdivisions[indX][indY][indZ] = nullptr;
            }
        } else {
            remLeaf(temp, position);

            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    for (int k = 0; k < 2; ++k) {
                        if (temp->node.subdivisions[i][j][k] != nullptr) {
                            return;
                        }
                    }
                }
            }
            delete temp;
            node->node.subdivisions[indX][indY][indZ] = nullptr;
        }
    }

    // Resizing bounding box
    node->node.boundaries.minVec = node->node.boundaries.maxVec = node->node.center;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k] != nullptr) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        glm::vec3 chunkPos = node->node.subdivisions[i][j][k]->leaf.chunk->getPosition();
                        node->node.boundaries.addPoint(chunkPos);
                        node->node.boundaries.addPoint(chunkPos + glm::vec3((int)Chunk::CHUNK_WIDTH, (int)Chunk::CHUNK_HEIGHT, (int)Chunk::CHUNK_DEPTH));
                    } else {
                        node->node.boundaries.addPoint(node->node.subdivisions[i][j][k]->node.boundaries.minVec);
                        node->node.boundaries.addPoint(node->node.subdivisions[i][j][k]->node.boundaries.maxVec);
                    }
                }
            }
        }
    }
}

void ChunkTree::deleteAll(TreeLeafNode* node)
{
    if (node) {
        if (node->isLeaf) {
            std::lock_guard<std::mutex> lck(erasedBeingModified);
            chunksToErase.push_back(ChunkTimeBundle(node->leaf.chunk));
        } else {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    for (int k = 0; k < 2; ++k) {
                        deleteAll(node->node.subdivisions[i][j][k]);
                    }
                }
            }
        }
        delete node;
    }
}

ChunkTree::TreeLeafNode* ChunkTree::getLeaf(TreeLeafNode* node, const glm::vec3& pos, EChunkFlags flags) const
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k]) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        if (node->node.subdivisions[i][j][k]->leaf.chunk->getPosition() == pos) {
                            Chunk* temp = node->node.subdivisions[i][j][k]->leaf.chunk;
                            bool flagsPass;
                            bool isInited = temp->isStateInitialized();
                            flagsPass = ((flags & ChunkTree::UNINITIALIZED) && !isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::INITIALIZED) && isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_GENERATION) && isInited && !temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::GENERATED) && isInited && temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_MESH_UPDATE) && isInited && temp->chunkNeedsMeshUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_STATE_UPDATE) && isInited && temp->chunkNeedsStateUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::RENDERABLE) && isInited && temp->isChunkRenderable());
                            flagsPass = flagsPass || ((flags & ChunkTree::EDITED) && isInited && temp->wasChunkEdited());

                            if (flagsPass) {
                                return node->node.subdivisions[i][j][k];
                            }
                        }
                    } else {
                        glm::vec3 temp(1.0f, 1.0f, 1.0f);
                        utils3d::AABBox box(pos + temp, pos - temp + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));
                        if (utils3d::AABBcollision(box, node->node.subdivisions[i][j][k]->node.boundaries)) {
                            return getLeaf(node->node.subdivisions[i][j][k], pos, flags);
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

void ChunkTree::getLeafArea(TreeLeafNode* node, std::vector<Chunk*>& output, const utils3d::AABBox& area, EChunkFlags flags) const
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k]) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        glm::vec3 chunkPos = node->node.subdivisions[i][j][k]->leaf.chunk->getPosition();
                        utils3d::AABBox box(chunkPos, chunkPos + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));
                        if (utils3d::AABBcollision(area, box)) {
                            Chunk* temp = node->node.subdivisions[i][j][k]->leaf.chunk;
                            bool flagsPass;
                            bool isInited = temp->isStateInitialized();
                            flagsPass = ((flags & ChunkTree::UNINITIALIZED) && !isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::INITIALIZED) && isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_GENERATION) && isInited && !temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::GENERATED) && isInited && temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_MESH_UPDATE) && isInited && temp->chunkNeedsMeshUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_STATE_UPDATE) && isInited && temp->chunkNeedsStateUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::RENDERABLE) && isInited && temp->isChunkRenderable());
                            flagsPass = flagsPass || ((flags & ChunkTree::EDITED) && isInited && temp->wasChunkEdited());

                            if (flagsPass) {
                                output.push_back(node->node.subdivisions[i][j][k]->leaf.chunk);
                            }
                        }
                    } else {
                        if (utils3d::AABBcollision(area, node->node.subdivisions[i][j][k]->node.boundaries)) {
                            getLeafArea(node->node.subdivisions[i][j][k], output, area, flags);
                        }
                    }
                }
            }
        }
    }
}

void ChunkTree::getIntersectingLeafs(TreeLeafNode* node, std::vector<Chunk*>& output, const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, EChunkFlags flags) const
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k]) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        glm::vec3 chunkPos = node->node.subdivisions[i][j][k]->leaf.chunk->getPosition();
                        utils3d::AABBox box(chunkPos, chunkPos + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));
                        if (utils3d::RayAABBcollision(rayPos, rayDir_inverted, box)) {
                            Chunk* temp = node->node.subdivisions[i][j][k]->leaf.chunk;
                            bool flagsPass;
                            bool isInited = temp->isStateInitialized();
                            flagsPass = ((flags & ChunkTree::UNINITIALIZED) && !isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::INITIALIZED) && isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_GENERATION) && isInited && !temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::GENERATED) && isInited && temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_MESH_UPDATE) && isInited && temp->chunkNeedsMeshUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_STATE_UPDATE) && isInited && temp->chunkNeedsStateUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::RENDERABLE) && isInited && temp->isChunkRenderable());
                            flagsPass = flagsPass || ((flags & ChunkTree::EDITED) && isInited && temp->wasChunkEdited());

                            if (flagsPass) {
                                output.push_back(node->node.subdivisions[i][j][k]->leaf.chunk);
                            }
                        }
                    } else {
                        if (utils3d::RayAABBcollision(rayPos, rayDir_inverted, node->node.subdivisions[i][j][k]->node.boundaries)) {
                            getIntersectingLeafs(node->node.subdivisions[i][j][k], output, rayPos, rayDir_inverted, flags);
                        }
                    }
                }
            }
        }
    }
}

void ChunkTree::getFrustumLeafs(TreeLeafNode* node, std::vector<Chunk*>& output, const utils3d::Frustum& frustum, EChunkFlags flags) const
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k]) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        glm::vec3 chunkPos = node->node.subdivisions[i][j][k]->leaf.chunk->getPosition();
                        utils3d::AABBox box(chunkPos, chunkPos + glm::vec3((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH));
                        if (utils3d::FrustumAABBcollision(frustum, box)) {
                            Chunk* temp = node->node.subdivisions[i][j][k]->leaf.chunk;
                            bool flagsPass;
                            bool isInited = temp->isStateInitialized();
                            flagsPass = ((flags & ChunkTree::UNINITIALIZED) && !isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::INITIALIZED) && isInited);
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_GENERATION) && isInited && !temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::GENERATED) && isInited && temp->isChunkGenerated());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_MESH_UPDATE) && isInited && temp->chunkNeedsMeshUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::NEED_STATE_UPDATE) && isInited && temp->chunkNeedsStateUpdate());
                            flagsPass = flagsPass || ((flags & ChunkTree::RENDERABLE) && isInited && temp->isChunkRenderable());
                            flagsPass = flagsPass || ((flags & ChunkTree::EDITED) && isInited && temp->wasChunkEdited());

                            if (flagsPass) {
                                output.push_back(node->node.subdivisions[i][j][k]->leaf.chunk);
                            }
                        }
                    } else {
                        if (utils3d::FrustumAABBcollision(frustum, node->node.subdivisions[i][j][k]->node.boundaries)) {
                            getFrustumLeafs(node->node.subdivisions[i][j][k], output, frustum, flags);
                        }
                    }
                }
            }
        }
    }
}

bool ChunkTree::loadChunk(const glm::vec3& pos)
{
    Chunk::SBlock bakedData[Chunk::CHUNK_WIDTH][Chunk::CHUNK_DEPTH][Chunk::CHUNK_HEIGHT];
    bool wasGenerated;

    std::vector<uint8_t> deflatedData;
    loadFromBundle(pos, deflatedData, wasGenerated);
    if (deflatedData.empty()) {
        return false;
    }

    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = deflatedData.size();
    infstream.next_in = (Bytef*)(&deflatedData[0]);
    infstream.avail_out = sizeof(bakedData);
    infstream.next_out = (Bytef*)bakedData;

    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    Chunk *loadedChunk = getChunk(pos, ALL);
    if (!loadedChunk) {
        addChunk(pos);
        loadedChunk = getChunk(pos, ALL);
    }

    loadedChunk->setBlockData((Chunk::SBlock*)bakedData, wasGenerated);

    return true;
}

void ChunkTree::saveChunk(const glm::vec3& pos)
{
    Chunk::SBlock bakedData[Chunk::CHUNK_WIDTH][Chunk::CHUNK_DEPTH][Chunk::CHUNK_HEIGHT];
    uint8_t deflatedData[Chunk::CHUNK_WIDTH*Chunk::CHUNK_DEPTH*Chunk::CHUNK_HEIGHT*sizeof(Chunk::SBlock)];

    Chunk *chunkToSave = getChunk(pos, ALL);
    chunkToSave->getBlockData((Chunk::SBlock*)bakedData);

    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = sizeof(bakedData);
    defstream.next_in = (Bytef *)bakedData;
    defstream.avail_out = sizeof(deflatedData);
    defstream.next_out = (Bytef *)deflatedData;

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    saveToBundle(pos, std::vector<uint8_t>(deflatedData, deflatedData + defstream.total_out), chunkToSave->isChunkGenerated());
}

void fatalError(const std::string& prefix, const std::string& msg);

void ChunkTree::loadFromBundle(const glm::vec3& chunkPos, std::vector<uint8_t>& deflatedData, bool& wasGenerated) const
{
    glm::vec3 regionIndex = glm::floor(chunkPos/glm::vec3(Chunk::CHUNK_WIDTH*256, Chunk::CHUNK_HEIGHT*256, Chunk::CHUNK_DEPTH*256));
    glm::vec3 chunkIndex = glm::floor(chunkPos/glm::vec3(Chunk::CHUNK_WIDTH, Chunk::CHUNK_HEIGHT, Chunk::CHUNK_DEPTH)) - regionIndex*16.0f;

    std::stringstream formatBuffer;
    formatBuffer << "region_" << (int)regionIndex.x << '_' << (int)regionIndex.y << '_' << (int)regionIndex.z;
    std::string regionName = formatBuffer.str();
    formatBuffer.str("");
    formatBuffer << "chunk_" << (int)chunkIndex.x << '_' << (int)chunkIndex.y << '_' << (int)chunkIndex.z;
    std::string chunkName = formatBuffer.str();

    deflatedData.clear();

    std::ifstream infile("chunks/" + regionName, std::ios::binary);
    if (infile.is_open()) {
        uint8_t nameSize;
        std::string name;
        int deflatedSize;
        std::vector<uint8_t> deflated;
        char wasGened;

        for (;;) {
            if (!infile.read((char*)(&nameSize), sizeof(nameSize))) {
                return;
            }
            name.resize(nameSize);
            if (!infile.read(&name[0], nameSize)) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (!infile.read((char*)(&deflatedSize), sizeof(deflatedSize))) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }
            deflated.resize(deflatedSize);
            if (!infile.read((char*)(&deflated[0]), deflatedSize)) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (!infile.read((char*)(&wasGened), sizeof(wasGened))) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (chunkName == name) {
                break;
            }
        }
        deflatedData = deflated;
        wasGenerated = (bool)wasGened;
    }
}

void ChunkTree::saveToBundle(const glm::vec3& chunkPos, const std::vector<uint8_t>& deflatedData, bool wasGenerated) const
{
    glm::vec3 regionIndex = glm::floor(chunkPos/glm::vec3(Chunk::CHUNK_WIDTH*256, Chunk::CHUNK_HEIGHT*256, Chunk::CHUNK_DEPTH*256));
    glm::vec3 chunkIndex = glm::floor(chunkPos/glm::vec3(Chunk::CHUNK_WIDTH, Chunk::CHUNK_HEIGHT, Chunk::CHUNK_DEPTH)) - regionIndex*16.0f;

    std::stringstream formatBuffer;
    formatBuffer << "region_" << (int)regionIndex.x << '_' << (int)regionIndex.y << '_' << (int)regionIndex.z;
    std::string regionName = formatBuffer.str();
    formatBuffer.str("");
    formatBuffer << "chunk_" << (int)chunkIndex.x << '_' << (int)chunkIndex.y << '_' << (int)chunkIndex.z;
    std::string chunkName = formatBuffer.str();

    std::ifstream infile("chunks/" + regionName, std::ios::binary);
    std::ofstream outfile("chunks/" + regionName + ".tmp", std::ios::binary);
    if (outfile.is_open()) {
        uint8_t nameSize;
        std::string name;
        int deflatedSize;
        std::vector<uint8_t> deflated;
        char wasGened;

        for (;;) {
            if (!infile.read((char*)(&nameSize), sizeof(nameSize))) {
                break;
            }
            name.resize(nameSize);
            if (!infile.read(&name[0], nameSize)) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (!infile.read((char*)(&deflatedSize), sizeof(deflatedSize))) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }
            deflated.resize(deflatedSize);
            if (!infile.read((char*)(&deflated[0]), deflatedSize)) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (!infile.read((char*)(&wasGened), sizeof(wasGened))) {
                fatalError("I/O Error: ", "Region file \'" + regionName + "\' is malformed!");
            }

            if (chunkName == name) {
                break;
            }
            outfile.write((char*)(&nameSize), sizeof(nameSize));
            outfile.write(&name[0], nameSize);
            outfile.write((char*)(&deflatedSize), sizeof(deflatedSize));
            outfile.write((char*)(&deflated[0]), deflatedSize);
            outfile.write((char*)(&wasGened), sizeof(wasGened));
        }

        nameSize = chunkName.length();
        deflatedSize = deflatedData.size();
        wasGened = wasGenerated;

        outfile.write((char*)(&nameSize), sizeof(nameSize));
        outfile.write(&chunkName[0], nameSize);
        outfile.write((char*)(&deflatedSize), sizeof(deflatedSize));
        outfile.write((char*)(&deflatedData[0]), deflatedSize);
        outfile.write((char*)(&wasGened), sizeof(wasGened));

        if (infile) {
            auto curFilePos = infile.tellg();
            infile.seekg(0, std::ios::end);
            auto remDataSize = infile.tellg() - curFilePos;
            infile.seekg(curFilePos, std::ios::beg);

            std::vector<uint8_t> remData(remDataSize);
            infile.read((char*)(&remData[0]), remDataSize);
            outfile.write((char*)(&remData[0]), remDataSize);

            infile.close();
        }
        outfile.close();
        remove(("chunks/" + regionName).c_str());
        rename(("chunks/" + regionName + ".tmp").c_str(), ("chunks/" + regionName).c_str());
    }
}

