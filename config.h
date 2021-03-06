#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

#define HT_NIL  0
#define HT_DB   1 << 0
#define HT_GS   1 << 1

struct TSvrGrp
{
    string name;
    string config;
};

struct TSvn
{
    string username;
    string passwd;
};

struct THost
{
    bool Disable;
    int HostType;
    string IpAddr;
    int Port;
    string UserName;
    string Passwd;
    string ProjectPath;
    string Caption;

    int update(TSvn *svn, int hostType);
    int stop();
    int run();
};

class CSvrGrp
{
    map<string, TSvrGrp> _grps;
public:
    void load(string config);
    void list();

    void addGrp(string svr, TSvrGrp grp) {
        _grps[svr] = grp;
    }

    TSvrGrp* getGrp(string svr) {
        auto iter = _grps.find(svr);
        if (iter == _grps.end()) return NULL;
        return &iter->second;
    }
};

class CHostMgr
{
    TSvn _svn;
    vector<THost> _hostlist;
public:
    void load(string config);
    void list();
    int update(int hostType);
    int stop();
    int run();

    void addHost(THost host) {
        _hostlist.push_back(host);
    }
};

#endif
