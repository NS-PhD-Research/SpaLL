#ifndef MATHCMAKER_H_
#define MATHCMAKER_H_

#include <string>
#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>
#include <algorithm>
#include <random>
#include <fstream>

#include "../include/rtree.h"

#define MINLOAD 40
#define MAXLOAD 80

enum PlacementType{
    DIST,
    LAT,
    SPATIAL
};
class MatchMaker;

struct GlobalInfo {
    MatchMaker *matchmaker_;
    std::map<int, Host*> hosts;
    std::map<int, Producer*> producers;
    std::map<int, Consumer*> consumers;
    PlacementType ptype;
    RTree *rtree;
    std::mutex rtree_mutex;
    std::vector<float> host_lb, host_ub;
    std::ofstream fout;
    std::mutex log_mutex;
    
    std::mutex prods_mutex;
    int n_prods;

    std::mutex host_mutex;
    int n_hosts;

    int replica_threshold;
};

struct Request {
    int id_;
    int type_;
    Request(int id, int type): id_(id), type_(type){}
};

class MatchMaker {
    public:

        std::mt19937 engine_;

        int id_;
        GlobalInfo *ginfo_;
        RequestQueue<Request> prod_queue_, cons_queue_;

        void processProdRequestQueue();
        void processConsRequestQueue();

        int selectHostForProducer(int prod_id, bool replica, int cons_id);
        void selectHostForConsumer(int cons_id);
        int selectReplicaForConsumer(int cons_id, int prod_id);

        float generateRandomFloat(float min_val, float max_val){
            std::uniform_real_distribution<> dis(min_val, max_val);
            return dis(engine_);
        }

        MatchMaker(){
            engine_.seed(10);
        }
        
};
#endif