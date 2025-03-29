/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include "amigos_database.h"

class AmigosDatabaseFactory : public AmigosDatabaseFactoryBase
{
public:
    AmigosDatabaseFactory(const std::string &strIn);
    AmigosDatabaseFactory(const std::string &strIn, const std::string &strOut);
    const std::string &GetInFile();
    const std::string &GetOutFile();
    int LoadFile();
    int UnloadFile();
    AmigosDatabase *Create(const std::string &strSec) override final;
    void Destroy(AmigosDatabase *pDb) override final;
private:
    std::string strOutputFile;
    std::string strInputFile;
};
