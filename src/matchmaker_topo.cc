#include "../include/matchmaker.h"
#include "../include/rtree.h"
void MatchMaker::processProdRequestQueue(){
    std::vector<std::thread> prod_thds;
    for(int i = 0; i < 10; i++){
        prod_thds.push_back(std::thread([this]{
            while(true){
                auto request = prod_queue_.dequeue();
                // std::cout << "Select host for producer " << request->id_ << std::endl;
                int host_id = selectHostForProducer(request->id_, false, -1);
                // std::cout << "----Selected host for producer " << request->id_ << " " << host_id << std::endl;
            }
        }));
        
    }
    for(auto &thd: prod_thds)
        thd.join();
}

void MatchMaker::processConsRequestQueue(){
    std::vector<std::thread> cons_thds;
    for(int i = 0; i < 10; i++){
        cons_thds.push_back(std::thread([this]{
            while(true){
                auto request = cons_queue_.dequeue();
                selectHostForConsumer(request->id_);
            }
        }));
    
    }

    for(auto &thd: cons_thds)
        thd.join();
}

bool sortByMetric(const std::pair<int,float> &p1,
              const std::pair<int,float> &p2){
    return (p1.second < p2.second);
}

float latLonDisplacement(std::vector<float> &loc1, std::vector<float> &loc2) {
    float x = (loc1[0] - loc2[0]) * (loc1[0] - loc2[0]);
	float y = (loc1[1] - loc2[1]) * (loc1[1] - loc2[1]);

	return x + y;
}

int MatchMaker::selectHostForProducer(int prod_id, bool replica, int cons_id){
    Producer *producer = ginfo_->producers[prod_id];
    int selected_host = -1;
    if(ginfo_->ptype == PlacementType::DIST){
        std::vector<std::pair<int, float>> dist_hosts;
        std::vector<float> centroid = {producer->geo_loc_[0], producer->geo_loc_[1]};
        if(replica){
            producer->sub_mutex_.lock();
            auto subscribers = producer->sub_consumers_;
            producer->sub_mutex_.unlock();
            for(auto sub: subscribers){
                centroid[0] += ginfo_->consumers[sub]->geo_loc_[0];
                centroid[1] += ginfo_->consumers[sub]->geo_loc_[1];
            }
            centroid[0] /= (float)(subscribers.size() + 1);
            centroid[1] /= (float)(subscribers.size() + 1);
            for(auto host: ginfo_->hosts)
                dist_hosts.push_back({host.first, latLonDisplacement(host.second->geo_loc_, centroid)});
        } else {
            producer->dls_mutex_.lock();
            for(auto dh: producer->dist_hosts_)
                dist_hosts.push_back(dh);
            producer->dls_mutex_.unlock();
        }

        bool found_host = false;
        std::sort(dist_hosts.begin(), dist_hosts.end(), sortByMetric);
        for(auto hd: dist_hosts){
            Host *host = ginfo_->hosts[hd.first];
            // std::cout << prod_id << " check host " << host->id_ << std::endl;
            bool producer_lock = true;
            host->producer_load_mutex_.lock();
            bool consumer_lock = false;
            // std::cout << prod_id << " entered CS " << std::endl;
            if(host->producer_load_ + 1 <= host->producer_load_threshold_){
                if(replica){
                    consumer_lock =true;
                    host->consumer_load_mutex_.lock();
                } else{
                    if(host->producer_load_ + 1 >= host->producer_load_threshold_/2){
                        host->producer_load_mutex_.unlock();
                        continue;
                    }
                }
                bool storage_lock = true;
                host->storage_mutex_.lock();
                if(host->remaining_storage_capacity_ >= producer->total_transfer_size_){
                    if(replica){
                        // std::cout << "R " << prod_id << "<=>" << host->id_ <<" => " << host->producer_load_ + 1 << " --- " << host->producer_load_threshold_  << " ======== " << host->consumer_load_ + 1 << " --- " << host->consumer_load_threshold_ <<std::endl;
                        if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                            host->consumer_load_++;
                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();
                        } else{
                            storage_lock = false;
                            host->storage_mutex_.unlock();

                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();

                            producer_lock = false;
                            host->producer_load_mutex_.unlock();
                            continue;
                        }
                    } else{
                        // std::cout <<"NR " << prod_id << "<=>" << host->id_ <<" => " << host->producer_load_ + 1 << " --- " << host->producer_load_threshold_ <<std::endl;
                    }
                    // std::cout << prod_id << " updating storage capacity " << std::endl;
                    host->remaining_storage_capacity_ -= producer->total_transfer_size_;
                    host->producer_load_++;
                    host->storage_mutex_.unlock();
                    storage_lock = false;
                    host->producer_load_mutex_.unlock();
                    producer_lock = false;
                    producer->host_mutex_.lock();
                    auto already_alloted_hosts = producer->alloted_hosts_;
                    producer->alloted_hosts_.push_back(hd.first);
                    producer->host_mutex_.unlock();
                    // std::cout << prod_id << " selected " << hd.first << std::endl;
                    if(replica){
                        std::vector<std::pair<float, int>> host_distances;
                        for(auto &h: already_alloted_hosts)
                            host_distances.push_back({latLonDisplacement(ginfo_->hosts[h]->geo_loc_, ginfo_->hosts[hd.first]->geo_loc_), h});
                        std::sort(host_distances.begin(), host_distances.end());

                        ginfo_->hosts[host_distances[0].second]->prod_path_mutex_.lock();
                        float curr_latency = ginfo_->hosts[host_distances[0].second]->prod_path_latency_[prod_id];
                        ginfo_->hosts[host_distances[0].second]->prod_path_mutex_.unlock();
                        
                        float new_latency = curr_latency + ginfo_->hosts[host_distances[0].second]->host_latency_[hd.first];
                        ginfo_->hosts[hd.first]->prod_path_mutex_.lock();
                        ginfo_->hosts[hd.first]->prod_path_latency_[prod_id] =  new_latency;
                        ginfo_->hosts[hd.first]->prod_path_mutex_.unlock();

                        float p2c_latency = new_latency;

                        RepHostRequest *rhr = new RepHostRequest(hd.first, prod_id);
                        ginfo_->hosts[hd.first]->producer_load_mutex_.lock();
                        ginfo_->hosts[hd.first]->producer_load_++;
                        ginfo_->hosts[hd.first]->producer_load_mutex_.unlock();
                        ginfo_->hosts[host_distances[0].second]->rep_host_queue_.enqueue(rhr);

                        while(ginfo_->hosts[hd.first]->producer_data_.count(prod_id) <= 0);
                        ginfo_->hosts[host_distances[0].second]->consumer_load_mutex_.lock();
                        ginfo_->hosts[host_distances[0].second]->consumer_load_++;
                        ginfo_->hosts[host_distances[0].second]->consumer_load_mutex_.unlock();

                        int total_load = ginfo_->hosts[hd.first]->consumer_load_threshold_ + ginfo_->hosts[hd.first]->producer_load_threshold_;
                        float score = (float)(total_load - 20)/30.0;
                        float latency;
                        if(score > 0.66){
                            latency = generateRandomFloat(5, 10);
                        } else if(score > 0.33){
                            latency = generateRandomFloat(10, 15);
                        } else{
                            latency = generateRandomFloat(15, 20);
                        }
                        p2c_latency += latency;

                        ginfo_->log_mutex.lock();
                        ginfo_->fout << "P2C " << p2c_latency << std::endl;
                        ginfo_->fout << "TL " << latency + (generateRandomFloat(5.0, 10.0) * 2.0) << std::endl;
                        ginfo_->log_mutex.unlock();
                        // std::cout << prod_id  << " completed request to new host" << std::endl;
                    } else {
                        ginfo_->hosts[hd.first]->prod_path_mutex_.lock();
                        ginfo_->hosts[hd.first]->prod_path_latency_[prod_id] = producer->latency_hosts_map_[hd.first];
                        ginfo_->hosts[hd.first]->prod_path_mutex_.unlock();

                        int total_load = ginfo_->hosts[hd.first]->consumer_load_threshold_ + ginfo_->hosts[hd.first]->producer_load_threshold_;
                        float score = (float)(total_load - 20)/30.0;
                        float latency;
                        if(score > 0.66){
                            latency = generateRandomFloat(5, 10);
                        } else if(score > 0.33){
                            latency = generateRandomFloat(10, 15);
                        } else{
                            latency = generateRandomFloat(15, 20);
                        }
                        float p2c_latency = latency + producer->latency_hosts_map_[hd.first];

                        ginfo_->log_mutex.lock();
                        ginfo_->fout << "P2C " << p2c_latency << std::endl;
                        ginfo_->log_mutex.unlock();

                    }
                    found_host = true;
                    selected_host = hd.first;
                }
                if(storage_lock)
                    host->storage_mutex_.unlock();
                if(replica && consumer_lock){
                    host->consumer_load_mutex_.unlock();
                }
            }
            if(producer_lock)
                host->producer_load_mutex_.unlock();
            if(found_host) break;
            // else{
            //     std::cout << prod_id  << " cannot find new host" << std::endl;
            // }
        }
    } else if(ginfo_->ptype == PlacementType::LAT){
        producer->dls_mutex_.lock();
        std::vector<std::pair<int, float>> latency_hosts;
        for(auto lh: producer->latency_hosts_)
            latency_hosts.push_back(lh);
        latency_hosts = producer->latency_hosts_;
        producer->dls_mutex_.unlock();
        std::sort(latency_hosts.begin(), latency_hosts.end(), sortByMetric);
        
        bool found_host = false;
        for(auto hd: latency_hosts){
            Host *host = ginfo_->hosts[hd.first];

            bool producer_lock = true;
            host->producer_load_mutex_.lock();
            bool consumer_lock = false;

            if(host->producer_load_ + 1 <= host->producer_load_threshold_){
                if(replica){
                    consumer_lock = true;
                    host->consumer_load_mutex_.lock();
                } else{
                    if(host->producer_load_ + 1 >= host->producer_load_threshold_/2){
                        producer_lock = false;
                        host->producer_load_mutex_.unlock();
                        continue;
                    }
                }

                bool storage_lock = true;
                host->storage_mutex_.lock();
                if(host->remaining_storage_capacity_ >= producer->total_transfer_size_){
                    if(replica){
                        if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                            host->consumer_load_++;
                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();
                        } else{
                            storage_lock = false;
                            host->storage_mutex_.unlock();

                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();

                            producer_lock = false;
                            host->producer_load_mutex_.unlock();
                            continue;
                        }
                    }
                    host->remaining_storage_capacity_ -= producer->total_transfer_size_;
                    host->producer_load_++;
                    host->storage_mutex_.unlock();
                    storage_lock = false;
                    host->producer_load_mutex_.unlock();
                    producer_lock = false;

                    producer->host_mutex_.lock();
                    auto already_alloted_hosts = producer->alloted_hosts_;
                    producer->alloted_hosts_.push_back(hd.first);
                    producer->host_mutex_.unlock();
                    
                    if(replica){
                        std::vector<std::pair<float, int>> host_latencies;
                        float max_latency = -1;
                        for(auto &h: already_alloted_hosts){
                            int total_load = ginfo_->hosts[h]->consumer_load_threshold_ + ginfo_->hosts[h]->producer_load_threshold_;
                            float score = (float)(total_load - 20)/30.0;
                            float latency;
                            if(score > 0.66){
                                latency = generateRandomFloat(5, 10);
                            } else if(score > 0.33){
                                latency = generateRandomFloat(10, 15);
                            } else{
                                latency = generateRandomFloat(15, 20);
                            }
                            host_latencies.push_back({latency, h});
                            max_latency = std::fmax(latency, max_latency);
                        }
                        std::sort(host_latencies.begin(), host_latencies.end());
                        max_latency += 5 + 5; 
                        RepHostRequest *rhr = new RepHostRequest(hd.first, prod_id);
                        ginfo_->hosts[hd.first]->producer_load_mutex_.lock();
                        ginfo_->hosts[hd.first]->producer_load_++;
                        ginfo_->hosts[hd.first]->producer_load_mutex_.unlock();
                        ginfo_->hosts[host_latencies[0].second]->rep_host_queue_.enqueue(rhr);

                        while(ginfo_->hosts[hd.first]->producer_data_.count(prod_id) <= 0);
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.lock();
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_++;
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.unlock();

                        int total_load = ginfo_->hosts[hd.first]->consumer_load_threshold_ + ginfo_->hosts[hd.first]->producer_load_threshold_;
                        float score = (float)(total_load - 20)/30.0;
                        float latency;
                        if(score > 0.66){
                            latency = generateRandomFloat(5, 10);
                        } else if(score > 0.33){
                            latency = generateRandomFloat(10, 15);
                        } else{
                            latency = generateRandomFloat(15, 20);
                        }
                        ginfo_->log_mutex.lock();
                        ginfo_->fout << "TL " << latency + (generateRandomFloat(5.0, 10.0) * 2.0) + max_latency << std::endl;
                        ginfo_->log_mutex.unlock();
                    }
                    found_host = true;
                    selected_host = hd.first;
                }
                if(storage_lock)
                    host->storage_mutex_.unlock();
                if(replica && consumer_lock)
                    host->consumer_load_mutex_.unlock();
            }
            if(producer_lock)
                host->producer_load_mutex_.unlock();
            if(found_host) break;
        }
    } else{
        auto loc = producer->geo_loc_;
        std::vector<RTreeNode*> all_nearest_BB;
        ginfo_->rtree_mutex.lock();
        RTreeNode *root = ginfo_->rtree->GetRoot();
        root->SearchSingleNode(loc, all_nearest_BB);
        
        float min_dist = INT_MAX;
        if(all_nearest_BB.size() == 0)
            root->FindAllNearestBB(loc, min_dist, all_nearest_BB, 0);
        // else
            // std::cout << "ANB empty" << std::endl;

        std::vector<std::pair<float, Host*>> lat_hosts;
        for(auto rnode = all_nearest_BB.begin(); rnode != all_nearest_BB.end(); ++rnode) {
            RTreeNode *mbr = *(rnode);
            for(auto node = mbr->children_.begin(); node != mbr->children_.end(); node++){
                Host *host = (*node)->host_info_;
                lat_hosts.push_back({producer->latency_hosts_map_[host->id_], host});
            }
        }

        ginfo_->rtree_mutex.unlock();
        // std::cout << "Number of hosts ANB = " << lat_hosts.size() << std::endl;
        std::sort(lat_hosts.begin(), lat_hosts.end());            
        
        bool found_host = false;
        for(auto node = lat_hosts.begin(); node != lat_hosts.end(); node++){
            Host *host = node->second;

            bool producer_lock = true;
            host->producer_load_mutex_.lock();
            bool consumer_lock = false;

            if(host->producer_load_ + 1 <= host->producer_load_threshold_){
                if(replica){
                    consumer_lock = true;
                    host->consumer_load_mutex_.lock();
                } else{
                    if(host->producer_load_ + 1 >= host->producer_load_threshold_/2){
                        producer_lock = false;
                        host->producer_load_mutex_.unlock();
                        continue;
                    }
                }

                bool storage_lock = true;
                host->storage_mutex_.lock();
                if(host->remaining_storage_capacity_ >= producer->total_transfer_size_){
                    if(replica){
                        if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                            host->consumer_load_++;
                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();
                        } else{
                            storage_lock = false;
                            host->storage_mutex_.unlock();

                            consumer_lock = false;
                            host->consumer_load_mutex_.unlock();

                            producer_lock = false;
                            host->producer_load_mutex_.unlock();
                            continue;
                        }
                    }
                    host->remaining_storage_capacity_ -= producer->total_transfer_size_;
                    host->producer_load_++;
                    host->storage_mutex_.unlock();
                    storage_lock = false;
                    host->producer_load_mutex_.unlock();
                    producer_lock = false;

                    producer->host_mutex_.lock();
                    auto already_alloted_hosts = producer->alloted_hosts_;
                    producer->alloted_hosts_.push_back(host->id_);
                    producer->host_mutex_.unlock();
                    
                    if(replica){
                        std::vector<std::pair<float, int>> host_latencies;
                        float max_latency = -1;
                        for(auto &h: already_alloted_hosts){
                            int total_load = ginfo_->hosts[h]->consumer_load_threshold_ + ginfo_->hosts[h]->producer_load_threshold_;
                            float score = (float)(total_load - 20)/30.0;
                            float latency;
                            if(score > 0.66){
                                latency = generateRandomFloat(5, 10);
                            } else if(score > 0.33){
                                latency = generateRandomFloat(10, 15);
                            } else{
                                latency = generateRandomFloat(15, 20);
                            }
                            host_latencies.push_back({latency, h});
                            max_latency = std::fmax(latency, max_latency);
                        }
                        std::sort(host_latencies.begin(), host_latencies.end());
                        max_latency += 5 + 5;

                        RepHostRequest *rhr = new RepHostRequest(host->id_, prod_id);
                        ginfo_->hosts[host->id_]->producer_load_mutex_.lock();
                        ginfo_->hosts[host->id_]->producer_load_++;
                        ginfo_->hosts[host->id_]->producer_load_mutex_.unlock();
                        ginfo_->hosts[host_latencies[0].second]->rep_host_queue_.enqueue(rhr);

                        while(ginfo_->hosts[host->id_]->producer_data_.count(prod_id) <= 0);
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.lock();
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_++;
                        ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.unlock();

                        int total_load = ginfo_->hosts[host->id_]->consumer_load_threshold_ + ginfo_->hosts[host->id_]->producer_load_threshold_;
                        float score = (float)(total_load - 20)/30.0;
                        float latency;
                        if(score > 0.66){
                            latency = generateRandomFloat(5, 10);
                        } else if(score > 0.33){
                            latency = generateRandomFloat(10, 15);
                        } else{
                            latency = generateRandomFloat(15, 20);
                        }
                        ginfo_->log_mutex.lock();
                        ginfo_->fout << "TL " << latency + (generateRandomFloat(5.0, 10.0) * 2.0) + max_latency << std::endl;
                        ginfo_->log_mutex.unlock();
                    }
                    found_host = true;
                    selected_host = host->id_;;
                }
                if(storage_lock)
                    host->storage_mutex_.unlock();
                if(replica && consumer_lock)
                    host->consumer_load_mutex_.unlock();
            }
            if(producer_lock)
                host->producer_load_mutex_.unlock();
            if(found_host) break;
        }

        // Concentric Search
        if(!found_host){
            float dist_bl = latLonDisplacement(loc, ginfo_->host_lb);
            float dist_tr = latLonDisplacement(loc, ginfo_->host_ub);

            float max_dist = std::fmax(dist_bl, dist_tr);
            float per_epoch_enlarge_dist = max_dist / 3.0;

            int count = 0;
            float lb_dist = 0;
            float ub_dist = per_epoch_enlarge_dist;

            while(!found_host){
                ginfo_->rtree_mutex.lock();
                std::vector<RTreeNode*> all_concentric_BB;

                root->SearchInConcentricCircles(loc, lb_dist, ub_dist, all_concentric_BB);

                std::vector<std::pair<float, Host*>> lat_hosts;
                for(auto rnode = all_concentric_BB.begin(); rnode != all_concentric_BB.end(); ++rnode) {
                    RTreeNode *mbr = *(rnode);
                    for(auto node = mbr->children_.begin(); node != mbr->children_.end(); node++){
                        Host *host = (*node)->host_info_;
                        lat_hosts.push_back({producer->latency_hosts_map_[host->id_], host});
                    }
                }
                ginfo_->rtree_mutex.unlock();

                std::sort(lat_hosts.begin(), lat_hosts.end());            


                for(auto node = lat_hosts.begin(); node != lat_hosts.end(); node++){
                    Host *host = node->second;

                    bool producer_lock = true;
                    host->producer_load_mutex_.lock();
                    bool consumer_lock = false;

                    if(host->producer_load_ + 1 <= host->producer_load_threshold_){
                        if(replica){
                            consumer_lock = true;
                            host->consumer_load_mutex_.lock();
                        }  else{
                            if(host->producer_load_ + 1 >= host->producer_load_threshold_/2){
                                producer_lock = false;
                                host->producer_load_mutex_.unlock();
                                continue;
                            }
                        }

                        bool storage_lock = true;
                        host->storage_mutex_.lock();
                        if(host->remaining_storage_capacity_ >= producer->total_transfer_size_){
                            if(replica){
                                if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                                    host->consumer_load_++;
                                    consumer_lock = false;
                                    host->consumer_load_mutex_.unlock();
                                } else{
                                    storage_lock = false;
                                    host->storage_mutex_.unlock();

                                    consumer_lock = false;
                                    host->consumer_load_mutex_.unlock();

                                    producer_lock = false;
                                    host->producer_load_mutex_.unlock();
                                    continue;
                                }
                            }
                            host->remaining_storage_capacity_ -= producer->total_transfer_size_;
                            host->producer_load_++;
                            host->storage_mutex_.unlock();
                            storage_lock = false;
                            host->producer_load_mutex_.unlock();
                            producer_lock = false;

                            producer->host_mutex_.lock();
                            auto already_alloted_hosts = producer->alloted_hosts_;
                            producer->alloted_hosts_.push_back(host->id_);
                            producer->host_mutex_.unlock();

                            if(replica){
                                std::vector<std::pair<float, int>> host_latencies;
                                float max_latency = -1;
                                for(auto &h: already_alloted_hosts){
                                    int total_load = ginfo_->hosts[h]->consumer_load_threshold_ + ginfo_->hosts[h]->producer_load_threshold_;
                                    float score = (float)(total_load - 20)/30.0;
                                    float latency;
                                    if(score > 0.66){
                                        latency = generateRandomFloat(5, 10);
                                    } else if(score > 0.33){
                                        latency = generateRandomFloat(10, 15);
                                    } else{
                                        latency = generateRandomFloat(15, 20);
                                    }
                                    host_latencies.push_back({latency, h});
                                    max_latency = std::fmax(max_latency, latency);
                                }
                                std::sort(host_latencies.begin(), host_latencies.end());
                                max_latency += 5 + 5;

                                RepHostRequest *rhr = new RepHostRequest(host->id_, prod_id);
                                ginfo_->hosts[host->id_]->producer_load_mutex_.lock();
                                ginfo_->hosts[host->id_]->producer_load_++;
                                ginfo_->hosts[host->id_]->producer_load_mutex_.unlock();
                                ginfo_->hosts[host_latencies[0].second]->rep_host_queue_.enqueue(rhr);

                                while(ginfo_->hosts[host->id_]->producer_data_.count(prod_id) <= 0);
                                ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.lock();
                                ginfo_->hosts[host_latencies[0].second]->consumer_load_++;
                                ginfo_->hosts[host_latencies[0].second]->consumer_load_mutex_.unlock();

                                int total_load = ginfo_->hosts[host->id_]->consumer_load_threshold_ + ginfo_->hosts[host->id_]->producer_load_threshold_;
                                float score = (float)(total_load - 20)/30.0;
                                float latency;
                                if(score > 0.66){
                                    latency = generateRandomFloat(5, 10);
                                } else if(score > 0.33){
                                    latency = generateRandomFloat(10, 15);
                                } else{
                                    latency = generateRandomFloat(15, 20);
                                }
                                ginfo_->log_mutex.lock();
                                ginfo_->fout << "TL " << latency + (generateRandomFloat(5.0, 10.0) * 2.0) + max_latency << std::endl;
                                ginfo_->log_mutex.unlock();
                            }
                            found_host = true;
                            selected_host = host->id_;
                        }
                        if(storage_lock)
                            host->storage_mutex_.unlock();
                        if(replica && consumer_lock)
                            host->consumer_load_mutex_.unlock();
                    }
                    if(producer_lock)
                        host->producer_load_mutex_.unlock();
                    if(found_host) break;
                }

                lb_dist = ub_dist;
                if(max_dist - (ub_dist + per_epoch_enlarge_dist) < per_epoch_enlarge_dist)
                    ub_dist = FLT_MAX;
                else
                    ub_dist = ub_dist + per_epoch_enlarge_dist;
                if(lb_dist > max_dist){
                    // lb_dist = 0;
                    // ub_dist = per_epoch_enlarge_dist;
                    break;
                }
            }

        }
    }
    if(selected_host == -1){
        // std::cout << "NH" << std::endl;
        producer->host_mutex_.lock();
        producer->alloted_hosts_.push_back(selected_host);
        producer->host_mutex_.unlock();
        if(replica){
            ginfo_->log_mutex.lock();
            ginfo_->fout << "SHPR" << std::endl;
            ginfo_->log_mutex.unlock();
        } else{
            ginfo_->log_mutex.lock();
            ginfo_->fout << "SHP" << std::endl;
            ginfo_->log_mutex.unlock();
        }
    }
    return selected_host;
}

void MatchMaker::selectHostForConsumer(int cons_id){
    // std::cout << cons_id << " consumer selecting host" << std::endl;
    Consumer *consumer = ginfo_->consumers[cons_id];
    int selected_host = -1;
    bool found_host = false;
    if(ginfo_->ptype == PlacementType::DIST){
        auto subs = consumer -> subs_;
        for(auto sub: subs){
            while(true){
                ginfo_->producers[sub]->host_mutex_.lock();
                if(ginfo_->producers[sub]->alloted_hosts_.size() != 0){
                    ginfo_->producers[sub]->host_mutex_.unlock();
                    break;    
                }
                ginfo_->producers[sub]->host_mutex_.unlock();
            }
            
            ginfo_->producers[sub]->host_mutex_.lock();
            auto alloted_hosts = ginfo_->producers[sub]->alloted_hosts_;
            ginfo_->producers[sub]->host_mutex_.unlock();
            std::vector<std::pair<int, float>> dists;
            for(auto host_id: alloted_hosts){
                if(host_id == -1) continue;
                dists.push_back({latLonDisplacement(ginfo_->hosts[host_id]->geo_loc_, consumer->geo_loc_), host_id});
            }
            std::sort(dists.begin(), dists.end());
            // std::cout << cons_id << " checking existing list of hosts of " << sub << " of size " << alloted_hosts.size() << std::endl;
            for(auto hd: dists){
                // std::cout << cons_id << "checking ordered dist" << std::endl;
                 Host *host = ginfo_->hosts[hd.second];
                host->consumer_load_mutex_.lock();
                found_host = false;
                // std::cout <<cons_id << " -> " << sub << "<=>" << host->id_ <<" => " << host->producer_load_ << " --- " << host->producer_load_threshold_  << " **** " << host->consumer_load_ << " --- " << host->consumer_load_threshold_ <<std::endl;
                if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                    host->consumer_load_++;
                    consumer->sub_host_mutex_.lock();
                    consumer->sub_host_[sub] = hd.second;
                    selected_host = hd.second;
                    consumer->sub_host_mutex_.unlock();
                    found_host = true;
                }
                host->consumer_load_mutex_.unlock();
                if(found_host) break;
            }
            if(!found_host){
                auto start = std::chrono::steady_clock::now();
                selected_host = selectReplicaForConsumer(cons_id, sub);
                auto end = std::chrono::steady_clock::now();
                ginfo_->log_mutex.lock();
                int t = (int) std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                ginfo_->log_mutex.unlock();
                if(selected_host != -1){
                    int total_load = ginfo_->hosts[selected_host]->consumer_load_threshold_ + ginfo_->hosts[selected_host]->producer_load_threshold_;
                    float score = (float)(total_load - 20)/30.0;
                    float latency;
                    if(score > 0.66){
                        latency = generateRandomFloat(5, 10);
                    } else if(score > 0.33){
                        latency = generateRandomFloat(10, 15);
                    } else{
                        latency = generateRandomFloat(15, 20);
                    }
                    ginfo_->log_mutex.lock();
                    // ginfo_->fout << "R " << ((float)t/1000.0) + ginfo_->consumers[cons_id]->latency_hosts_map_[selected_host] + generateRandomFloat(-3.0, 3.0) << std::endl;
                    ginfo_->fout << "R " << ((float)t/1000.0) + ginfo_->consumers[cons_id]->latency_hosts_map_[selected_host] + (generateRandomFloat(5.0, 10.0) * 2.0) << std::endl;
                    ginfo_->log_mutex.unlock();
                } else {
                    ginfo_->log_mutex.lock();
                    ginfo_->fout << "SHC" << std::endl;
                    ginfo_->fout << "R " << ((float)t/1000.0) + (generateRandomFloat(5.0, 10.0) * 2.0) << std::endl;
                    ginfo_->log_mutex.unlock();
                }
            }

        }
    } else{
        auto subs = consumer -> subs_;
        for(auto sub: subs){
            while(true){
                ginfo_->producers[sub]->host_mutex_.lock();
                if(ginfo_->producers[sub]->alloted_hosts_.size() != 0){
                    ginfo_->producers[sub]->host_mutex_.unlock();
                    break;
                }
                ginfo_->producers[sub]->host_mutex_.unlock();
                // std::cout << "Prod " << sub << " has no host for Cons " << cons_id << std::endl;
            }
            ginfo_->producers[sub]->host_mutex_.lock();
            auto alloted_hosts = ginfo_->producers[sub]->alloted_hosts_;
            ginfo_->producers[sub]->host_mutex_.unlock();
            std::vector<std::pair<int, float>> latency;
            for(auto host_id: alloted_hosts){
                if(host_id == -1) continue;
                // float curr_latency = consumer->latency_hosts_map_[host_id] + generateRandomFloat(-3.0, 3.0);
                float curr_latency = consumer->latency_hosts_map_[host_id];
                latency.push_back({curr_latency, host_id});
            }
            // std::cout << "Hosts under consideration for consumer " << cons_id << " = " << latency.size() << std::endl;
            std::sort(latency.begin(), latency.end());

            for(auto hl: latency){
                // std::cout << hl.second << std::endl;
                Host *host = ginfo_->hosts[hl.second];
                host->consumer_load_mutex_.lock();
                found_host = false;
                if(host->consumer_load_ + 1 <= host->consumer_load_threshold_){
                    host->consumer_load_++;
                    consumer->sub_host_mutex_.lock();
                    consumer->sub_host_[sub] = hl.second;
                    selected_host = hl.second;
                    consumer->sub_host_mutex_.unlock();
                    found_host = true;
                }
                host->consumer_load_mutex_.unlock();
                if(found_host) break;
            }

            if(!found_host){
                // std::cout << cons_id << " going for replica service" << std::endl;
                auto start = std::chrono::steady_clock::now();
                selected_host = selectReplicaForConsumer(cons_id, sub);
                auto end = std::chrono::steady_clock::now();
                ginfo_->log_mutex.lock();
                int t = (int) std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                ginfo_->log_mutex.unlock();
                if(selected_host != -1){
                    int total_load = ginfo_->hosts[selected_host]->consumer_load_threshold_ + ginfo_->hosts[selected_host]->producer_load_threshold_;
                    float score = (float)(total_load - 20)/30.0;
                    float latency;
                    if(score > 0.66){
                        latency = generateRandomFloat(5, 10);
                    } else if(score > 0.33){
                        latency = generateRandomFloat(10, 15);
                    } else{
                        latency = generateRandomFloat(15, 20);
                    }
                    ginfo_->log_mutex.lock();
                    // ginfo_->fout << "R " << ((float)t/1000.0) + ginfo_->consumers[cons_id]->latency_hosts_map_[selected_host] + generateRandomFloat(-3.0, 3.0) << std::endl;
                    ginfo_->fout << "R " << ((float)t/1000.0) + ginfo_->consumers[cons_id]->latency_hosts_map_[selected_host] + (generateRandomFloat(5.0, 10.0) * 2.0) << std::endl;
                    ginfo_->log_mutex.unlock();
                } else {
                    ginfo_->log_mutex.lock();
                    ginfo_->fout << "SHC" << std::endl;
                    ginfo_->fout << "R " << ((float)t/1000.0) + (generateRandomFloat(5.0, 10.0) * 2.0) << std::endl;
                    ginfo_->log_mutex.unlock();
                }
            }
        }
    }
    // std::cout << "----Selected host for consumer " << cons_id << " " << selected_host << std::endl;
}

int MatchMaker::selectReplicaForConsumer(int cons_id, int prod_id){
    // std::cout << "REP " << cons_id << "::" << prod_id << std::endl;
    Producer *producer = ginfo_->producers[prod_id];
    Consumer *consumer = ginfo_->consumers[cons_id];
    int selected_host = selectHostForProducer(prod_id, true, cons_id);
    consumer->sub_host_mutex_.lock();
    consumer->sub_host_[prod_id] = selected_host;
    // std::cout << prod_id << "--" << selected_host << std::endl;
    consumer->sub_host_mutex_.unlock();
    return selected_host;
    // std::cout << "REP " << cons_id << " ==> " << prod_id << std::endl; 
}