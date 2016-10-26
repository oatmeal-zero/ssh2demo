#include "command.h"

TCommand commandlist[] = 
{
    {1, Command::quit,          "退出程序"},
    {2, Command::back,          "后退"},
    {3, Command::update_gs,     "更新游戏服逻辑代码"},
    {4, Command::update_db,     "更新配置数据库的配置"},
    {5, Command::update_all,    "更新所有的主机代码和配置"},
    {6, Command::stop_gs,       "关闭所有游戏服务"},
    {7, Command::run_gs,        "启动所有游戏服务"},
};

int Command::back(CHostMgr* hostmgr)
{
    /* 返回1表示后退 */
    return 2;
}

int Command::quit(CHostMgr* hostmgr)
{
    /* 返回1表示退出 */
    return 1;
}

int Command::update_all(CHostMgr* hostmgr)
{
    return hostmgr->update(HT_DB | HT_GS);
}

int Command::update_db(CHostMgr* hostmgr)
{
    return hostmgr->update(HT_DB);
}

int Command::update_gs(CHostMgr* hostmgr)
{
    return hostmgr->update(HT_GS);
}

int Command::stop_gs(CHostMgr* hostmgr)
{
    return hostmgr->stop();
}

int Command::run_gs(CHostMgr* hostmgr)
{
    return hostmgr->run();
}

void Command::list()
{
    int length = sizeof(commandlist) / sizeof(TCommand);
    printf("**目前提供的可用的命令如下:(%d)\n", length);
    for (int idx = 0; idx < length; ++idx)
    {
        TCommand command = commandlist[idx];
        printf("**[%02d] %s\n", command.code, command.desc); 
    }
}

int Command::execute(int code, CHostMgr* hostmgr)
{
    int rtCode = -1;
    int length = sizeof(commandlist) / sizeof(TCommand);
    for (int idx = 0; idx < length; ++idx) {
        TCommand command = commandlist[idx];
        if (command.code == code) {
            rtCode = command.func(hostmgr);
            break;
        }
    }
    return rtCode;
}
