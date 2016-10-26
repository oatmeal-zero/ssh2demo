#pragma once
#include "config.h"

typedef int (*cmd_func) (CHostMgr*);

struct TCommand 
{
    int         code;
    cmd_func    func;
    const char *desc;
};

class Command
{
public:
    static int back(CHostMgr* hostmgr);
    static int quit(CHostMgr* hostmgr);
    static int update_all(CHostMgr* hostmgr);
    static int update_db(CHostMgr* hostmgr);
    static int update_gs(CHostMgr* hostmgr);
    static int stop_gs(CHostMgr* hostmgr);
    static int run_gs(CHostMgr* hostmgr);
    static void list();
    static int execute(int code, CHostMgr* hostmgr);
};
