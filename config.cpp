#include <fstream>
#include <cassert>
#include "config.h"
#include "json/json.h"
#include "SshClient.h"

static int GetHostType(string cfg)
{
    switch (cfg[0])
    {
        case 'D':
            return HT_DB;
        case 'G':
            return HT_GS;
    }
    return HT_NIL;
}

void CSvrGrp::load(string config)
{
    ifstream ifs;
    ifs.open(config);
    assert(ifs.is_open());

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root, false))
        throw runtime_error("无法解析json配置文件，请检查!");

    Json::Value::Members mem = root.getMemberNames();
    for (auto iter = mem.begin(); iter != mem.end(); ++iter)
    {
        TSvrGrp grp = { 
            root[*iter]["SvrGroupName"].asString(), 
            root[*iter]["config"].asString()
        };
        addGrp(*iter, grp);
    }

    ifs.close();
}

void CSvrGrp::list()
{
    for (auto iter = _grps.begin(); iter != _grps.end(); ++iter)
    {
        printf("[%s] %s\n", iter->first.c_str(), (iter->second).name.c_str());
    }
}

void CHostMgr::load(string config)
{
    ifstream ifs;
    ifs.open(config);
    assert(ifs.is_open());

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root, false))
        throw runtime_error("无法解析json配置文件，请检查!");

    // svn
    _svn = {
        root["svn"]["UserName"].asString(),
        root["svn"]["Passwd"].asString()
    };

    // hostlist
    Json::Value hostlist = root["hostlist"];
    //Json::Value::ArrayIndex index = hostlist.size();
    int index = hostlist.size();
    for (int idx = 0; idx < index; ++idx)
    {
        THost host = { 
            hostlist[idx]["Disable"].asBool(), 
            //hostlist[idx]["HostType"].asString(),
            GetHostType(hostlist[idx]["HostType"].asString()),
            hostlist[idx]["IpAddr"].asString(),
            hostlist[idx]["Port"].asInt(),
            hostlist[idx]["UserName"].asString(),
            hostlist[idx]["Passwd"].asString(),
            hostlist[idx]["ProjectPath"].asString(),
            hostlist[idx]["Caption"].asString()
        };
        addHost(host);
    }

    ifs.close();
}

void CHostMgr::list()
{
    printf(" %-6s       %-18s   %-6s        %-30s  %-30s\n", 
            "有效", "服务器地址", "端口", "项目目录", "服务器名称");
    for (auto iter = _hostlist.begin(); iter != _hostlist.end(); ++iter)
    {
        printf("  %-6s %-18s %-6d %-30s %-30s\n", 
                (*iter).Disable ? "N" : "Y",
                (*iter).IpAddr.c_str(),
                (*iter).Port,
                (*iter).ProjectPath.c_str(),
                (*iter).Caption.c_str());
    }
}

int THost::update(TSvn *svn, int hostType)
{
    if (Disable) {
        printf("skip host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    if (!(HostType & hostType)) {
        printf("skip host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    printf("update host [%s:%d]\n", IpAddr.c_str(), Port);
    SshClient ssh(IpAddr.c_str(), Port);
    ssh.connect(UserName.c_str(), Passwd.c_str());

    char command[1024]{0};
    if (HostType == HT_DB) {
        sprintf(command, "cd %s\n"
                "source /etc/profile\n"
                "source ~/.bash_profile\n"
                "svn up %s --username %s --password %s --non-interactive\n"
                "sh auto_import.sh",
                ProjectPath.c_str(), ProjectPath.c_str(), 
                svn->username.c_str(), svn->passwd.c_str());
    } else {
        sprintf(command, "cd %s\n"
                "source /etc/profile\n"
                "source ~/.bash_profile\n"
                "svn up %s --username %s --password %s --non-interactive",
                ProjectPath.c_str(), ProjectPath.c_str(), 
                svn->username.c_str(), svn->passwd.c_str());
    }
    ssh.execute(command);
    return 0;
}

int THost::stop()
{
    if (Disable) {
        printf("skip host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    if (HostType == HT_DB) {
        printf("skip db host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    printf("stop host [%s:%d]\n", IpAddr.c_str(), Port);
    SshClient ssh(IpAddr.c_str(), Port);
    ssh.connect(UserName.c_str(), Passwd.c_str());

    char command[1024]{0};
    sprintf(command, "cd %s\n"
            "source /etc/profile\n"
            "source ~/.bash_profile\n"
            "sh stop.sh",
            ProjectPath.c_str());
    ssh.execute(command);
    return 0;
}

int THost::run()
{
    if (Disable) {
        printf("skip host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    if (HostType == HT_DB) {
        printf("skip db host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    printf("run host [%s:%d]\n", IpAddr.c_str(), Port);
    SshClient ssh(IpAddr.c_str(), Port);
    ssh.connect(UserName.c_str(), Passwd.c_str());

    char command[1024]{0};
    sprintf(command, "cd %s\n"
            "source /etc/profile\n"
            "source ~/.bash_profile\n"
            "sh run.sh",
            ProjectPath.c_str());
    ssh.execute(command);
    return 0;
}

int CHostMgr::update(int hostType) 
{
    for (auto iter = _hostlist.begin(); iter != _hostlist.end(); ++iter)
    {
        (*iter).update(&_svn, hostType);
    }

    return 0;
}

int CHostMgr::stop()
{
    for (auto iter = _hostlist.begin(); iter != _hostlist.end(); ++iter)
    {
        (*iter).stop();
    }

    return 0;
}

int CHostMgr::run()
{
    for (auto iter = _hostlist.begin(); iter != _hostlist.end(); ++iter)
    {
        (*iter).run();
    }

    return 0;
}
