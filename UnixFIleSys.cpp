//
// Created by xyy on 2019/12/17.
//
#include <cstring>
#include <cstdio>
#include <ctime>
#include "UnixFileSys.h"
#include "status.h"

// ��ʼ��ϵ�в��� ///////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ʼ������
void UnixFIleSys :: initGlobal(FILE* f) {
    cout << "���ڿ�ʼ��ʼ������" << endl;
    // ��ʼ��������
    sp -> size = DISK_SIZE;
    sp -> freeBlockNum = BLOCK_NUM; // ��ʼ���е��̿鶼Ϊ�����̿�
    sp -> freeINodeNum = INODE_NUM - 1;
    for (int j = 0; j < INODE_NUM; j ++)
        sp -> freeINode[j] = INODE_NUM + 1 - j; // Ϊÿһ��INode��ʼ��һ��INodeId
    sp -> nextFreeINode = INODE_NUM - 2;

    // ��ʼ����INode
    root -> parent = nullptr; // ��Ŀ¼�ĸ�Ŀ¼Ϊ��
    root -> nodeId = 2;
    root -> users = 1;
    time_t now;
    now = time(nullptr);
    root -> dINode.createTime = now;
    root -> dINode.fileSize = 0;
    root -> dINode.groupId = 0;
    root -> dINode.linkNum = 0;
    root -> dINode.mod = 14;
    root -> dINode.modifyTime = now;
    root -> dINode.ownerId = 1;
    root -> dINode.readTime = now;

    // ��ʼ���û�
    os -> ownerNum = 2;
    os -> os[0].ownerId = 1;
    os -> os[0].groupId = 1;
    strcpy(os -> os[0].ownerName, "osfinal");
    strcpy(os -> os[0].ownerPassword, "osfinal");
    os -> os[1].ownerId = 2;
    os -> os[1].groupId = 2;
    strcpy(os -> os[1].ownerName, "osfinal2");
    strcpy(os -> os[1].ownerPassword, "osfinal2");

    // ��ʼ���û���
    gs -> groupNum = 2;
    gs -> gs[0].groupId = 1;
    strcpy(gs -> gs[0].groupName, "zjut");
    gs -> gs[1].groupId = 2;
    strcpy(gs -> gs[1].groupName, "zjut2");

    f = fopen(FILE_PATH,"wb"); // wb - ֻд�򿪻��½�һ���������ļ�

    int bGs = BLOCK_NUM / BLOCK_GROUP_SIZE; // �ж�����
    int left = BLOCK_NUM - bGs * BLOCK_GROUP_SIZE; // ������ʣ�µĿ���

    fseek(f, DISK_SIZE, SEEK_SET);
    fwrite("?", 1, 1, f);

    // ���Ϊ���飬û�������
    if (left == 0) {
        for (int i = 0; i < BLOCK_GROUP_SIZE; i ++)
            sp -> freeBlock[i] = BLOCK_START + BLOCK_GROUP_SIZE - 1 - i; // ��ʼ����ǰ��Ŀ��
        sp -> nextFreeBlock = BLOCK_GROUP_SIZE - 1; // ��һ�����п������ڵ�ƫ����
        // �����ʼ��ȫ����ţ�������д�����
        for (int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1; // ��ʼ��ȫ�����
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum, sizeof(blocksNum), 1, f);
            fwrite(&blocks, sizeof(blocks), 1, f);
        }
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks, sizeof(blocks), 1, f);
    } else {
    // ���������飬������Գ�һ�飬������ڿ�����ջ��ջ��
        for (int i = 0; i < left; i ++)
            sp -> freeBlock[i] = BLOCK_START + left - 1 - i;
        sp -> nextFreeBlock = left - 1; // ��һ�����п������ڵ�ƫ����
        // �����ʼ��ȫ����ţ�������д�����
        for(int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1; // ������
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum,sizeof(blocksNum), 1, f);
            fwrite(&blocks, sizeof(blocks), 1, f);
        }
        // ����һ���̿���Ĳ���
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks, sizeof(blocks), 1, f);
    }

    // ����ʼ�����ȫ�ֱ���д�����
    fseek(f, BLOCK_SIZE, SEEK_SET); // ��λ���������λ�ã�һ��BLOCK_SIZE֮��
    fwrite(sp, sizeof(SuperBlock), 1, f);
    fwrite(os, sizeof(Owners), 1, f);
    fwrite(gs, sizeof(Groups), 1, f);
    fseek(f, BLOCK_SIZE * root -> nodeId, SEEK_SET);
    fwrite(&root -> dINode, sizeof(DINode), 1, f);
    fclose(f);

    // ��ʼ��Ŀ¼
    auto* dt = new Direct();
    dt -> iNodeId = 2;
    strcpy(dt -> name, "root");
    ds.push_back(dt);
    superMkdir(root, "test1", 1, 1);
    superMkdir(root, "test2", 2, 2);

    cout << "��ʼ�����ݳɹ���" << endl;

    cout << "�ܹ���" << os -> ownerNum << "���û�" << endl;
    for (int j = 0; j < os -> ownerNum; j ++)
        cout << os -> os[j].ownerName << endl;
}

void UnixFIleSys :: initSystem() {
    FILE *f = fopen(FILE_PATH,"rb");
    // ��������ļ������ھͽ���ȫ�ֳ�ʼ��
    if(f == nullptr) {
        cout << "ϵͳ��������" << endl;
        initGlobal(f);
    } else {
    // ��������ļ����ھͶ���ȫ�ֱ�������Ŀ¼��Ϣ���ҽ���Ŀ¼ѹ��·��ջ
        fseek(f, BLOCK_SIZE, SEEK_SET);
        fread(sp, sizeof(SuperBlock), 1, f);
        fread(os, sizeof(Owners), 1, f);
        fread(gs, sizeof(Groups), 1, f);
        root -> nodeId = 2;
        root -> parent = nullptr;
        root -> users = 2;
        fseek(f, BLOCK_SIZE * root -> nodeId, SEEK_SET);
        fread(&root -> dINode, sizeof(DINode), 1, f);
        auto* dt = new Direct();
        dt -> iNodeId = 2;
        strcpy(dt -> name, "root");
        ds.push_back(dt); // ����Ŀ¼ѹ��·��ջ
        fclose(f);
        readCurDir();
    }
}


// ���߷���//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���ڴ�ڵ�
// ͨ��INodeId�ҵ�INode����ʼλ�ã�����
bool UnixFIleSys :: readINode(INode* r)	{
    FILE *f=fopen(FILE_PATH,"rb");
    if(f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * r -> nodeId, SEEK_SET);
        fread(&r -> dINode, sizeof(DINode), 1, f);
        fclose(f);
        return true;
    }
}

// д�ڴ�ڵ�
// ͨ��INodeId�ҵ�INode����ʼλ�ã�д��
bool UnixFIleSys :: writeINode(INode* w) {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * w -> nodeId, SEEK_SET);
        fwrite(&w -> dINode, sizeof(DINode), 1, f);
        fclose(f);
        return true;
    }
}

// д������
// ��λ������ͷ��ʼ��һ��BLOCK_SIZE֮��SuperBlock����ʼλ�ã���д��
bool UnixFIleSys :: writeSuperBlock() {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE, SEEK_SET);
        fwrite(sp, sizeof(SuperBlock), 1, f);
        fclose(f);
        return true;
    }
}

// дĿ¼
// ͨ��blockId�ҵ�Ŀ¼��ʼλ�ã�д��
bool UnixFIleSys :: writeDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == NULL)
        return false;
    else {
        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
        fwrite(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}

// ��Ŀ¼
// ͨ��blockId�ҵ�Ŀ¼��ʼλ�ã�����
bool UnixFIleSys :: readDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
        fread(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}

// ����һ��
// ͨ����ǰ���ջ����Ŀ���ҵ���һ�����ʼλ�ã�ջ�ף���������һ��Ŀ���ջ��Ϣ����һ�����п���Ϣ
bool UnixFIleSys :: readNextBG() {
    FILE *f = fopen(FILE_PATH, "rb");
    if(f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * sp -> freeBlock[0], SEEK_SET);
        fread(&sp -> nextFreeBlock, sizeof(sp -> nextFreeBlock), 1, f);
        fread(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
        sp -> freeBlockNum --;
        sp -> nextFreeBlock --;
        fclose(f);
        writeSuperBlock();
        return true;
    }
}

// ��ȡһ�����п�
// ��ǰ�����Ϣ�洢�ڳ������У��������Ժ�д������
int UnixFIleSys :: getFreeBlock() {
    if(sp -> freeBlockNum > 0) {
        // �����ǰ���п��п����һ��
        if (sp -> nextFreeBlock > 0) {
            sp -> freeBlockNum --;
            sp -> nextFreeBlock --;
            writeSuperBlock();
            return sp -> freeBlock[sp -> nextFreeBlock + 1];
        } else {
            // �����ǰ���Ѿ�û�п��п��ˣ��Ͷ���һ���̣���ִ��һ�λ�ȡ���п�ķ���
            readNextBG();
            return getFreeBlock();
        }
    } else
        return STATUS_NO_BLOCK;
}

// ��ȡһ�����нڵ�
// ��ǰ���ڽڵ����Ϣ�洢�ڳ������У��������Ժ�д������
int UnixFIleSys :: getFreeINode() {
    if (sp -> freeINodeNum > 0) {
        sp -> freeINodeNum --;
        sp -> nextFreeINode --;
        writeSuperBlock();
        return sp -> freeINode[sp -> nextFreeINode + 1];
    }
    else
        return STATUS_NO_INODE;
}

// ����ǰĿ¼
// ���ȫ�ֱ����еĵ�ǰ�ڵ���ļ���С��ϢΪ0��ֱ���޸�ȫ�ֱ���d����Ȼͨ����ǰ�ڵ�ĵ�ַ�����ͷλ�ã�����Ŀ¼��Ϣ
int UnixFIleSys ::  readCurDir() {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == nullptr)
        return STATUS_FILE_OPEN_ERROR;
    else {
        if (curINode -> dINode.fileSize == 0)
            d -> dirNum = 0;
        else {
            fseek(f, BLOCK_SIZE * curINode -> dINode.addr[0], SEEK_SET);
            fread(d, sizeof(Dir), 1, f);
            fclose(f);
        }
        return STATUS_OK;
    }
}

// дOwners
// ��λ��һ�����С����һ���������С֮��Owners����ʼλ�ã���д��
bool UnixFIleSys ::  writeOS() {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == NULL)
        return false;
    else {
        fseek(f, BLOCK_SIZE + sizeof(SuperBlock), SEEK_SET);
        fwrite(os, sizeof(Owners), 1, f);
        fclose(f);
        return true;
    }
}

// дtext
// ��Ҫ����ԭ�ļ���û�п�տ�϶��׷���ַ����ֿ����û�п��϶���п�϶�����϶
int UnixFIleSys :: writeText(INode* temp, const string& text) {
    FILE *f = fopen(FILE_PATH,"rb+");
    if (f == nullptr)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE; // ԭ���ļ���С��ռ��������
        int ps = temp -> dINode.fileSize % BLOCK_SIZE; // ԭ���ļ���С��ռ�Ķ����ֽ���
        int bs = text.size() / BLOCK_SIZE; // ׷�ӵ��ַ�����С��ռ��������
        int ls = text.size() % BLOCK_SIZE; // ׷�ӵ��ַ�����С��ռ�Ķ����ֽ���
        int pos = 0;

        // �������д����ļ����ܿ��������ն����ֽ���
        int totalBlockNum = 0;
        int totalLeft = 0;
        if (ls <= BLOCK_SIZE - ps) { // Ҫд��Ĳ��ֵĶ����ֽڿ�������ԭ��϶
            totalBlockNum = as + bs;
            totalLeft = ps + ls;
        } else { // Ҫд��Ĳ��ֵĶ����ֽ�����ԭ��϶���ж�
            totalBlockNum = as + bs + 1;
            totalLeft = ps + ls - BLOCK_SIZE;
        }
        // �����ַʹ�����
        int status = 0; // statusΪ0����δʹ�ü�ַ��statusΪ1����ʹ������һ����ַ��statusΪ2����ʹ�����Զ�����ַ
        if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4) {
            status = 0;
        } else if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4 + 128) {
            status = 1;
        } else if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4 + 128 + 128 * 128) {
            status = 2;
        } else {
            return STATUS_BEYOND_SIZE;
        }
        // ����ԭ�ļ���ַʹ�����
        int status0 = 0;
        if (as + (ps > 0? 1: 0) <= 4) {
            status0 = 0;
        } else if (as + (ps > 0? 1: 0) <= 4 + 128) {
            status0 = 1;
        } else if (as + (ps > 0? 1: 0) <= 4 + 128 + 128 * 128) {
            status0 = 2;
        }

        // ���ԭ�ļ��Ŀ�������Ҫʹ�ü�ַ����ֱ�Ӷ�ÿһ�����飬׷�ӵ�content
        if (status0 == 0 && status == 0) {
            // ԭ���ļ�ǡ��������һ���飬�Ͱ���׷�ӵ��ַ�����С��ռ���������ÿ��п飬����ÿһ��blockId����ԭ�ļ��ĵ�ַ����
            if (ps == 0) {
                for (int i = 0; i < bs; i++) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp->dINode.addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // ���׷�ӵ��ַ����ж���������ֽڣ�������һ�����п飬��blockId����ԭ�ļ��ĵ�ַ����
                if (ls > 0) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp->dINode.addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                fclose(f);
                return STATUS_OK;
            } else {
                // ԭ���ļ�����ǡ��������һ����
                int lps = BLOCK_SIZE - ps; // ԭ�ļ��ķ������ʣ��ռ�
                // ���׷���ַ����Ĵ�СС��ԭ�ļ��ķ������ʣ��ռ䣬ֱ��д�룬���޸��ļ���С����
                if (text.size() <= lps) {
                    fseek(f, BLOCK_SIZE * temp->dINode.addr[as] + ps, SEEK_SET);
                    fwrite(text.c_str(), text.size(), 1, f);
                    temp->dINode.fileSize += text.size();
                    fclose(f);
                    return STATUS_OK;
                } else {
                    // ���׷���ַ����Ĵ�С����ԭ�ļ��ķ������ʣ��ռ䣬��д��׷���ַ���ǡ�������������ʣ��ռ�Ĳ���
                    fseek(f, BLOCK_SIZE * temp->dINode.addr[as++] + ps, SEEK_SET);
                    fwrite(text.c_str(), lps, 1, f);
                    temp->dINode.fileSize += lps;
                    int lts = text.size() - lps;  // ׷���ַ�ʣ��δд��С
                    int lbs = lts / BLOCK_SIZE;   // ׷���ַ�ʣ��δд��ռ��������
                    int lls = lts % BLOCK_SIZE;   // ׷���ַ�ʣ��δд��ռ�Ķ����ֽ���
                    int tpos = lps;
                    // ����׷���ַ�ʣ��δд��С��ռ���������ÿ��п飬����ÿһ��blockId����ԭ�ļ��ĵ�ַ����
                    for (int i = 0; i < lbs; i++) {
                        int blockId = getFreeBlock();
                        if (blockId < 0) {
                            fclose(f);
                            return blockId;
                        } else {
                            temp->dINode.addr[as++] = blockId;
                            fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                            fwrite(text.substr(tpos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                            temp->dINode.fileSize += BLOCK_SIZE;
                            tpos += BLOCK_SIZE;
                        }
                    }
                    // ���׷���ַ�ʣ��δд��С�ж���������ֽڣ�������һ�����п飬��blockId����ԭ�ļ��ĵ�ַ����
                    if (lls > 0) {
                        int blockId = getFreeBlock();
                        if (blockId < 0) {
                            fclose(f);
                            return blockId;
                        } else {
                            temp->dINode.addr[as++] = blockId;
                            fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                            fwrite(text.substr(tpos, lls).c_str(), BLOCK_SIZE, 1, f);
                            temp->dINode.fileSize += lls;
                            tpos += lls;
                        }
                        fclose(f);
                        return STATUS_OK;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
            }
        } else if (status0 == 0 && status == 1) {
        // ���ԭ�ļ�û��ʹ�ü�ַ��׷�Ӻ���Ҫʹ��һ����ַ
            // ԭ���ļ�ǡ��������һ���飬�Ͱ���׷�ӵ��ַ�����С��ռ���������ÿ��п飬����ÿһ��blockId����ԭ�ļ��ĵ�ַ����
            if (ps == 0) {
                // ��ʹ����ֱ�ӿ�ŵĲ���
                int remainDirectBlockNum = 4 - as;
                for (int i = 0; i < remainDirectBlockNum; i++) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        (temp -> dINode).addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // ����һ������block������һ��ջ
                int blockId = getFreeBlock();
                int remainBlockNum = totalBlockNum - 4;
                (temp -> dINode).addr[4] = blockId; // д���ַ����ĵ�5�һ����ַ��
                vector<unsigned int> firstStackBlocks[remainBlockNum];
                // Ϊʣ��ÿһ����Ҫ���һ��ջ�ģ��飩������һ�����п飬��д�룬ͬʱ����pos
                for (int p = 0; p < remainBlockNum; p ++) {
                    int blockIdFir = getFreeBlock();
                    if (blockIdFir < 0) {
                        fclose(f);
                        return blockIdFir;
                    } else {
                        // ���뵽���п��д��һ��ջ����
                        firstStackBlocks -> push_back(blockIdFir);
                        // д����
                        fseek(f, BLOCK_SIZE * blockIdFir, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // ���׷�ӵ��ַ����ж���������ֽڣ�������һ�����п飬��blockId����ԭ�ļ��ĵ�ַ����
                if (ls > 0) {
                    int blockIdLeft = getFreeBlock();
                    if (blockIdLeft < 0) {
                        fclose(f);
                        return blockIdLeft;
                    } else {
                        // ���뵽���п��д��һ��ջ����
                        firstStackBlocks -> push_back(blockIdLeft);
                        // д����
                        fseek(f, BLOCK_SIZE * blockIdLeft, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                fclose(f);
                return STATUS_OK;
            } else {
                // ԭ���ļ�����ǡ��������һ����
                int lps = BLOCK_SIZE - ps; // ԭ�ļ��ķ������ʣ��ռ�
                // ��Ϊԭ�ļ�δʹ�ü�ַ��д���ʹ�ü�ַ�����Բ�����׷���ַ����Ĵ�СС��ԭ�ļ��ķ������ʣ��ռ�����
                // ��д��׷���ַ���ǡ�������������ʣ��ռ�Ĳ���
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[as ++] + ps, SEEK_SET);
                fwrite(text.substr(pos, ls).c_str(), lps, 1, f);
                temp -> dINode.fileSize += lps;
                pos += lps;

                int lts = text.size() - lps;  // ׷���ַ�ʣ��δд��С
                int lbs = lts / BLOCK_SIZE;   // ׷���ַ�ʣ��δд��ռ��������
                int lls = lts % BLOCK_SIZE;   // ׷���ַ�ʣ��δд��ռ�Ķ����ֽ���
                int tpos = lps;
                // ����һ�����п�洢һ��ջ
                int blockId = getFreeBlock();
                int remainBlockNum = totalBlockNum - 4;
                (temp -> dINode).addr[4] = blockId; // д���ַ����ĵ�5�һ����ַ��
                vector<unsigned int> firstStackBlocks;
                // Ϊʣ��ÿһ����Ҫ���һ��ջ�ģ��飩������һ�����п飬��д�룬ͬʱ����pos
                for (int p = 0; p < remainBlockNum; p ++) {
                    int blockIdFir = getFreeBlock();
                    if (blockIdFir < 0) {
                        fclose(f);
                        return blockIdFir;
                    } else {
                        // ���뵽���п��д��һ��ջ����
                        firstStackBlocks.push_back(blockIdFir);
                        // д����
                        fseek(f, BLOCK_SIZE * blockIdFir, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // ���׷�ӵ��ַ����ж���������ֽڣ�������һ�����п飬��blockId����ԭ�ļ��ĵ�ַ����
                if (totalLeft > 0) {
                    int blockIdLeft = getFreeBlock();
                    if (blockIdLeft < 0) {
                        fclose(f);
                        return blockIdLeft;
                    } else {
                        // ���뵽���п��д��һ��ջ����
                        firstStackBlocks.push_back(blockIdLeft);
                        // д����
                        fseek(f, BLOCK_SIZE * blockIdLeft, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                // һ��ջд�ش���
                fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                fwrite(firstStackBlocks.data(),sizeof(decltype(firstStackBlocks)::value_type)* firstStackBlocks.size(), 1, f);
                // fwrite(firstStackBlocks, sizeof(firstStackBlocks), 1, f);

                fclose(f);
                return STATUS_OK;
            }
        } else if (status0 == 0 && status == 2) {

        } else if (status0 == 1 && status == 0) {

        } else if (status0 == 1 && status == 1) {

        } else if (status0 == 1 && status == 2) {

        } else if (status0 == 2 && status == 0) {

        } else if (status0 == 2 && status == 1) {

        } else if (status0 == 2 && status == 2) {

        }
    }
}

// ��text
// ��Ҫ�����Ƿ��п����ֽڣ����ݴ����INode��Ϣ�еĵ�ַ�����ҵ��洢�飬����ÿһ�������
int UnixFIleSys :: readText(INode *temp) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;  // �ļ���С��ռ��������
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;  // �ļ���С��ռ�Ķ����ֽ���
        char content[BLOCK_SIZE];
        // ���ԭ�ļ��Ŀ�������Ҫʹ�ü�ַ����ֱ�Ӷ�ÿһ�����飬���
        if (as <= 4) {
            int i = 0;
            for (; i < as; i ++) {
                memset(content, '\0', sizeof(content));
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                memset(content, '\0', sizeof(content));
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        } else if(as >= 5 && as <= 132) {
            // ���ֻ�õ���һ����ַ,�ȶ�ֱ�ӵ�ַ��ÿһ�����飬׷�ӵ�content,��ȥһ��ջ��ÿһ������ݣ����
            // �ȶ�ֱ�ӵ�ַ��ÿһ�����飬���
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // ��һ��ջ���ڵĿ�ţ�����λ��һ��ջ���������е�ֱ�ӿ������
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            int blockNumInFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4; // ͨ���ļ���С������һ����ַ��ʹ���˼���block
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            int i = 0;
            for (; i < blockNumInFirstStack; i ++) {
                // ����һ��ջ�е�ֱ�ӿ�Ų������������ݣ����
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        } else if (as > 132) {
            // ����õ��˶�����ַ
            // �ȶ�ֱ�ӵ�ַ��ÿһ�����飬���
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // ��һ��ջ���ڵĿ�ţ�����λ��һ��ջ���������еĶ���ջ���ڵĿ�Ŀ������
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            // ����һ��ջԪ��λ�÷���β�����Ӧ�Ķ���ջ�����ܳ��ֿ�϶��
            for (int i = 0; i < 128 - 1; i ++) {
                // ����һ��ջ�еĶ���ջ���ڵĿ�Ŀ�ţ�����λ����Ӧ����ջ����������ջ�е�ֱ�ӿ������
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                unsigned int blocksInSecStack[128];
                fread(blocksInSecStack, BLOCK_SIZE, 1, f);
                for (int j = 0; j < 128; j ++) {
                    // ��������ջ�е�ֱ�ӿ�Ų������������ݣ����
                    fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                    fread(content, BLOCK_SIZE, 1, f);
                    cout << content;
                }
            }
            // ����һ��ջԪ��λ����β�����Ӧ�Ķ���ջ���ܳ��ֿ�϶��
            fseek(f, BLOCK_SIZE * blocksInFirstStack[128], SEEK_SET);
            unsigned int blocksInSecStack[128];
            fread(blocksInSecStack, BLOCK_SIZE, 1, f);
            int blockNumInLastFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4 - 127 * 128; // ����һ��ջԪ��λ����β��Ӧ�Ķ���ջʹ�õ�blockNum
            int j = 0;
            for (; j < blockNumInLastFirstStack; j ++) {
                // ��������ջ�е�ֱ�ӿ�Ų������������ݣ����
                fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[j], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        }
        fclose(f);
    }
    return STATUS_OK;

}

// �黹���п�
// ����blockId���黹�������޸ĳ��������Ϣ���޸ĺ�д������
int UnixFIleSys :: returnFreeBlock(unsigned int blockId) {
    // �����ǰ�黹�пգ��͹黹�ڵ�ǰ��
    if (sp -> nextFreeBlock <= 18) {
        sp -> freeBlockNum ++;
        sp -> nextFreeBlock ++;
        sp -> freeBlock[sp -> nextFreeBlock] = blockId;
        writeSuperBlock();
        return STATUS_OK;
    } else {
    // �����ǰ���޿գ�ȡ����һ�飬�黹����һ��
        FILE *f = fopen(FILE_PATH, "rb+");
        if (f == NULL)
            return STATUS_FILE_OPEN_ERROR;
        else {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            fseek(f, BLOCK_SIZE * (sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2), SEEK_SET);
            fwrite(&blocksNum, sizeof(unsigned int), 1, f);
            fwrite(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
            fclose(f);
            sp -> freeBlockNum += 2;
            sp -> freeBlock[0] = sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2; // �洢��һ�������̵���λ�̿��
            sp -> freeBlock[1] = blockId;
            sp -> nextFreeBlock = 1;
            writeSuperBlock();
            return STATUS_OK;
        }
    }
}

// �黹INode
// �޸ĳ�������Ϣ��Ȼ��д������
int UnixFIleSys :: returnFreeINode(unsigned int iNodeId) {
    sp -> freeINodeNum ++;
    sp -> nextFreeINode ++;
    sp -> freeINode[sp -> nextFreeINode] = iNodeId; // д������黹�˵�INodeId
    writeSuperBlock();
    return STATUS_OK;
}

// ��text
// ͨ��INodeId��ȡ�ļ���С��Ϣ�͵�ַ��Ϣ���ȶ�ÿһ�����飬׷�ӵ�content������ļ��п����ֽ�Ҳ����׷��
string UnixFIleSys :: getText(INode* temp) {
    string text;
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return "";
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;  // �ļ���С��ռ��������
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;  // �ļ���С��ռ�Ķ����ֽ���
        char content[BLOCK_SIZE];
        // ���ԭ�ļ��Ŀ�������Ҫʹ�ü�ַ����ֱ�Ӷ�ÿһ�����飬׷�ӵ�content
        if (as <= 4) {
            int i = 0;
            for (; i < as; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        } else if(as == 5) {
        // ���ֻ�õ���һ����ַ,�ȶ�ֱ�ӵ�ַ��ÿһ�����飬׷�ӵ�content,��ȥһ��ջ��ÿһ������ݣ�׷�ӵ�content
            // �ȶ�ֱ�ӵ�ַ��ÿһ�����飬׷�ӵ�content
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // ��һ��ջ���ڵĿ�ţ�����λ��һ��ջ���������е�ֱ�ӿ������
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            int blockNumInFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4; // ͨ���ļ���С������һ����ַ��ʹ���˼���block
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            int i = 0;
            for (; i < blockNumInFirstStack; i ++) {
                // ����һ��ջ�е�ֱ�ӿ�Ų������������ݣ�׷�ӵ�content
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        } else if (as == 6) {
        // ����õ��˶�����ַ
            // �ȶ�ֱ�ӵ�ַ��ÿһ�����飬׷�ӵ�content
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // ��һ��ջ���ڵĿ�ţ�����λ��һ��ջ���������еĶ���ջ���ڵĿ�Ŀ������
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            // ����һ��ջԪ��λ�÷���β�����Ӧ�Ķ���ջ�����ܳ��ֿ�϶��
            for (int i = 0; i < 128 - 1; i ++) {
                // ����һ��ջ�еĶ���ջ���ڵĿ�Ŀ�ţ�����λ����Ӧ����ջ����������ջ�е�ֱ�ӿ������
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                unsigned int blocksInSecStack[128];
                fread(blocksInSecStack, BLOCK_SIZE, 1, f);
                for (int j = 0; j < 128; j ++) {
                    // ��������ջ�е�ֱ�ӿ�Ų������������ݣ�׷�ӵ�content
                    fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                    fread(content, BLOCK_SIZE, 1, f);
                    text += content;
                }
            }
            // ����һ��ջԪ��λ����β�����Ӧ�Ķ���ջ���ܳ��ֿ�϶��
            fseek(f, BLOCK_SIZE * blocksInFirstStack[128], SEEK_SET);
            unsigned int blocksInSecStack[128];
            fread(blocksInSecStack, BLOCK_SIZE, 1, f);
            int blockNumInLastFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4 - 127 * 128; // ����һ��ջԪ��λ����β��Ӧ�Ķ���ջʹ�õ�blockNum
            int j = 0;
            for (; j < blockNumInLastFirstStack; j ++) {
                // ��������ջ�е�ֱ�ӿ�Ų������������ݣ�׷�ӵ�content
                fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // ����ļ��п����ֽ�
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[j], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        }
        fclose(f);
        return text;
    }
}

// ����
// ȥ��ͷβ�ո�
string UnixFIleSys :: trim(string s) {
    if (s.empty())
        return s;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}


// ҵ�񷽷�//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// չʾ��ǰĿ¼
// ���ζ�·��ջ�����ݣ���ϳ��ַ���
string UnixFIleSys :: pwd() {
    string path;
    for (int i = 0; i < ds.size(); i ++) {
        path += ds[i] -> name;
        path += "/";
    }
    return path;
}

// �����ļ���
// �ڵ�ǰĿ¼�£�����Ƿ���ͬ��Ŀ¼
int UnixFIleSys :: checkFileName(char name[MAX_NAME_SIZE]) {
    int flag = 0;
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL) {
        cout << "f������" << endl;
        flag = 1;
        return flag;
    } else {
        //cout << "f����" << endl;
        Dir d;
        // d.dirNum = 0;
        fseek(f, BLOCK_SIZE * curINode -> dINode.addr[0], SEEK_SET);
        fread(&d, sizeof(Dir), 1, f);
        fclose(f);
        //cout << "�������ڵ�Ŀ¼��Ŀ¼����Ϊ" << d.dirNum << endl;
        for (int i = 0; i < d.dirNum; i ++)
            if(strcmp(d.direct[i].name, name) == 0)
                flag = 1;
        return flag;
    }
}

// �����ļ���
// �����Ǵ����ڵ㣨��С��Dir�Ĵ�С����Ŀ¼��
int UnixFIleSys :: mkdir(INode* parent, char name[MAX_NAME_SIZE]) {
    // cout << "��Ҫ������Ŀ¼����" + name << endl;
    bool exist = checkFileName(name);
    // ԭ���ڵ�Ϊ��
    if (parent -> dINode.fileSize == 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int blockId = getFreeBlock();
        if(blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(nullptr);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = parent -> dINode.groupId;
                di -> linkNum = 0;
                di -> mod = 14;
                di -> modifyTime = now;
                di -> ownerId = parent -> dINode.ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = parent -> users;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    // ԭ���ڵ㲻Ϊ��
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = parent -> dINode.groupId;
            di -> linkNum = 0;
            di -> mod = 14;
            di -> modifyTime = now;
            di -> ownerId = parent -> dINode.ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = parent -> users;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd); // �������ڵ��Ŀ¼�ṹ
            if (dd.dirNum < MAX_DIRECT_NUM) { // ���Ŀ¼����׷�Ӿ�׷��
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            } else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    } else if (parent -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
        return STATUS_BEYOND_RIGHT;
    else if(parent -> dINode.mod == 12)
        return STATUS_READ_ONLY;
    else if(exist)
        return STATUS_FILENAME_EXIST;
    return STATUS_ERROR;
}

// ��superȨ�޴���Ŀ¼
// �����Ǵ����ڵ㣨��С��Dir�Ĵ�С����Ŀ¼��
int UnixFIleSys :: superMkdir(INode* parent, char name[MAX_NAME_SIZE], unsigned short ownerId, unsigned short groupId) {
    int exist = checkFileName(name);
    if (parent -> dINode.fileSize == 0) {
        int blockId = getFreeBlock();
        if (blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(NULL);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = groupId;
                di -> linkNum = 0;
                di -> mod = 14;
                di -> modifyTime = now;
                di -> ownerId = ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = 0;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    }
    else if (parent -> dINode.fileSize != 0) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = groupId;
            di -> linkNum = 0;
            di -> mod = 14;
            di -> modifyTime = now;
            di -> ownerId = ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = 0;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd);
            if (dd.dirNum < MAX_DIRECT_NUM) {
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
            else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    }
    return STATUS_ERROR;
}

// ��¼
// ��ȫ�ֱ���Owners�в��Ҷ�Ӧ�û�����������û�
int UnixFIleSys :: login() {
    cout << "please input your userName:";
    string userName;
    cin >> userName;
    cout << "please input your password:";
    char password[MAX_NAME_SIZE]={0};
    int i=0;
    while (i < MAX_NAME_SIZE) {
        password[i] = getchar();
        if (password[i] == '\n' && i == 0) i = -1;
        if (i != -1 && password[i] == '\n') {
            cout << "��⵽����" << endl;
            for (int j = i; j <= 13; j ++) {
                password[j]='\0';
            }
            break;
        }
        if (password[i] == 13) {
            password[i]='\0';
            break;
        }
        i++;
    }
    cout<<endl;
    char un[MAX_NAME_SIZE];
    strcpy(un, userName.c_str());
    for (int j = 0; j < os -> ownerNum; j ++) {
        int flag = strcmp(un, os->os[j].ownerName);
        int flag2 = strcmp(password, os->os[j].ownerPassword);
        if (strcmp(un, os->os[j].ownerName) == 0 && strcmp(password, os->os[j].ownerPassword) == 0) {
            curOwner = &os->os[j];
            return STATUS_OK;
        }
    }
    cout << "UserName or Password Wrong!!!" << endl;
    return login();
}

// ָ��ַ�
void UnixFIleSys :: commandDispatcher() {
    cout << endl;
    cout << curOwner -> ownerName << "@UnixFileSystem:~" << pwd() << "$";
    char c[100];
    cin.getline(c,100,'\n');
    while(c[0] == NULL)
        cin.getline(c,100,'\n');
    string command = c;
    command = trim(command);
    int flag = -1;
    string target = " -h";
    int result = command.find(target);
    if (result != -1) {
        flag = 18;
        goto aa;
    }
    int subPos;
    subPos = command.find_first_of(" ");
    if (subPos == -1) {
        if (command == "ls") {
            flag = 1;
        } else if (command == "pwd")
            flag=5;
        else if (command == "passwd")
            flag = 14;
        else if (command == "sp")
            flag = 17;
        else if (command == "help")
            flag = 19;
    } else {
        string c = command.substr(0, subPos);
        if (c == "chmod")
            flag = 2;
        else if (c == "chown")
            flag = 3;
        else if (c == "chgrp")
            flag = 4;
        else if (c == "cd")
            flag = 6;
        else if (c == "mkdir")
            flag = 7;
        else if (c == "rmdir")
            flag = 8;
        else if (c == "mv")
            flag = 9;
        else if (c == "cp")
            flag = 10;
        else if (c == "rm")
            flag = 11;
        else if (c == "ln")
            flag = 12;
        else if (c == "cat")
            flag = 13;
        else if (c == "touch")
            flag = 15;
        else if (c == ">>")
            flag = 16;
    }
    aa: switch(flag) {
        // ls
        case 1: {
            cout << ls() << endl;
            break;
        };
        // chmod (chmod Ȩ���� �ļ���/Ŀ¼��)
        case 2: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string mod = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short m = atoi(mod.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name,fileName.c_str());
                    int result = chmod(name, m);
                    if (result == STATUS_MOD_ERROR)
                        cout << "Error: Pattern is illegal..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // chown
        case 3: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string ownerId = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short oi = atoi(ownerId.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name, fileName.c_str());
                    int result = chown(name, oi);
                    if (result == STATUS_OWNER_NONEXIST)
                        cout << "Error: OwnerId is nonexisted..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // chgrp
        case 4: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string groupId = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short gi = atoi(groupId.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name,fileName.c_str());
                    int result = chgrp(name, gi);
                    if (result == STATUS_GROUP_NONEXIST)
                        cout << "Error: GroupId is nonexisted..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // pwd
        case 5: {
            cout << pwd() << endl;
            break;
        };
        // cd
        case 6:  {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error DirectName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = cd(name);
                if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: Dir is not existed..." << endl;
                else if (result == STATUS_NOT_DIRECT)
                    cout << "Error: Not a direct..." << endl;
            }
            break;
        };
        // mkdir
        case 7: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = mkdir(curINode, name);
                if(result == STATUS_NO_BLOCK)
                    cout << "Error: No Free Block..." << endl;
                else if (result == STATUS_NO_INODE)
                    cout << "Error: No Free INode..." << endl;
                else if (result == STATUS_BEYOND_DIRECT_NUM)
                    cout << "Error: Beyond Max Direct Num..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The Dir can not be written..." << endl;
                else if (result == STATUS_ERROR)
                    cout << "Error: Unexpected..." << endl;
                else if (result == STATUS_FILENAME_EXIST)
                    cout << "Error: FileName has existed..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond Your Right..." << endl;
            }
            break;
        };
        // rmdir
        case 8: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = rmdir(name);
                if (result == STATUS_NOT_DIRECT)
                    cout << "Error: Not a direct..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Byond your right..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
            }
            break;
        };
        // mv
        case 9: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string oldName = pattern.substr(0, subPos2);
                string newName = pattern.substr(subPos2 + 1);
                newName = trim(newName);
                int subPos3 = newName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char oldName1[MAX_NAME_SIZE];
                    strcpy(oldName1, oldName.c_str());
                    char newName1[MAX_NAME_SIZE];
                    strcpy(newName1, newName.c_str());
                    int result = mv(oldName1, newName1);
                    if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                    else if (result == STATUS_FILENAME_EXIST)
                        cout << "Error: NewName has existed..." << endl;
                }
            }
            break;
        };
        // cp
        case 10: {
            string pattern = command.substr(subPos+1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string source = pattern.substr(0, subPos2);
                string des = pattern.substr(subPos2 + 1);
                des = trim(des);
                int subPos3 = des.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char s[MAX_NAME_SIZE];
                    strcpy(s, source.c_str());
                    char d[MAX_NAME_SIZE];
                    strcpy(d, des.c_str());
                    int result = cp(s, d);
                    if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName doesn't exist..." << endl;
                    else if (result == STATUS_SDNAME_OVERLAP)
                        cout << "Error: SourceName overlap desName..." << endl;
                    else if (result == STATUS_NOT_FILE)
                        cout << "Error: Not a file..." << endl;
                    else if (result == STATUS_READ_ONLY)
                        cout << "Error: Read only..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond your right..." << endl;
                    else if (result == STATUS_NO_BLOCK)
                        cout << "Error: No free block..." << endl;
                    else if (result == STATUS_NO_INODE)
                        cout << "Error: No free iNode..." << endl;
                }
            }
            break;
        };
        // rm
        case 11: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = rm(name);
                if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond your right..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
            }
            break;
        };
        case 12: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string source = pattern.substr(0, subPos2);
                string des = pattern.substr(subPos2 + 1);
                des = trim(des);
                int subPos3 = des.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char s[MAX_NAME_SIZE];
                    strcpy(s, source.c_str());
                    char d[MAX_NAME_SIZE];
                    strcpy(d, des.c_str());
                    int result = ln(s, d);
                    if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond your right..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName doesn't exist..." << endl;
                    else if (result == STATUS_TYPE_NOT_MATCH)
                        cout << "Error: Source Type doesn't match des type..." << endl;
                    else if (result == STATUS_SDNAME_OVERLAP)
                        cout << "Error: SourceName overlap desName..." << endl;
                }
            };
            break;
        }
        // cat
        case 13: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = cat(name);
                if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond your right..." << endl;
                else if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_WRITE_ONLY)
                    cout << "Error: Write only..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
                else if (result == STATUS_FILE_OPEN_ERROR)
                    cout << "Error: File open error..." << endl;
            }
            break;
        }
        // password
        case 14: {
            int result = passwd();
            if (result == STATUS_PASSWORD_WRONG)
                cout << "Error: Password is wrong..." << endl;
            else if (result == STATUS_CONFIRM_WRONG)
                cout << "Error: Confirm password is wrong..." << endl;
            else if (result == STATUS_ERROR)
                cout << "Error: Unexpected error..." << endl;
            break;
        };
        // touch
        case 15: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name,fileName.c_str());
                int result = touch(curINode, name);
                if (result == STATUS_NO_BLOCK)
                    cout << "Error: No Free Block..." << endl;
                else if (result == STATUS_NO_INODE)
                    cout << "Error: No Free INode..." << endl;
                else if (result == STATUS_BEYOND_DIRECT_NUM)
                    cout << "Error: Beyond Max Direct Num..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The Dir can not be written..." << endl;
                else if (result == STATUS_ERROR)
                    cout << "Error: Unexpected..." << endl;
                else if (result == STATUS_FILENAME_EXIST)
                    cout << "Error: FileName has existed..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond Your Right..." << endl;
            }
            break;
        };
        // >>
        case 16: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = textAppend(name);
                if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Byond your right..." << endl;
                else if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The file can not be written..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
                else if (result == STATUS_BEYOND_SIZE)
                    cout << "Error: Beyond file's max size..." << endl;
                else if (result == STATUS_FILE_OPEN_ERROR)
                    cout << "Error: File open error..." << endl;
            }
            break;
        };
        // sp
        case 17: {
            cout << "FreeBlockNum:" << sp -> freeBlockNum << endl;
            cout << "NextFreeBlock:" << sp -> nextFreeBlock << endl;
            cout << "FreeBlock:" << endl;
            for (int i = 0; i <= sp -> nextFreeBlock; i ++)
                cout << "[" << i << "]:" << sp -> freeBlock[i] << endl;
            cout << "FreeINodeNum:" << sp -> freeINodeNum << endl;
            cout << "NextFreeINode:" << sp -> nextFreeINode << endl;
            cout << "FreeINode:" << endl;
            for (int j = 0; j <= sp -> nextFreeINode; j ++)
                cout << "[" << j << "]:" << sp -> freeINode[j] << endl;
            break;
        };
        // -h
        case 18: { // ��ʾָ�������÷�
            int subPos = command.find_first_of(" ");
            string c = command.substr(0, subPos);
            help(c);
            break;
        }
        // help
        case 19: { // ��ʾ���������÷�
            displayCommands();
            break;
        }
        default: {
            cout << "Error Command..." << endl;
            break;
        };
    }
    commandDispatcher();
}

void UnixFIleSys :: help(string c) {
    if (c == "ls") {
        cout << "������ ls" << endl;
        cout << "����������ʾ��ǰĿ¼�µ�ȫ���ļ���Ŀ¼" << endl;
    } else if (c == "chmod") {
        cout << "������ chmod 1 zjut.txt" << endl;
        cout << "���������޸ĵ�ǰĿ¼�µ�zjut.txt�ļ���Ȩ��Ϊ1����ִ�У�" << endl;
        cout << "dir�� 8" << endl;
        cout << "r  �� 4" << endl;
        cout << "w  �� 2" << endl;
        cout << "x  �� 1" << endl;
    } else if (c == "chown") {
        cout << "������ chowner 2 zjut.txt" << endl;
        cout << "���������޸ĵ�ǰĿ¼�µ�zjut.txt�ļ���Ȩ��ΪownerIdΪ2���û�" << endl;
    } else if (c == "chgrp") {
        cout << "������ chgrp 2 zjut.txt" << endl;
        cout << "���������޸ĵ�ǰĿ¼�µ�zjut.txt�ļ���ΪgroupIdΪ2����" << endl;
    } else if (c == "pwd") {
        cout << "������ pwd" << endl;
        cout << "��������չʾ��ǰ·��" << endl;
    } else if (c == "cd") {
        cout << "������ cd zjut" << endl;
        cout << "�������Խ��뵱ǰĿ¼�µ���Ϊzjut����Ŀ¼" << endl;
    } else if (c == "mkdir") {
        cout << "������ mkdir zjut" << endl;
        cout << "���������ڵ�ǰĿ¼�´���zjutĿ¼�����ҽ�����ǰ�û�ӵ���Ǹ�Ŀ¼��ӵ���߻��ǹ���Ա" << endl;
    } else if (c == "rmdir") {
        cout << "������ rmdir zjut" << endl;
        cout << "��������ɾ����ǰĿ¼�´���zjutĿ¼������ɾ��zjutĿ¼�µ�ȫ���ļ���Ŀ¼" << endl;
    } else if (c == "mv") {
        cout << "������ mv zjut zjut1" << endl;
        cout << "���������޸ĵ�ǰĿ¼��zjutĿ¼��Ϊzjut1" << endl;
    } else if (c == "cp") {
        cout << "������ cp zjut.txt zjut1.txt" << endl;
        cout << "�������Ը��Ƶ�ǰĿ¼��zjut.txt�ļ����ڵ�ǰĿ¼������һ����zjut.txt������ͬ��zjut1.txt�ļ�" << endl;
    } else if (c == "rm") {
        cout << "������ rm zjut.txt" << endl;
        cout << "��������ɾ����ǰĿ¼�µ�zjut.txt��������������Ӿͼ�ȥһ�������Ӳ���ɾȥĿ¼��" << endl;
    } else if (c == "ln") {
        cout << "������ ln zjut.txt zjut1.txt" << endl;
        cout << "��������Ϊzjut.txt����һ�������ӵ�zjut1.txt" << endl;
    } else if (c == "cat") {
        cout << "������ cat zjut.txt" << endl;
        cout << "��������չʾzjut.txt�ļ�������" << endl;
    } else if (c == "passwd") {
        cout << "������ passwd" << endl;
        cout << "�������Ը��ݺ���ϵͳ��ʾ�޸�����" << endl;
    } else if (c == "touch") {
        cout << "������ touch zjut.txt" << endl;
        cout << "���������ڵ�ǰĿ¼�´���һ����Ϊzjut.txt���ļ�" << endl;
    } else if (c == ">>") {
        cout << "������ >> zjut.txt" << endl;
        cout << "���������ڵ�ǰĿ¼��Ϊһ����Ϊzjut.txt���ļ�׷�����ݡ��ԣ�wq���н�����" << endl;
    } else if (c == "sp") {
        cout << "������ sp" << endl;
        cout << "��������չʾ��ʱ���������Ϣ" << endl;
    } else if (c == "-h") {
        cout << "������ ls -h" << endl;
        cout << "��������չʾ����ls��ʹ�÷���" << endl;
    } else if (c == "help") {
        cout << "������ help" << endl;
        cout << "��������չʾȫ�������ʹ�÷���" << endl;
    }
    else
        cout<<"Error Command..."<<endl;
}

// չʾָ��
void UnixFIleSys :: displayCommands() {
    cout<<"ls		��ʾĿ¼�ļ�"<<endl;
    cout<<"chmod		�ı��ļ�Ȩ��"<<endl;
    cout<<"chown		�ı��ļ�ӵ����"<<endl;
    cout<<"chgrp		�ı��ļ�������"<<endl;
    cout<<"pwd		��ʾ��ǰĿ¼"<<endl;
    cout<<"cd		�ı䵱ǰĿ¼"<<endl;
    cout<<"mkdir		������Ŀ¼"<<endl;
    cout<<"rmdir		ɾ����Ŀ¼"<<endl;
    cout<<"mv		�ı��ļ���"<<endl;
    cout<<"cp		�ļ�����"<<endl;
    cout<<"rm		�ļ�ɾ��"<<endl;
    cout<<"ln		�����ļ�����"<<endl;
    cout<<"cat		������ʾ�ļ�����"<<endl;
    cout<<"passwd		�޸��û�����"<<endl;
    cout<<"touch		�����ļ�"<<endl;
    cout<<">>		�ı�����׷��"<<endl;
}

// չʾ��ǰĿ¼�µ�ȫ��Ŀ¼���ļ�
// ����ȫ�ֱ���d����ǰĿ¼������ӡÿһ��Ŀ¼�������
string UnixFIleSys :: ls() {
    string ls;
    for (int i = 0; i < d -> dirNum; i++) {
        ls += d -> direct[i].name;
        ls += " ";
    }
    return ls;
}

// ����Ŀ¼
// �������л�curINode
int UnixFIleSys :: cd(char name[MAX_NAME_SIZE]) {
    Direct *dt = new Direct();
    // ����Ҫ���뵱ǰĿ¼��������������
    if (strcmp(name, "./") == 0) {
        delete dt;
        return STATUS_OK;
    } else if (strcmp(name, "../") == 0) {
    // ����Ҫ�����ϼ���������ȫ�ֱ���curINode���ɸ��ڵ㣬ͬʱ·��ջ����
        if (curINode -> parent != nullptr) {
            INode *temp = curINode;
            curINode = curINode -> parent;
            delete temp;
            Direct* t = ds[ds.size() - 1];
            ds.pop_back();
            delete t;
            readCurDir();
            return STATUS_OK;
        }
        delete dt;
        return STATUS_OK;
    }
    else {
    // ����Ŀ¼��ͨ��name�ҵ�NodeId������INode��д��curINode
        bool dtExist = false;
        for (int i = 0; i < d -> dirNum; i ++)
            if (strcmp(d -> direct[i].name, name) == 0) {
                *dt = d -> direct[i];
                dtExist = true;
            }
        if (!dtExist) {
            delete dt;
            return STATUS_FILENAME_NONEXIST;
        } else {
            INode *now = new INode();
            now -> nodeId = dt -> iNodeId;
            now -> parent = curINode;
            now -> users = curINode -> users;
            readINode(now);
            if (now -> dINode.mod > 7) {
                curINode = now;
                ds.push_back(dt);
                readCurDir();
                return STATUS_OK;
            } else {
                delete dt;
                delete now;
                return STATUS_NOT_DIRECT;
            }
        }
    }
}

// ����Ȩ��
// �������޸�dINode��mod����
int UnixFIleSys :: chmod(char name[MAX_NAME_SIZE], unsigned int mod)	{
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            if (mod == 8 || mod == 9 || mod == 11 || mod == 13 || mod > 14)
                return STATUS_MOD_ERROR;
            else {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (mod < 8 && temp -> dINode.mod < 8 && (temp -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
                    temp -> dINode.mod = mod;
                    writeINode(temp);
                    delete temp;
                    return STATUS_OK;
                } else if (mod > 9 && temp -> dINode.mod > 9 && (temp -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
                    temp -> dINode.mod = mod;
                    writeINode(temp);
                    delete temp;
                    return STATUS_OK;
                } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    delete temp;
                    return STATUS_MOD_ERROR;
                }
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// �޸�ӵ����
// �������޸�dINode��ownerId����
int UnixFIleSys :: chown(char name[MAX_NAME_SIZE], unsigned short ownerId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            for (int j = 0; j < os -> ownerNum; j ++)
                if (ownerId == os -> os[j].ownerId) {
                    INode *temp = new INode();
                    temp -> nodeId = d -> direct[i].iNodeId;
                    readINode(temp);
                    if (curOwner -> ownerId == temp -> dINode.ownerId || curOwner -> ownerId == ROOT) {
                        temp -> dINode.ownerId = ownerId;
                        writeINode(temp);
                        delete temp;
                        return STATUS_OK;
                    }  else {
                        delete temp;
                        return STATUS_BEYOND_RIGHT;
                    }
                }
            return STATUS_OWNER_NONEXIST;
        }
    return STATUS_FILENAME_NONEXIST;
}

// �޸�������
// �������޸�dINode��groupId����
int UnixFIleSys :: chgrp(char name[MAX_NAME_SIZE], unsigned short groupId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            for (int j = 0; j < gs -> groupNum; j ++)
                if (groupId == gs -> gs[j].groupId) {
                    INode *temp = new INode();
                    temp -> nodeId = d -> direct[i].iNodeId;
                    readINode(temp);
                    if (curOwner -> ownerId == temp -> dINode.ownerId || curOwner -> ownerId == ROOT) {
                        temp -> dINode.groupId = groupId;
                        writeINode(temp);
                        delete temp;
                        return STATUS_OK;
                    } else {
                        delete temp;
                        return STATUS_BEYOND_RIGHT;
                    }
                }
            return STATUS_GROUP_NONEXIST;
        }
    return STATUS_FILENAME_NONEXIST;
}

// �޸�����
// �������޸�Owners�еĶ�ӦOwner��ownerPassword����
int UnixFIleSys :: passwd() {
    cout << "please input your old password:";
    char password[MAX_NAME_SIZE]={0};
    int i = 0;
    while (i < MAX_NAME_SIZE) {
        password[i] = getchar();
        if (password[i] == '\n' && i == 0) i = -1;
        if (i != -1 && password[i] == '\n') {
            cout << "��⵽����" << endl;
            for (int j = i; j <= 13; j ++) {
                password[j]='\0';
            }
            break;
        }
        if (password[i] == 13) {
            password[i]='\0';
            break;
        }
        i++;
    }
    cout<<endl;
    // ��֤�������Ƿ���ȷ
    if (strcmp(curOwner -> ownerPassword, password) != 0)
        return STATUS_PASSWORD_WRONG;
    else {
        cout << "please input your new password:";
        char p1[MAX_NAME_SIZE]={0};
        int i1 = 0;
        while (i1 < MAX_NAME_SIZE) {
            password[i1] = getchar();
            if (password[i1] == '\n' && i1 == 0) i1 = -1;
            if (i1 != -1 && password[i1] == '\n') {
                cout << "��⵽����" << endl;
                for (int j = i1; j <= 13; j ++) {
                    password[j]='\0';
                }
                break;
            }
            if (password[i1] == 13) {
                password[i1]='\0';
                break;
            }
            i1 ++;
        }
        cout<<endl;
        cout << "please input your old password:";
        char p2[MAX_NAME_SIZE]={0};
        int i2 = 0;
        while (i2 < MAX_NAME_SIZE) {
            password[i2] = getchar();
            if (password[i2] == '\n' && i2 == 0) i2 = -1;
            if (i2 != -1 && password[i2] == '\n') {
                cout << "��⵽����" << endl;
                for (int j = i2; j <= 13; j ++) {
                    password[j]='\0';
                }
                break;
            }
            if (password[i2] == 13) {
                password[i2]='\0';
                break;
            }
            i2 ++;
        }
        cout << endl;
        if (strcmp(p1, p2) != 0)
            return STATUS_CONFIRM_WRONG;
        else {
            strcpy(curOwner -> ownerPassword, p1);
            for (int i = 0; i < os -> ownerNum; i ++)
                if (os -> os[i].ownerName == curOwner -> ownerName) {
                    strcpy(os->os[i].ownerPassword, p1);
                    writeOS();
                    return STATUS_OK;
                }
        }
    }
    return STATUS_ERROR;
}

// �������ļ�
// �������޸�Ŀ¼���name
int UnixFIleSys :: mv(char oldName[MAX_NAME_SIZE], char newName[MAX_NAME_SIZE]) {
    for (int i = 0; i < d -> dirNum; i++) {
        if (strcmp(d -> direct[i].name, oldName) == 0) {
            INode* temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            } else {
                for (int j = 0; j < d -> dirNum; j ++)
                    if (strcmp(d -> direct[j].name, newName) == 0)
                        return STATUS_FILENAME_EXIST;
                strcpy(d -> direct[i].name, newName);
                writeDir(curINode -> dINode.addr[0], d);
                delete temp;
                return STATUS_OK;
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

// �½��ļ�
// �����ǻ�ȡһ�����п飨�����ݣ�����ȡһ���µ�INode�����ļ�������Ϣ����Ȼ���޸�Ŀ¼
int UnixFIleSys :: touch(INode* parent, char name[MAX_NAME_SIZE]) {
    bool exist = checkFileName(name);
    // ���ԭ����dINode���ļ���СΪ0��
        // ��ȡһ�����п飨�����ݣ����ڸ�dINode��׷�����ļ����ݴ洢�Ŀ�ţ�
        // ��ȡһ���µ�INode�����ļ�������Ϣ������INOdeId����һ���µ�Ŀ¼�����Ŀ¼
    if (parent -> dINode.fileSize == 0 && parent -> dINode.mod != 12 && !exist &&
            (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int blockId = getFreeBlock();
        if (blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(nullptr);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = parent -> dINode.groupId;
                di -> linkNum = 0;
                di -> mod = 6;
                di -> modifyTime = now;
                di -> ownerId = parent -> dINode.ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = parent -> users;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    // ���ԭ����dINode���ļ���С��Ϊ0
        // ��ȡһ���µ�INode�����ļ�������Ϣ������INOdeId����һ���µ�Ŀ¼�����Ŀ¼
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist &&
            (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(nullptr);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = parent -> dINode.groupId;
            di -> linkNum = 0;
            di -> mod = 6;
            di -> modifyTime = now;
            di -> ownerId = parent -> dINode.ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = parent -> users;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd);
            if (dd.dirNum < MAX_DIRECT_NUM) {
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
            else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    }
    else if (parent -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
        return STATUS_BEYOND_RIGHT;
    else if (parent -> dINode.mod == 12)
        return STATUS_READ_ONLY;
    else
    if(exist)
        return STATUS_FILENAME_EXIST;
    return STATUS_ERROR;
}

// ׷������
// ͨ������Ŀ¼�Ա��ļ����ҵ�Ŀ¼���ȡINodeId����INode����Ϣ����ȡ׷�ӵ��ַ���Ϣ��׷��
int UnixFIleSys :: textAppend(char name[MAX_NAME_SIZE])	{
    for (int i = 0; i < d -> dirNum; i ++) {
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode* temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
                return STATUS_BEYOND_RIGHT;
            else if (temp -> dINode.mod > 7) {
                delete temp;
                return STATUS_NOT_FILE;
            } else if (temp -> dINode.mod != 2 && temp -> dINode.mod != 3 && temp -> dINode.mod != 6 && temp -> dINode.mod != 7)  {
                delete temp;
                return STATUS_READ_ONLY;
            } else {
                // ���ԭ�ļ���С�������׷�ӣ����ϵض�ȡ�ַ�����ַ���text��׷��
                if (temp -> dINode.fileSize < FILE_MAX_SIZE) {
                    string text;
                    int j = 0;
                    while (1) {
                        text.push_back(getchar());
                        if (text[j] == 'q' && text[j - 1] == 'w' && text[j - 2] == ':' && text[j - 3] == '\n') {
                            cout << "��⵽����" << endl;
                            text.pop_back();
                            text.pop_back();
                            text.pop_back();
                            text.pop_back();
                            break;
                        }
                        j ++;
                    }
                    int result = writeText(temp, text);
                    writeINode(temp);
                    delete temp;
                    return result;
                }
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

// ��ȡ�ļ�����
// ͨ������Ŀ¼�Ա��ļ����ҵ�Ŀ¼���ȡINodeId����INode����Ϣ����ȡ
int UnixFIleSys :: cat(char name[MAX_NAME_SIZE]) {
    for (int i = 0;i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
                return STATUS_BEYOND_RIGHT;
            else if (temp -> dINode.mod > 7)
                return STATUS_NOT_FILE;
            else if (temp -> dINode.mod < 4)
                return STATUS_WRITE_ONLY;
            else {
                int result = readText(temp);
                delete temp;
                return result;
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// ɾ��
// ͨ������Ŀ¼�Ա��ļ����ҵ�Ŀ¼���ȡINodeId����INode����Ϣ
    // ����������ӣ���ɾ��Ŀ¼�������ɾ���ļ�
    // ����ļ���СΪ0������INode��ɾ��Ŀ¼��
int UnixFIleSys :: rm(char name[MAX_NAME_SIZE]) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.mod > 7) {
                delete temp;
                return STATUS_NOT_FILE;
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            // ����������ӣ���ɾ��Ŀ¼�������ɾ���ļ�
            } else if (temp -> dINode.linkNum > 0) {
                temp -> dINode.linkNum --;
                writeINode(temp);
                Dir* td = new Dir();
                td -> dirNum = d -> dirNum - 1;
                int k = 0;
                for (int j = 0; j < d -> dirNum; j ++)
                    // ���ǵ�Ҫɾ����Ŀ¼��
                    if (j != i)
                        td -> direct[k ++] = d -> direct[j];
                writeDir(curINode -> dINode.addr[0], td);
                Dir* dd = d;
                d = td;
                delete temp;
                delete dd;
                return STATUS_OK;
            } else {
                int bs = temp -> dINode.fileSize / BLOCK_SIZE;
                int ls = temp -> dINode.fileSize % BLOCK_SIZE;
                // ����ļ���СΪ0������INode��ɾ��Ŀ¼��
                if (temp -> dINode.fileSize == 0) {
                    returnFreeINode(temp -> nodeId);
                    Dir* td=new Dir();
                    td -> dirNum = d -> dirNum - 1;
                    int k = 0;
                    for (int j = 0; j < d -> dirNum; j ++)
                        if (j != i)
                            td -> direct[k ++] = d -> direct[j];
                    writeDir(curINode -> dINode.addr[0], td);
                    Dir* dd = d;
                    d = td;
                    delete temp;
                    delete dd;
                    return STATUS_OK;
                } else {
                // ����ļ���С��Ϊ�㣬�����գ��ٻ���INode�����ɾ��Ŀ¼��
                    for (int ii = 0; ii < (ls > 0 ? bs + 1 : bs); ii ++)
                        returnFreeBlock(temp -> dINode.addr[ii]);
                    returnFreeINode(temp -> nodeId);
                    Dir* td = new Dir();
                    td -> dirNum = d -> dirNum - 1;
                    int k = 0;
                    for (int j = 0; j < d -> dirNum; j ++)
                        if (j != i)
                            td -> direct[k ++] = d -> direct[j];
                    writeDir(curINode -> dINode.addr[0], td);
                    Dir* dd = d;
                    d = td;
                    delete temp;
                    delete dd;
                    return STATUS_OK;
                }
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// ����������
// �����Ǵ���һ���µ�Ŀ¼�Ŀ¼���е�INodeIdΪsource�ļ���INodeId
int UnixFIleSys :: ln(char source[MAX_NAME_SIZE], char des[MAX_NAME_SIZE]) {
    if (strcmp(source, des) == 0)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            // ͨ���ļ����ҵ�source�ļ�
            if(strcmp(d -> direct[i].name, source) == 0) {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    for (int j = 0; j < d -> dirNum; j ++)
                        // ���Ŀ���ļ��Ѿ����ڣ���ɾ��
                        if (strcmp(d -> direct[j].name, des) == 0) {
                            INode* t = new INode();
                            t -> nodeId = d -> direct[j].iNodeId;
                            readINode(t);
                            if (t -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                                delete t;
                                delete temp;
                                return STATUS_BEYOND_RIGHT;
                            }
                            if((t -> dINode.mod < 8 && temp -> dINode.mod > 7) || (t -> dINode.mod > 7 && temp -> dINode.mod < 8)) {
                                delete t;
                                delete temp;
                                return STATUS_TYPE_NOT_MATCH;
                            }
                            rmIter(d -> direct[j].iNodeId);
                            d -> direct[j].iNodeId = d -> direct[i].iNodeId;
                            writeDir(curINode -> dINode.addr[0], d);
                            delete t;
                            delete temp;
                            return STATUS_OK;
                        }
                    // ����Ŀ¼�INodeId��Ӧ�����õ��ļ���INodeId
                    Direct dt;
                    dt.iNodeId = d -> direct[i].iNodeId;
                    strcpy(dt.name, des);
                    d -> direct[d -> dirNum] = dt;
                    d -> dirNum ++;
                    writeDir(curINode -> dINode.addr[0], d);
                    delete temp;
                    return STATUS_OK;
                }
            }
    return STATUS_FILENAME_NONEXIST;
}

// ɾ�����ļ���Ŀ¼��
// ͨ�������INodeId�ж�ʹ�ļ�����Ŀ¼��ɾ���ı���ʹ�ǹ黹�ڵ�Ϳ�
void UnixFIleSys :: rmIter(unsigned short iNodeId) {
    INode* temp = new INode();
    temp -> nodeId = iNodeId;
    readINode(temp);
    // �����������inodeid��Ӧ�����ļ�
    if (temp -> dINode.mod < 8) {
        // �������������0���ͼ���һ��������
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
        // ���������Ϊ0��ֱ��ɾ��
            // �黹�ڵ�
            returnFreeINode(temp -> nodeId);
            if (temp -> dINode.fileSize != 0) {
                int bs = temp -> dINode.fileSize / BLOCK_SIZE;
                int ls = temp -> dINode.fileSize % BLOCK_SIZE;
                for (int i = 0; i < (ls > 0 ? bs + 1 : bs); i ++)
                    returnFreeBlock(temp -> dINode.addr[i]);
                delete temp;
            }
        }
    } else {
    // �������������Ŀ¼
        // �������������0���ͼ���һ��������
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
        // ���������Ϊ0��ֱ��ɾ��
            // �黹�ڵ�
            returnFreeINode(temp -> nodeId);
            if (temp -> dINode.fileSize != 0) {
                Dir *dd=new Dir();
                readDir(temp -> dINode.addr[0], dd);
                // ����Ŀ¼ɾ��ÿһ��
                for (int i = 0; i < dd -> dirNum; i ++)
                    rmIter(dd -> direct[i].iNodeId);
                returnFreeBlock(temp -> dINode.addr[0]);
                delete dd;
                delete temp;
            }
        }
    }
}

// ɾ��Ŀ¼
// ʵ���ǽ���Ŀ¼ɾ��Ŀ¼�е����ݣ�ͨ��Ŀ¼���INodeId������ɾ��Ŀ¼��ƶ�֮ǰ��Ŀ¼ʹ��ɾ����Ŀ¼����ǣ�
int UnixFIleSys :: rmdir(char name[MAX_NAME_SIZE])	{
    for (int i = 0; i < d -> dirNum; i ++)
        // �ҵ�����ƥ���Ŀ¼
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            // ���ļ��ͱ���
            if (temp -> dINode.mod < 8) {
                delete temp;
                return STATUS_NOT_DIRECT;
            // Ȩ��У��δͨ��
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            // ͨ��Ȩ��У��
            } else {
                // ����Ŀ¼ɾ��ÿһ��
                rmIter(d -> direct[i]. iNodeId);
                Dir* td = new Dir();
                td -> dirNum = d -> dirNum - 1;
                int k = 0;
                // ������Ҫɾ����Ŀ¼��
                for (int j = 0; j < d -> dirNum; j ++)
                    if (j != i)
                        td -> direct[k ++] = d -> direct[j];
                writeDir(curINode -> dINode.addr[0], td);
                Dir* dd = d;
                d = td;
                delete temp;
                delete dd;
                return STATUS_OK;

            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// ����
// �½�Ŀ¼�name��Ŀ���ļ���������ȡһ������INode����ȡһ�����п飨����touch��������д�����ݣ�������Դ�ļ������ݣ�
int UnixFIleSys :: cp(char source[MAX_NAME_SIZE], char des[MAX_NAME_SIZE]) {
    if (strcmp(source, des) == 0)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            if (strcmp(d -> direct[i].name, source) == 0) {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (temp -> dINode.mod > 7) {
                    delete temp;
                    return STATUS_NOT_FILE;
                } else if (temp -> dINode.mod < 4) {
                    delete temp;
                    return STATUS_READ_ONLY;
                } else if (temp -> dINode.ownerId != curOwner -> ownerId && temp -> dINode.ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    for (int j = 0; j < d -> dirNum; j ++)
                        // ���Ŀ���ļ��Ѿ����ڣ���ɾ��
                        if (d -> direct[j].name == des) {
                            int r1 = rm(des);
                            if (r1 != STATUS_OK) {
                                delete temp;
                                return r1;
                            }
                        }
                    int r2 = touch(curINode, des);
                    if (r2 != STATUS_OK) {
                        delete temp;
                        return r2;
                    }
                    for (int k = 0; k < d -> dirNum; k ++)
                        if (strcmp(d -> direct[k].name, des) == 0) {
                            INode *t = new INode();
                            t -> nodeId = d -> direct[k].iNodeId;
                            readINode(t);
                            string text = getText(temp);
                            writeText(t, text);
                            writeINode(t);
                            delete t;
                            delete temp;
                            return STATUS_OK;
                        }
                }
            }
    return STATUS_FILENAME_NONEXIST;
}

