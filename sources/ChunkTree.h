#ifndef CHUNK_TREE_H
#define CHUNK_TREE_H

#include "Chunk.h"
#include "utils3d.h"

#include <mutex>

#include <SDL2/SDL.h>
#include <new>
#include <iostream>

class ChunkTree
{
public:
    enum EChunkFlags { UNINITIALIZED = 1, INITIALIZED = 2, NEED_GENERATION = 4, GENERATED = 8,
                       NEED_MESH_UPDATE = 16, NEED_STATE_UPDATE = 32, RENDERABLE = 64, EDITED = 128,
                       NEED_LIGHT_UPDATE = 256, ALL = 511 };

    ChunkTree(): root(nullptr) {}
    ~ChunkTree() {
        eraseOldChunks(utils3d::AABBox());
        std::cout << "Saving loaded chunks..." << std::endl;
        while (!chunksToErase.empty()) {
            eraseChunks();
            SDL_Delay(10);
        }
    }

    void addChunk(const glm::vec3& position);
    void remChunk(const glm::vec3& position);

    Chunk* getChunk(const glm::vec3& pos, EChunkFlags flags = GENERATED) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        TreeLeafNode* foundLeaf = nullptr;
        if (root) {
            foundLeaf = getLeaf(root, pos, flags);
        }
        if (foundLeaf) {
            return foundLeaf->leaf.chunk;
        }
        return nullptr;
    }

    void getChunkArea(std::vector<Chunk*>& output, const utils3d::AABBox& area, EChunkFlags flags = GENERATED) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        output.clear();
        if (root) {
            getLeafArea(root, output, area, flags);
        }
    }

    void getIntersectingChunks(std::vector<Chunk*>& output,
                               const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, EChunkFlags flags = RENDERABLE) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        output.clear();
        if (root) {
            getIntersectingLeafs(root, output, rayPos, rayDir_inverted, flags);
        }
    }

    void getFrustumChunks(std::vector<Chunk*>& output, const utils3d::Frustum& frustum, EChunkFlags flags = RENDERABLE) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        output.clear();
        if (root) {
            getFrustumLeafs(root, output, frustum, flags);
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        deleteAll(root);
        root = nullptr;
    }

    // Generates new chunks or load saved ones inside the active area
    void genNewChunks(const utils3d::AABBox& activeArea, int curSunlight);

    // Sets up for deletion chunks outside the active area
    void eraseOldChunks(const utils3d::AABBox& activeArea);


    // Main thread only!
    void initChunks() {
        std::lock_guard<std::mutex> lck(initedBeingModified);
        for (Chunk* curChunk : chunksToInit) {
            curChunk->initOpenGLState();
        }
        chunksToInit.clear();
    }

    // Main thread only!
    void eraseChunks() {
        using namespace std::chrono;

        std::lock_guard<std::mutex> lck(erasedBeingModified);
        auto nowTime = high_resolution_clock::now();
        auto it = chunksToErase.begin();
        while (it != chunksToErase.end()) {
            if (duration_cast<seconds>(nowTime - it->eraseTime).count() > 1) {
                delete it->chunk;
                it = chunksToErase.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    union TreeLeafNode;

    void addLeaf(TreeLeafNode* node, const glm::vec3& position, TreeLeafNode* leaf = nullptr);
    void remLeaf(TreeLeafNode* node, const glm::vec3& position);
    void deleteAll(TreeLeafNode* node);

    TreeLeafNode* getLeaf(TreeLeafNode* node, const glm::vec3& pos, EChunkFlags flags) const;
    void getLeafArea(TreeLeafNode* node, std::vector<Chunk*>& output, const utils3d::AABBox& area, EChunkFlags flags) const;
    void getIntersectingLeafs(TreeLeafNode* node, std::vector<Chunk*>& output, const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, EChunkFlags flags) const;
    void getFrustumLeafs(TreeLeafNode* node, std::vector<Chunk*>& output, const utils3d::Frustum& frustum, EChunkFlags flags) const;

    void loadChunks(std::map<glm::vec3, Chunk*>& chunksToLoad);
    void saveChunks(const std::vector<glm::vec3>& chunksToSave);

    struct BundleChunk
    {
        bool gotLoaded;
        std::vector<uint8_t> deflatedData;
        bool wasGenerated;

        BundleChunk(): gotLoaded(false), deflatedData(), wasGenerated(false) {}
        BundleChunk(bool loaded, std::vector<uint8_t> data, bool wasGened)
        : gotLoaded(loaded), deflatedData(data), wasGenerated(wasGened)
        {}
    };

    void loadFromBundle(std::map<glm::vec3, BundleChunk>& chunksToLoad) const;
    void saveToBundle(std::map<glm::vec3, BundleChunk>& chunksToSave) const;

    union TreeLeafNode
    {
        TreeLeafNode() {}

        bool isLeaf;
        struct { // isLeaf == false
            bool isLeaf;
            glm::vec3 center;
            TreeLeafNode *subdivisions[2][2][2];
            utils3d::AABBox boundaries;
        } node;
        struct { // isLeaf == true
            bool isLeaf;
            Chunk *chunk;
        } leaf;
    };
    std::mutex rootBeingModified;
    TreeLeafNode *root;

    std::mutex initedBeingModified;
    std::vector<Chunk*> chunksToInit;

    std::mutex erasedBeingModified;
    struct ChunkTimeBundle
    {
        Chunk* chunk;
        std::chrono::high_resolution_clock::time_point eraseTime;

        explicit ChunkTimeBundle(Chunk* chunk) {
            this->chunk = chunk;
            eraseTime = std::chrono::high_resolution_clock::now();
        }
    };
    std::vector<ChunkTimeBundle> chunksToErase;
};

namespace std
{
    template<> struct less<glm::vec3> {
        bool operator() (const glm::vec3& lhs, const glm::vec3& rhs) const
        {
            return lhs.x < rhs.x || (lhs.x == rhs.x && (lhs.y < rhs.y || (lhs.x == rhs.x && lhs.z < rhs.z)));
        }
    };
}

#endif // CHUNK_TREE_H
