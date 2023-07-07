#include "../include/main.h"

std::mt19937 engine_(10);
int GenerateRandomInt(int min_val, int max_val){
    std::uniform_int_distribution<> dis(min_val, max_val);
    return dis(engine_);
}

float GenerateRandomFloat(float min_val, float max_val){
    std::uniform_real_distribution<> dis(min_val, max_val);
    return dis(engine_);
}

float displacement(std::vector<float> &loc1, std::vector<float> &loc2) {

	float x = (loc1[0] - loc2[0]) * (loc1[0] - loc2[0]);
	float y = (loc1[1] - loc2[1]) * (loc1[1] - loc2[1]);

	float dist = x + y;

	return dist;
}

float nextTime(float rateParameter){
    return -logf(1.0f - (float) random() / (RAND_MAX + 1.0)) / rateParameter;
}

void tokenize(std::string line, std::string delim, std::vector<std::string> &tokens) {
    int start = 0;
    int end = line.find(delim);
    while (end != -1) {
        tokens.push_back(line.substr(start, end - start));
        start = end + delim.size();
        end = line.find(delim, start);
    }
    tokens.push_back(line.substr(start, end - start));
}

void readClusterData(int n_producers, int n_hosts, int n_consumers){
    std::map<int, std::vector<std::string>> cluster_data;

    int nclusters = 1;
    int entity = 0;
    int total_sub = 0;
    int total_cons_load = 0;
    for(int i = 0; i < nclusters; i++){
        // std::string file_name = "../data/dataset/c" + std::to_string(i) + ".csv";
        std::string file_name = "./data/dataset/clustered_iot_devices.csv";
        std::ifstream fin;
        fin.open(file_name);
        // if(fin.is_open()) std::cout << "File open" << std::endl;
        // else std::cout << "File not open" << std::endl;
        std::string line;
        fin >> line;
        float lat, lon;
        int id;
        std::map<int, std::vector<float>> geolocs;
        std::vector<float> bottom_left(2, FLT_MAX), top_right(2, FLT_MIN);
        int n_items = n_producers + n_hosts + n_consumers;
        int seen = 0;
        while(fin >> line){
            std::vector<std::string> tokens;
		    tokenize(line, ",", tokens);
		
            id = std::stoi(tokens[0]);
            lat = std::stof(tokens[1]);
            lon = std::stof(tokens[2]);

            if(lat < bottom_left[0]) bottom_left[0] = lat;
		    if(lon < bottom_left[1]) bottom_left[1] = lon;
		    if(lat > top_right[0]) top_right[0] = lat;
		    if(lon > top_right[1]) top_right[1] = lon;
            
            geolocs[id] = {lat, lon};
            seen++;
            if(seen == n_items) break;
        }
        fin.close();

        ginfo.host_lb = bottom_left;
        ginfo.host_ub = top_right;
        float lat_diff = bottom_left[0] - top_right[0];
        float lon_diff = bottom_left[1] - top_right[1];
        float max_width = lat_diff*lat_diff + lon_diff*lon_diff;
        float per_lat_dist = max_width/3.0;
	    std::vector<float> radius = {per_lat_dist, 2 * per_lat_dist};
        std::vector<int> prod_ids;
        int sub_iter = 0;
        int inserted = 0;
        int capacity, load , n_prod_load, n_cons_load;
        int req_capacity;
        int n_subs;
        float score;
        for(auto elmt: geolocs){
            id = elmt.first;
            lat = elmt.second[0];
            lon = elmt.second[1];
            switch(entity){
                case 0:
                    capacity = pow(2, GenerateRandomInt(15, 20));
                    score = (float)(capacity - pow(2, 15)) / (float)(pow(2, 20) - pow(2, 15));
                    load = score * (MAXLOAD - MINLOAD) + MINLOAD;
                    // load = GenerateRandomInt(10, 20);
			        n_prod_load = load / 3;
			        n_cons_load = load - n_prod_load;
                    total_cons_load += n_cons_load;
                    // std::cout << "Host " << id << " pl = " << n_prod_load << " cl = " << n_cons_load << std::endl;
                    ginfo.hosts[id] = new Host(id, 0, lat, lon, capacity, n_prod_load, n_cons_load);
                    inserted++;
                    if(inserted == n_hosts){
                        inserted = 0;
                        entity = 1;
                    }
                    break;
                case 1:
                    req_capacity = pow(2, GenerateRandomInt(10, 15));
                    ginfo.producers[id] = new Producer(id, 1, lat, lon, req_capacity);
                    prod_ids.push_back(id);
                    for(auto host: ginfo.hosts){
                        float dist = displacement(ginfo.producers[id]->geo_loc_, host.second->geo_loc_);
                        ginfo.producers[id]->dist_hosts_.push_back({host.first, dist});
                        int total_load = host.second->consumer_load_threshold_ + host.second->producer_load_threshold_;
                        score = (float)(total_load - MINLOAD)/(float)(MAXLOAD - MINLOAD);
                        float latency;
                        if(score > 0.66)
                            latency = GenerateRandomFloat(7, 12);
                        else
                            latency = GenerateRandomFloat(13, 18);

                        ginfo.producers[id]->latency_hosts_map_[host.first] = latency;
                        ginfo.producers[id]->latency_hosts_.push_back({host.first, latency});
                    }
                    inserted++;
                    if(inserted == n_producers){
                        inserted = 0;
                        entity = 2;
                    }
                    break;
                case 2:
                    ginfo.consumers[id] = new Consumer(id, 2, lat, lon);
                    n_subs = GenerateRandomInt(1, 3);
                    total_sub += n_subs;
                    for(int i = 0; i < n_subs; i++){
                        ginfo.consumers[id]->subs_.push_back(prod_ids[sub_iter]);
                        sub_iter = ((sub_iter + 1) % n_producers);
                    }
                    for(auto host: ginfo.hosts){
                        float dist = displacement(ginfo.consumers[id]->geo_loc_, host.second->geo_loc_);
                        
                        int total_load = host.second->consumer_load_threshold_ + host.second->producer_load_threshold_;
                        score = (float)(total_load - MINLOAD)/(float)(MAXLOAD - MINLOAD);
                        float latency;
                        if(score > 0.66)
                            latency = GenerateRandomFloat(7, 12);
                        else
                            latency = GenerateRandomFloat(13, 18);

                        ginfo.consumers[id]->latency_hosts_map_[host.first] = latency;
                    }
                    inserted++;
                    if(inserted == n_consumers){
                        inserted = 0;
                        entity = -1;
                    }
                    break;
                default: std::cout << "Invalid case" << std::endl;
            }
            if(entity == -1) break;
        }

        for(auto hostval: ginfo.hosts){

            for(auto hostinfo: ginfo.hosts){
                if(hostval.first != hostinfo.first){
                    int total_load = hostinfo.second->consumer_load_threshold_ + hostinfo.second->producer_load_threshold_;
                    score = (float)(total_load - MINLOAD)/(float)(MAXLOAD - MINLOAD);
                    float latency;
                    if(score > 0.66)
                        latency = GenerateRandomFloat(7, 12);
                    else
                        latency = GenerateRandomFloat(13, 18);
                    hostval.second->host_latency_[hostinfo.first] = latency;
                }
            }   
        }
    }

    std::cout << total_cons_load << " " << total_sub << std::endl;
}

void runMatchMaker(){
    std::thread prod_queue_thd(&MatchMaker::processProdRequestQueue, ginfo.matchmaker_);
    std::thread cons_queue_thd(&MatchMaker::processConsRequestQueue, ginfo.matchmaker_);

    cons_queue_thd.join();
    prod_queue_thd.join();
}

void runHost(int id){
    Host *host = ginfo.hosts[id];
    if(ginfo.ptype == PlacementType::SPATIAL){
        ginfo.rtree_mutex.lock();
        RTreeNode *root = ginfo.rtree->GetRoot();
        RTreeNode *new_node = new RTreeNode(host);
        root->Insert(ginfo.rtree, new_node);
        ginfo.rtree_mutex.unlock();
    }

    ginfo.host_mutex.lock();
    ginfo.n_hosts++;
    ginfo.host_mutex.unlock();

    std::thread prod_queue_thd([id, host](){
        while(true){
            auto request = host->prod_queue_.dequeue();
            host->producer_data_[request->prod_id_] = request->chunk_id_;
        }
    });
    std::thread cons_queue_thd([host](){
        while(true){
            auto request = host->cons_queue_.dequeue();
            while(host->producer_data_.count(request->prod_id_) <= 0);
            while(host->producer_data_[request->prod_id_] < request->chunk_id_);
            ginfo.consumers[request->app_id_]->data_mutex_.lock();
            ginfo.consumers[request->app_id_]->data_from_subs_[request->prod_id_] = request->chunk_id_;
            ginfo.consumers[request->app_id_]->data_mutex_.unlock();
        }
    });

    std::thread rep_host_thd([host](){
        while(true){
            auto request = host->rep_host_queue_.dequeue();
            ginfo.hosts[request->host_id_]->producer_data_[request->prod_id_] = host->producer_data_[request->prod_id_];
        }
    });

    prod_queue_thd.join();
    cons_queue_thd.join();
    rep_host_thd.join();
}

void runProducer(int id){
    // std::cout << "Producer " << id << " starting..." << std::endl;
    Producer *producer = ginfo.producers[id];

    // Talk to MatchMaker
    Request *r = new Request(id, 0);
    auto start = std::chrono::steady_clock::now();
    ginfo.matchmaker_->prod_queue_.enqueue(r);
    while(ginfo.producers[id]->alloted_hosts_.size() == 0);
    auto end = std::chrono::steady_clock::now();
    // ginfo.log_mutex.lock();
    // ginfo.fout << "PET " <<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;
    // ginfo.log_mutex.unlock();
    ginfo.prods_mutex.lock();
    ginfo.n_prods++;
    ginfo.prods_mutex.unlock();

    // Talk to Hosts
    std::vector<std::thread> host_thds;
    auto alloted_hosts = producer->getAllotedHosts();
    for(auto host: alloted_hosts){
        if(host == -1) continue;
        host_thds.push_back(std::thread([id, host](){
            for(int i = 0; i < NCHUNKS; i++){
                ProducerRequest *pr = new ProducerRequest(id, i);
                ginfo.hosts[host]->prod_queue_.enqueue(pr);
                // ginfo.log_mutex.lock();
                // ginfo.fout << "P " << ginfo.producers[id]->latency_hosts_map_[host] + GenerateRandomFloat(-3.0, 3.0) << std::endl;
                // ginfo.log_mutex.unlock();
            }
        }));
    }
    for(auto &host_thd: host_thds)
        host_thd.join();

    // std::cout << "Producer " << id << " exiting..." << std::endl;
}

void runConsumer(int id){
    // std::cout << "Consumer " << id << std::endl;
    Consumer *consumer = ginfo.consumers[id];

    // Talk to Matchmaker
    Request *r = new Request(id, 1);
    auto start = std::chrono::steady_clock::now();
    ginfo.matchmaker_->cons_queue_.enqueue(r);
    while(true){
        ginfo.consumers[id]->sub_host_mutex_.lock();
        if(ginfo.consumers[id]->sub_host_.size() != ginfo.consumers[id]->subs_.size()){
            ginfo.consumers[id]->sub_host_mutex_.unlock();
            continue;
        }
        ginfo.consumers[id]->sub_host_mutex_.unlock();
        break;
    }
    // std::cout << "subs allocated " << id << std::endl;
    auto end = std::chrono::steady_clock::now();
    ginfo.log_mutex.lock();
    ginfo.fout << "CET " <<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;
    ginfo.log_mutex.unlock();
    // while(ginfo.consumers[id]->sub_host_.size() == 0);

    // Talk to hosts
    ginfo.consumers[id]->sub_host_mutex_.lock();
    auto sub_hosts = consumer->sub_host_;
    ginfo.consumers[id]->sub_host_mutex_.unlock();
    std::vector<std::thread> sub_thds;
    for(auto sub: sub_hosts){
        int prod = sub.first;
        int host = sub.second;
        if(host == -1){
            // ginfo.log_mutex.lock();
            // ginfo.fout << "CM " << id << " P " << prod  << std::endl;
            // ginfo.log_mutex.unlock();
            continue;
        }
        // std::cout << "Consumer " << id << " sending request to " << host << " for " << prod << std::endl;
        sub_thds.push_back(std::thread([id, prod, host](){

            for(int i = 0; i < NCHUNKS; i++){
                ConsumerRequest *cr = new ConsumerRequest(id, prod, i);
                ginfo.hosts[host]->cons_queue_.enqueue(cr);
                while(true){
                    ginfo.consumers[id]->data_mutex_.lock();
                    if(ginfo.consumers[id]->data_from_subs_[prod] == i){
                        ginfo.consumers[id]->data_mutex_.unlock();
                        break;
                    }
                    ginfo.consumers[id]->data_mutex_.unlock();
                }
                ginfo.log_mutex.lock();
                ginfo.fout << "C " << ginfo.consumers[id]->latency_hosts_map_[host] + ginfo.producers[prod]->latency_hosts_map_[host] + GenerateRandomFloat(-1.0, 1.0) << std::endl;
                ginfo.log_mutex.unlock();
            }
        }));
    }
    for(auto &sub_thd: sub_thds)
        sub_thd.join();
    
    // std::cout << "Consumer " << id << " exiting..." << std::endl;
}

int main(int argc, char *argv[]){

    ginfo.ptype = static_cast<PlacementType>(std::stoi(argv[1]));
    int n_hosts = std::stoi(argv[2]), n_prods = std::stoi(argv[3]), n_cons = std::stoi(argv[4]);
    ginfo.rtree = new RTree();
    ginfo.fout.open("timing.log");
    ginfo.n_prods = 0;
    ginfo.n_hosts = 0;
    ginfo.replica_threshold = 5;

    readClusterData(n_prods, n_hosts, n_cons);
    ginfo.matchmaker_ = new MatchMaker();
    ginfo.matchmaker_->ginfo_ = &ginfo;

    
    std::thread mm_thd(runMatchMaker);
    
    std::vector<std::thread> host_thds;
    for(auto &host: ginfo.hosts)
        host_thds.push_back(std::thread(runHost, host.second->id_));
    while(ginfo.n_hosts != n_hosts);

    std::vector<std::thread> prod_thds;
    for(auto &prod: ginfo.producers)
        prod_thds.push_back(std::thread(runProducer, prod.second->id_));

    while(true){
        ginfo.prods_mutex.lock();
        if(ginfo.n_prods != n_prods){
            ginfo.prods_mutex.unlock();
            break;
        }
        ginfo.prods_mutex.unlock();
    }
    srandom(10);
    std::vector<std::thread> cons_thds;
    for(auto &cons: ginfo.consumers){
        float sleep_time = -logf(1.0f - (float) random() / (RAND_MAX + 1.0)) / 10.0;
        std::chrono::milliseconds duration((int)std::ceil(sleep_time*1000));
        std::this_thread::sleep_for(duration);
        cons_thds.push_back(std::thread(runConsumer, cons.second->id_));
    }

    for(auto &cthd: cons_thds)
        cthd.join();

    for(auto &pthd: prod_thds)
        pthd.join();

    for(auto &hthd: host_thds)
        hthd.join();

    mm_thd.join();

    return 0;
}