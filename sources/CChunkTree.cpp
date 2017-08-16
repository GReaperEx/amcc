#include "CChunkTree.h"

void CChunkTree::addLeaf(TreeLeafNode* node, const glm::vec3& position, TreeLeafNode* leaf)
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

    if (node->node.subdivisions[indX][indY][indZ] == nullptr) {
        if (leaf != nullptr) {
            node->node.subdivisions[indX][indY][indZ] = leaf;
        } else {
            node->node.subdivisions[indX][indY][indZ] = new TreeLeafNode;
            node->node.subdivisions[indX][indY][indZ]->isLeaf = true;
            node->node.subdivisions[indX][indY][indZ]->leaf.chunk = new CChunk(position);
        }
    } else if (node->node.subdivisions[indX][indY][indZ]->isLeaf) {
        TreeLeafNode *newNode = new TreeLeafNode;
        memset(newNode, 0, sizeof(TreeLeafNode));
        newNode->node.center = position;
        addLeaf(newNode, position);
        addLeaf(newNode, node->node.subdivisions[indX][indY][indZ]->leaf.chunk->getPosition(), node->node.subdivisions[indX][indY][indZ]);
        node->node.subdivisions[indX][indY][indZ] = newNode;
    } else {
        addLeaf(node->node.subdivisions[indX][indY][indZ], position);
    }
}

void CChunkTree::remLeaf(TreeLeafNode* node, const glm::vec3& position)
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
                delete temp->leaf.chunk;
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
}

void CChunkTree::deleteAll(TreeLeafNode* node)
{
    if (node->isLeaf) {
        delete node->leaf.chunk;
    } else {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    if (node->node.subdivisions[i][j][k]) {
                        deleteAll(node->node.subdivisions[i][j][k]);
                        delete node->node.subdivisions[i][j][k];
                    }
                }
            }
        }
    }
}

void CChunkTree::getLeafs(TreeLeafNode* node, std::vector<CChunk*>& output) const
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                if (node->node.subdivisions[i][j][k]) {
                    if (node->node.subdivisions[i][j][k]->isLeaf) {
                        output.push_back(node->node.subdivisions[i][j][k]->leaf.chunk);
                    } else {
                        getLeafs(node->node.subdivisions[i][j][k], output);
                    }
                }
            }
        }
    }
}
