#ifndef C_CHUNK_TREE_H
#define C_CHUNK_TREE_H

#include "CChunk.h"
#include "utils3d.h"

class CChunkTree
{
public:
    CChunkTree(): root(nullptr) {}
    ~CChunkTree() {
        deleteAll(root);
    }

    void addChunk(const glm::vec3& position);
    void remChunk(const glm::vec3& position);

    CChunk* getChunk(const glm::vec3& pos, bool beGenerated = true) const {
        TreeLeafNode* foundLeaf = nullptr;
        if (root) {
            foundLeaf = getLeaf(root, pos);
        }
        if (foundLeaf) {
            if (!beGenerated || (beGenerated && foundLeaf->leaf.chunk->isChunkGenerated())) {
                return foundLeaf->leaf.chunk;
            }
        }
        return nullptr;
    }

    void getChunkArea(std::vector<CChunk*>& output, const utils3d::AABBox& area) const {
        output.clear();
        if (root) {
            getLeafArea(root, output, area);
        }
    }

    void getIntersectingChunks(std::vector<CChunk*>& output,
                               const glm::vec3& rayPos, const glm::vec3& rayDir_inverted) const {
        output.clear();
        if (root) {
            getIntersectingLeafs(root, output, rayPos, rayDir_inverted);
        }
    }

    void getFrustumChunks(std::vector<CChunk*>& output, const utils3d::Frustum& frustum) const {
        output.clear();
        if (root) {
            getFrustumLeafs(root, output, frustum);
        }
    }

    void clear() {
        if (root) {
            deleteAll(root);
            delete root;
            root = nullptr;
        }
    }

private:
    union TreeLeafNode;

    void addLeaf(TreeLeafNode* node, const glm::vec3& position, TreeLeafNode* leaf = nullptr);
    void remLeaf(TreeLeafNode* node, const glm::vec3& position);
    void deleteAll(TreeLeafNode* node);

    TreeLeafNode* getLeaf(TreeLeafNode* node, const glm::vec3& pos) const;
    void getLeafArea(TreeLeafNode* node, std::vector<CChunk*>& output, const utils3d::AABBox& area) const;
    void getIntersectingLeafs(TreeLeafNode* node, std::vector<CChunk*>& output, const glm::vec3& rayPos, const glm::vec3& rayDir_inverted) const;
    void getFrustumLeafs(TreeLeafNode* node, std::vector<CChunk*>& output, const utils3d::Frustum& frustum) const;

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
    TreeLeafNode *root;
};

#endif // C_CHUNK_TREE_H
