//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_GROUPS_H
#define OSFINAL1_GROUPS_H

#include "../define.h"
#include "Group.h"

// 组集合
class Groups {
public:
    unsigned short groupNum;	// 组数
    Group gs[MAX_GROUP_NUM];
};

#endif //OSFINAL1_GROUPS_H
