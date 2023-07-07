#include "../include/common.h"

#define NCHUNKS 100

class Node;
class Producer;
class Consumer;
class Host;

class Node {
    public:
        int id_;
        int type_;
        std::vector<float> geo_loc_;
        Node(int id, int type, float lat, float lon){
            id_ = id;
            type_ = type;
            geo_loc_ = {lat, lon};
        }
};

class Producer: public Node {
        public:
        int total_transfer_size_;
        int upload_speed_;
        std::mutex host_mutex_;
        std::vector<int> alloted_hosts_;
        std::mutex sub_mutex_;
        std::vector<int> sub_consumers_;

        std::mutex dls_mutex_;
        std::map<int, float> latency_hosts_map_;
        std::vector<std::pair<int, float>> dist_hosts_;
        std::vector<std::pair<int, float>> latency_hosts_;
        
        std::vector<int> getAllotedHosts(){
            std::lock_guard<std::mutex> lock(host_mutex_);
            return alloted_hosts_;
        }

        Producer(int id, int type, float lat, float lon, int capacity): Node(id, type, lat, lon){
            total_transfer_size_ = capacity;
        }

};

struct ProducerRequest {
    int prod_id_;
    int chunk_id_;
    ProducerRequest(int prod_id, int chunk_id): prod_id_(prod_id), chunk_id_(chunk_id){}
};

struct ConsumerRequest {
    int app_id_;
    int prod_id_;
    int chunk_id_;
    ConsumerRequest(int app_id, int prod_id, int chunk_id): app_id_(app_id), prod_id_(prod_id), chunk_id_(chunk_id){}
};

struct RepHostRequest {
    int host_id_;
    int prod_id_;
    RepHostRequest(int host_id, int prod_id): host_id_(host_id), prod_id_(prod_id){}
};

class Host: public Node {
    public:
        std::mutex storage_mutex_;
        int storage_capacity_;
        int remaining_storage_capacity_;

        std::mutex producer_load_mutex_;
        int producer_load_threshold_;
        int producer_load_;
        std::map<int, int> producer_data_;

        std::mutex consumer_load_mutex_;
        int consumer_load_threshold_;
        int consumer_load_;

        std::map<int, float> host_latency_;

        std::mutex prod_path_mutex_;
        std::map<int, float> prod_path_latency_;

        RequestQueue<ConsumerRequest> cons_queue_;
        RequestQueue<ProducerRequest> prod_queue_;
        RequestQueue<RepHostRequest> rep_host_queue_;

        Host(int id, int type, float lat, float lon, int capacity, int prod_load, int cons_load): Node(id, type, lat, lon){
            storage_capacity_ = capacity;
            remaining_storage_capacity_ = capacity;
            producer_load_threshold_ = prod_load;
            producer_load_ = 0;
            consumer_load_threshold_ = cons_load;
            consumer_load_ = 0;
        }
};

class Consumer: public Node {
    public:
        std::vector<int> subs_;
        std::mutex sub_host_mutex_;
        std::map<int, int> sub_host_;
        std::mutex data_mutex_;
        std::map<int, int> data_from_subs_;
        std::map<int, float> latency_hosts_map_;

        Consumer(int id, int type, float lat, float lon): Node(id, type, lat, lon){}

        std::map<int, int> getSubHosts() {return sub_host_;}
        void setData(int sub_id, int chunk_id) {data_from_subs_[sub_id] = chunk_id; }
        int getChunkId(int sub_id) { return data_from_subs_[sub_id];}
    
};