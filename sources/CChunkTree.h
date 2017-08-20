#ifndef C_CHUNK_TREE_H
#define C_CHUNK_TREE_H

#include "CChunk.h"
#include "utils3d.h"

#include <mutex>

#include <SDL2/SDL.h>

class CChunkTree
{
public:
    enum EChunkFlags { UNINITIALIZED = 1, INITIALIZED = 2, NEED_GENERATION = 4, GENERATED = 8,
                       NEED_MESH_UPDATE = 16, NEED_STATE_UPDATE = 32, RENDERABLE = 64, ALL = 127 };

    CChunkTree(): root(nullptr) {}
    ~CChunkTree() {
        deleteAll(root);
        while (!chunksToErase.empty()) {
            eraseChunks();
            SDL_Delay(10);
        }
    }

    void addChunk(const glm::vec3& position);
    void remChunk(const glm::vec3& position);

    CChunk* getChunk(const glm::vec3& pos, EChunkFlags flags = GENERATED) {
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

    void getChunkArea(std::vector<CChunk*>& output, const utils3d::AABBox& area, EChunkFlags flags = GENERATED) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        output.clear();
        if (root) {
            getLeafArea(root, output, area, flags);
        }
    }

    void getIntersectingChunks(std::vector<CChunk*>& output,
                               const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, EChunkFlags flags = RENDERABLE) {
        std::lock_guard<std::mutex> lck(rootBeingModified);
        output.clear();
        if (root) {
            getIntersectingLeafs(root, output, rayPos, rayDir_inverted, flags);
        }
    }

    void getFrustumChunks(std::vector<CChunk*>& output, const utils3d::Frustum& frustum, EChunkFlags flags = RENDERABLE) {
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

    // Generates new chunks inside the active area
    void genNewChunks(const utils3d::AABBox& activeArea);

    // Sets up for deletion chunks outside the active area
    void eraseOldChunks(const utils3d::AABBox& activeArea);


    // Main thread only!
    void initChunks() {
        std::lock_guard<std::mutex> lck(initedBeingModified);
        for (CChunk* curChunk : chunksToInit) {
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
    void getLeafArea(TreeLeafNode* node, std::vector<CChunk*>& output, const utils3d::AABBox& area, EChunkFlags flags) const;
    void getIntersectingLeafs(TreeLeafNode* node, std::vector<CChunk*>& output, const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, EChunkFlags flags) const;
    void getFrustumLeafs(TreeLeafNode* node, std::vector<CChunk*>& output, const utils3d::Frustum& frustum, EChunkFlags flags) const;

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
            CChunk *chunk;
        } leaf;
    };
    std::mutex rootBeingModified;
    TreeLeafNode *root;

    std::mutex initedBeingModified;
    std::vector<CChunk*> chunksToInit;

    std::mutex erasedBeingModified;
    struct ChunkTimeBundle
    {
        CChunk* chunk;
        std::chrono::high_resolution_clock::time_point eraseTime;

        ChunkTimeBundle(CChunk* chunk) {
            this->chunk = chunk;
            eraseTime = std::chrono::high_resolution_clock::now();
        }
    };
    std::vector<ChunkTimeBundle> chunksToErase;
};

#endif // C_CHUNK_TREE_H
