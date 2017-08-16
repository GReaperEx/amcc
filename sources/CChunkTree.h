#ifndef C_CHUNK_TREE_H
#define C_CHUNK_TREE_H

#include "CChunk.h"

class CChunkTree
{
public:
    CChunkTree(): root(nullptr) {}
    ~CChunkTree() {
        deleteAll(root);
    }

    void addChunk(const glm::vec3& position) {
        if (root == nullptr) {
            root = new TreeLeafNode;
            memset(root, 0, sizeof(TreeLeafNode));
            root->node.center = position;
        }

        addLeaf(root, position);
    }

    void remChunk(const glm::vec3& position) {
        if (root != nullptr) {
            remLeaf(root, position);
        }

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

    void getAllChunks(std::vector<CChunk*>& output) const {
        output.clear();
        if (root) {
            getLeafs(root, output);
        }
    }

    void clear() {
        deleteAll(root);
        delete root;
        root = nullptr;
    }

private:
    union TreeLeafNode;

    void addLeaf(TreeLeafNode* node, const glm::vec3& position, TreeLeafNode* leaf = nullptr);
    void remLeaf(TreeLeafNode* node, const glm::vec3& position);
    void deleteAll(TreeLeafNode* node);
    void getLeafs(TreeLeafNode* node, std::vector<CChunk*>& output) const;

    union TreeLeafNode
    {
        TreeLeafNode() {}

        bool isLeaf;
        struct { // isLeaf == false
            bool isLeaf;
            glm::vec3 center;
            TreeLeafNode *subdivisions[2][2][2];
        } node;
        struct { // isLeaf == true
            bool isLeaf;
            CChunk *chunk;
        } leaf;
    };
    TreeLeafNode *root;
};

#endif // C_CHUNK_TREE_H
