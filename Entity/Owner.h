//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_OWNER_H
#define OSFINAL1_OWNER_H

#include <string>
using namespace std;

// 用户
class Owner	{
public:
    unsigned short ownerId;	    // 用户Id
    unsigned short groupId;	    // 组Id
    string ownerName;	        // 用户名
    string ownerPassword;       // 用户密码
};

#endif //OSFINAL1_OWNER_H
