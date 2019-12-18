//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_OWNERS_H
#define OSFINAL1_OWNERS_H

#include "../define.h"
#include "Owner.h"

// 用户组
class Owners {
public:
    unsigned short ownerNum;	// 用户项数
    Owner os[MAX_OWNER_NUM];	// 用户项组
};

#endif //OSFINAL1_OWNERS_H
