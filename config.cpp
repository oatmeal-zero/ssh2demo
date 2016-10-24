#include <fstream>
#include <cassert>
#include "config.h"
#include "json/json.h"
#include "SshClient.h"

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
        cout << "[" << iter->first << "]" << (iter->second).name << endl;
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
            hostlist[idx]["HostType"].asString(),
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

int THost::update(TSvn *svn)
{
    if (Disable) {
        printf("skip host [%s:%d]\n", IpAddr.c_str(), Port);
        return 0;
    }

    printf("update host [%s:%d]\n", IpAddr.c_str(), Port);
    SshClient ssh(IpAddr.c_str(), Port);
    ssh.connect(UserName.c_str(), Passwd.c_str());

    char command[1024]{0};
    sprintf(command, "cd %s\n"
            "svn up %s --username %s --password %s --non-interactive",
            ProjectPath.c_str(), ProjectPath.c_str(), 
            svn->username.c_str(), svn->passwd.c_str());
    ssh.execute(command);
    return 0;
}

int CHostMgr::update() 
{
    for (auto iter = _hostlist.begin(); iter != _hostlist.end(); ++iter)
    {
        (*iter).update(&_svn);
    }

    return 0;
}

int CHostMgr::stop()
{
    return 0;
}

int CHostMgr::run()
{
    return 0;
}
