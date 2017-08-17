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

    void getAllChunks(std::vector<CChunk*>& output) const {
        output.clear();
        if (root) {
            getAllLeafs(root, output);
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

    void getAllLeafs(TreeLeafNode* node, std::vector<CChunk*>& output) const;
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
