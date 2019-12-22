//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_INODE_H
#define OSFINAL1_INODE_H

#include "DINode.h"

// 内存iNode
class INode	{
public:
    DINode dINode;
    INode *parent;
    unsigned int nodeId;	    // i节点Id
    unsigned int users;
};

#endif //OSFINAL1_INODE_H
