//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_DIR_H
#define OSFINAL1_DIR_H

#include "../define.h"
#include "Direct.h"

// 目录结构
class Dir {
public:
    unsigned short dirNum;	             // 目录项数
    Direct direct[MAX_DIRECT_NUM];	     // 目录项数组
    string padding;	                     // 目录结构填充字符组
};

#endif //OSFINAL1_DIR_H
