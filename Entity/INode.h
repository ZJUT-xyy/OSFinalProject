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
    INode *parent;	            // 所属的目录i节点
    unsigned int nodeId;	    // i节点Id
    unsigned int users;	        // 引用计数
};

#endif //OSFINAL1_INODE_H
