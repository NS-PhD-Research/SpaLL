#include "../include/rtree.h"

int RTreeNode::instance_count = 0;

RTreeNode::RTreeNode(Host *host){
    is_leaf_ = true;
    bb_ = NULL;
    parent_ = NULL;
    id_ = instance_count++;
    host_info_ = host;
    n_children_ = 0;
}

RTreeNode::~RTreeNode(){

}

void RTreeNode::AdjustBB(GeoLoc &loc){
	// for first entry into the leaf node
    if (bb_ == NULL) {
        bb_ = new BoundingBox;
        for (int i = 0; i < DIM; i++) {
            bb_->top_right_.push_back(loc[i]);
            bb_->bottom_left_.push_back(loc[i]);
        } 
    } else { // for all entries after first one
        
        for (int i = 0; i < DIM; i++) {
            bb_->top_right_[i] = std::fmax(loc[i], bb_->top_right_[i]);
            bb_->bottom_left_[i] = std::fmin(loc[i], bb_->bottom_left_[i]);
        }
    }
}

void RTreeNode::AdjustBB(BoundingBox& bb) {
    if (bb_ == NULL) {
        bb_ = new BoundingBox();
        for (int i = 0; i < DIM; i++) {
            bb_->top_right_.push_back(bb.top_right_[i]);
            bb_->bottom_left_.push_back(bb.bottom_left_[i]);
        } 
    } else {
        for (int i = 0; i < DIM; i++) {
            bb_->top_right_[i] = std::fmax(bb.top_right_[i], bb_->top_right_[i]);
            bb_->bottom_left_[i] = std::fmin(bb.bottom_left_[i], bb_->bottom_left_[i]);
        }
    }
}

BoundingBox* RTreeNode::AdjustedBB(GeoLoc &loc){
    BoundingBox* bb = new BoundingBox();
    for (int i = 0; i < DIM; i++) {
        bb->top_right_.push_back(std::fmax(loc[i], bb_->top_right_[i]));
        bb->bottom_left_.push_back(std::fmin(loc[i], bb_->bottom_left_[i]));
    }

    return bb;
}

float RTreeNode::ExpandedArea(BoundingBox* bb) {
    float current_area = 1.0, expanded_area = 1.0;
    for (int i = 0; i < DIM; i++) {
        current_area *= fabs(bb_->top_right_[i] - bb_->bottom_left_[i]);
    }
    for (int i = 0; i < DIM; i++) {
        expanded_area *= fabs(bb->top_right_[i] - bb->bottom_left_[i]);
    }

    return fabs(current_area - expanded_area);
}

float RTreeNode::BoundedArea() {
    float area = 1;
    for (int i = 0; i < DIM; i++) {
        area *= fabs(bb_->top_right_[i] - bb_->bottom_left_[i]);
    }

    return area;
}

float RTreeNode::CombinedAreaLoc(GeoLoc &loc){
    BoundingBox* bb = new BoundingBox();
    
    GeoLoc *own_loc = &(host_info_->geo_loc_);
    for (int i = 0; i < DIM; i++) {
        bb->top_right_.push_back(std::fmax(loc[i], (*own_loc)[i]));
        bb->bottom_left_.push_back(std::fmin(loc[i], (*own_loc)[i]));
    }

    float area = 1;
    for (int i = 0; i < DIM; i++) {
        area *= fabs(bb->top_right_[i] - bb->bottom_left_[i]);
    }

    delete bb;

    return area;
}

float RTreeNode::CombinedBoundingBoxArea(GeoLoc &loc){
    BoundingBox *temp_bb = new BoundingBox;
    for (int i = 0; i < DIM; i++) {
        temp_bb->top_right_.push_back(std::fmax(loc[i], bb_->top_right_[i]));
        temp_bb->bottom_left_.push_back(std::fmin(loc[i], bb_->bottom_left_[i]));
    }

    float area = 1;
    for (int i = 0; i < DIM; i++) {
        area *= fabs(temp_bb->top_right_[i] - temp_bb->bottom_left_[i]);
    }

    delete temp_bb;
    
    return area;
}

float RTreeNode::CombinedBoundingBoxArea(BoundingBox* bb){
    BoundingBox *temp_bb = new BoundingBox();
    for (int i = 0; i < DIM; i++) {
        temp_bb->top_right_.push_back(std::fmax(bb->top_right_[i], bb_->top_right_[i]));
        temp_bb->bottom_left_.push_back(std::fmin(bb->bottom_left_[i], bb_->bottom_left_[i]));
    }

    float area = 1;
    for (int i = 0; i < DIM; i++) {
        area *= fabs(temp_bb->top_right_[i] - temp_bb->bottom_left_[i]);
    }

    delete temp_bb;
    
    return area;
}

float RTreeNode::IncreasedArea(RTreeNode* entry) {
    // Node is a leaf child => bb_ = NULL
    BoundingBox* temp_bb = new BoundingBox();

    if(entry->bb_ == NULL){
        GeoLoc loc = entry->host_info_->geo_loc_;
        for (int i = 0; i < DIM; i++) {
            if(bb_ == NULL){
                temp_bb->top_right_.push_back(std::fmax(loc[i], host_info_->geo_loc_[i]));
                temp_bb->bottom_left_.push_back(std::fmin(loc[i], host_info_->geo_loc_[i]));
            } else {
                temp_bb->top_right_.push_back(std::fmax(loc[i], bb_->top_right_[i]));
                temp_bb->bottom_left_.push_back(std::fmin(loc[i], bb_->bottom_left_[i]));
            }
            
        }
    } else {
        for (int i = 0; i < DIM; i++) {
            if(bb_ == NULL){
                temp_bb->top_right_.push_back(std::fmax(entry->bb_->top_right_[i], host_info_->geo_loc_[i]));
                temp_bb->bottom_left_.push_back(std::fmin(entry->bb_->bottom_left_[i], host_info_->geo_loc_[i]));
            } else {
                temp_bb->top_right_.push_back(std::fmax(entry->bb_->top_right_[i], bb_->top_right_[i]));
                temp_bb->bottom_left_.push_back(std::fmin(entry->bb_->bottom_left_[i], bb_->bottom_left_[i]));
            }
        }
    }

    float area_increase = ExpandedArea(temp_bb);

    delete temp_bb;

    return area_increase;
}


bool RTreeNode::FullChildren(){
	if(children_.size() < M) return false;
	return true;
}

void RTreeNode::RemoveChildEntry(RTreeNode* child, bool change_bb) {
    for(auto it=children_.begin(); it != children_.end(); ++it) {
        RTreeNode *curr_child = *it;
        if(curr_child == child){
            children_.remove(*it);
            break;
        }
    }

    if(!change_bb) return;

    // After removing the entry adjust the bounding box of parent to minimum size
    bool first_entry = true;
    for(auto it=children_.begin(); it != children_.end(); ++it) {
        RTreeNode *curr_child = *it;
        if(first_entry){
            for (int i = 0; i < DIM; i++) {
                if(curr_child->bb_ != NULL){
                    bb_->top_right_[i] = curr_child->bb_->top_right_[i];
                    bb_->bottom_left_[i] = curr_child->bb_->bottom_left_[i];
                } else {
                    GeoLoc loc = curr_child->host_info_->geo_loc_;
                    bb_->top_right_[i] = loc[i];
                    bb_->bottom_left_[i] = loc[i];
                }
            }
            first_entry = false;
        } else {
            for (int i = 0; i < DIM; i++) {
                if(curr_child->bb_ != NULL){
                    bb_->top_right_[i] = std::fmax(curr_child->bb_->top_right_[i], bb_->top_right_[i]);
                    bb_->bottom_left_[i] = std::fmin(curr_child->bb_->bottom_left_[i], bb_->bottom_left_[i]);
                } else {
                    GeoLoc loc = curr_child->host_info_->geo_loc_;
                    bb_->top_right_[i] = std::fmax(loc[i], bb_->top_right_[i]);
                    bb_->bottom_left_[i] = std::fmin(loc[i], bb_->bottom_left_[i]);
                }
            }
        }
    }
}

void RTreeNode::AdjustTree(RTree *rtree, RTreeNode* snode_1, RTreeNode* snode_2){
	// Propagates only bounding box when there is no split
    if(snode_2 == NULL) {
        AdjustBB(*(snode_1->bb_));
        if(parent_ != NULL) {
            parent_->AdjustTree(rtree, this, NULL);
        }
        return;
    }

    // Propagate changes upwards on a node split
    RemoveChildEntry(snode_1); // this entry is already in parent, so remove it and then add back
    children_.push_back(snode_1);
    AdjustBB(*(snode_1->bb_));
    snode_1->parent_ = this;
    if(parent_ != NULL)
        parent_->AdjustTree(rtree, this, NULL);

    if(n_children_ < M) {
        children_.push_back(snode_2);
        AdjustBB(*(snode_2->bb_));
        snode_2->parent_ = this;
        n_children_++;
        if(parent_ != NULL)
            parent_->AdjustTree(rtree, this, NULL);
    } else {
        SplitAddChild(rtree, snode_2);
    }
}

void RTreeNode::AddChild(RTree *rtree, RTreeNode* new_entry){
	new_entry->parent_ = this;
    children_.push_back(new_entry);
	GeoLoc geoloc = new_entry->host_info_->geo_loc_; 
    AdjustBB(geoloc);
    n_children_++;

	if(parent_ == NULL)
        return;
	
    // Propagate changes upwards
    parent_->AdjustTree(rtree, this, NULL);
}

void RTreeNode::PickSeeds(RTreeNode* new_entry, RTreeNode** first_seed, RTreeNode** second_seed) {
    float area_diff = -1.0;
    children_.push_back(new_entry);
    n_children_++;
    for(auto it1=children_.begin(); it1 != children_.end(); ++it1) {
        RTreeNode *child1 = *it1;
        float temp_area1 = 0.0;
        if(child1->bb_ != NULL)
            temp_area1 = child1->BoundedArea();
        for(auto it2 = std::next(it1); it2 != children_.end(); ++it2) {
            RTreeNode* child2 = *it2;
            float temp_area2 = 0.0;
            if(child2->bb_ != NULL) 
                temp_area2 = child2->BoundedArea();
            float enclosed_area = 0.0;
            if(child1->bb_ == NULL && child2->bb_ == NULL)
                enclosed_area = child1->CombinedAreaLoc(child2->host_info_->geo_loc_);
            else if(child1->bb_== NULL)
                enclosed_area = child2->CombinedBoundingBoxArea(child1->host_info_->geo_loc_);
            else if(child2->bb_== NULL)
                enclosed_area = child1->CombinedBoundingBoxArea(child2->host_info_->geo_loc_);
            else
                enclosed_area = child1->CombinedBoundingBoxArea(child2->bb_);
            float diff = fabs(enclosed_area - temp_area1 - temp_area2);
            if (diff > area_diff) {
                *first_seed = child1;
                *second_seed = child2;
                area_diff = diff;
            }
        }
    }
    RemoveChildEntry(*first_seed, false);
    n_children_--;
    RemoveChildEntry(*second_seed, false);
    n_children_--;
}

RTreeNode* RTreeNode::PickNext(RTreeNode* s_node) {
    // this: split node 1, s_node: split node 2
    RTreeNode* next_pick = NULL;
    float diff_of_diff_area = 0.0;
    bool selected = false;
    
    for(auto it = children_.begin(); it != children_.end(); it++) {
        if(children_.size() == 1){
            next_pick = *it;
            RemoveChildEntry(next_pick, false);
            return next_pick;
        }
        float d_1 = IncreasedArea(*it);
        float d_2 = s_node->IncreasedArea(*it);
        float d = fabs(d_1 - d_2);
        
        if(d > diff_of_diff_area || next_pick == NULL){
            next_pick = *it;
            diff_of_diff_area = d;
        }
    }

    RemoveChildEntry(next_pick, false);
    return next_pick;
}

RTreeNode* RTreeNode::SplitNode(RTreeNode* new_entry) {
    RTreeNode *first_seed = NULL, *second_seed = NULL;
    PickSeeds(new_entry, &first_seed, &second_seed);
    // Create node (current node) and s_node with initial entries first_seed and second seed respectively

    // node's temporary children placeholder
    std::list<RTreeNode*>*   t_children = new std::list<RTreeNode*>;
    // push first seed into temporary placeholder
    t_children->push_back(first_seed);
    first_seed->parent_ = this;
    n_children_ = 1;
    delete bb_;
    bb_ = NULL;
    if(first_seed->bb_ == NULL){
        AdjustBB(first_seed->host_info_->geo_loc_);
    } else {
        AdjustBB(*(first_seed->bb_));
    }

    // s_node
    RTreeNode *s_node  = new RTreeNode(NULL);
    s_node->children_.push_back(second_seed);
    second_seed->parent_ = s_node;
    s_node->n_children_ = 1;
    s_node->parent_ = parent_;
    if(second_seed->bb_ == NULL){
        s_node->AdjustBB(second_seed->host_info_->geo_loc_);
    } else {
        s_node->AdjustBB(*(second_seed->bb_));
    }
    
    // new entry added to the children list during pick seed => total #children remaining = M + 1 - 2
    int n_child_remain = M - 1;

    while(n_child_remain > 0) {
        // if ((n_children_ + n_child_remain == m) && (s_node->n_children_ >= m)){
        if (s_node->n_children_ > m){
            for(auto it = children_.begin(); it != children_.end(); ++it) {
                RTreeNode* child = *it;
                t_children->push_back(child);
                child->parent_ = this;
                if(child->bb_ != NULL)
                    AdjustBB(*(child->bb_));
                else
                    AdjustBB(child->host_info_->geo_loc_);
                n_children_++;
                n_child_remain--;
            }
            break;
        }
        // if ((s_node->n_children_ + n_child_remain == m) && (n_children_ >= m)){
        if (n_children_ > m){
            for(auto it = children_.begin(); it != children_.end(); ++it) {
                RTreeNode* child = *it;
                s_node->children_.push_back(child);
                child->parent_ = s_node;
                if(child->bb_ != NULL)
                    s_node->AdjustBB(*(child->bb_));
                else
                    s_node->AdjustBB(child->host_info_->geo_loc_);
                s_node -> n_children_++;
                n_child_remain--;
            }
            break;
        }

        RTreeNode* child = PickNext(s_node);
        
        float area1 = 0.0;
        if (child->bb_ != NULL)// internal node
            area1 = CombinedBoundingBoxArea(child->bb_);
        else // external node
            area1 = CombinedBoundingBoxArea(child->host_info_->geo_loc_);
        
        float area2 = 0.0;
        if (child->bb_ != NULL) // internal node
            area2 = s_node->CombinedBoundingBoxArea(child->bb_);
        else // external node
            area2 = s_node->CombinedBoundingBoxArea(child->host_info_->geo_loc_);
        
        if (area1 < area2){
            t_children->push_back(child);
            if(child->bb_ != NULL){
                AdjustBB(*(child->bb_));
            }
            else{
                AdjustBB(child->host_info_->geo_loc_);
            }
            child->parent_ = this;
            n_children_++;
        }
        else if (area2 < area1){
            s_node->children_.push_back(child);
            if(child->bb_ != NULL)
                s_node->AdjustBB(*(child->bb_));
            else{
                s_node->AdjustBB(child->host_info_->geo_loc_);
            }
            child->parent_ = s_node;
            s_node->n_children_++;
        } else {
            float ownarea1 = BoundedArea();
            float ownarea2 = s_node->BoundedArea();
            if (ownarea1 < ownarea2){
                children_.push_back(child);
                if(child->bb_ != NULL)
                    AdjustBB(*(child->bb_));
                else
                    AdjustBB(child->host_info_->geo_loc_);
                child->parent_ = this;
                n_children_++;
            }else {
                s_node->children_.push_back(child);
                if(child->bb_ != NULL)
                    s_node->AdjustBB(*(child->bb_));
                else
                    s_node->AdjustBB(child->host_info_->geo_loc_);;
                child->parent_ = s_node;
                s_node->n_children_++;
            }
        }
        n_child_remain--;
    }

    children_.clear();
    std::copy(t_children->begin(), t_children->end(), std::back_inserter(children_));

    return s_node;
}

void RTreeNode::SplitAddChild(RTree *rtree, RTreeNode* new_entry) {
    // Split node and accomodate new entry
    RTreeNode* s_node = SplitNode(new_entry);

    if (parent_ == NULL){ // root
        RTreeNode *new_root = new RTreeNode(NULL);
        parent_ = new_root;
        s_node->parent_ = new_root;
        new_root->parent_ = NULL;
        new_root->children_.push_back(this);
        new_root->children_.push_back(s_node);
        new_root->AdjustBB(*bb_);
        new_root->AdjustBB(*(s_node->bb_));
        new_root->n_children_ = 2;
        rtree->SetRoot(new_root);
        return;
    }
    s_node->parent_ = parent_;

    // Propogate changes upwards
    parent_->AdjustTree(rtree, this, s_node);
}

RTreeNode* RTreeNode::ChooseLeaf(RTreeNode* new_entry){
    // If leaf entry return the node
    if (bb_ == NULL) {
        return this;
    }

    // Internal node children checked to identify the right node to traverse
    RTreeNode* min_exp_child = NULL;
    BoundingBox* min_bb = NULL;
    float min_area_exp = 0.0;
    
    for(auto it=children_.begin(); it != children_.end(); ++it) {
        RTreeNode* child = *it;
        if(child->bb_==NULL)
            continue;
        // First node is taken as the pivot for comparison
        if (min_exp_child == NULL) {
            min_exp_child = child;
            min_bb = child -> AdjustedBB(new_entry->host_info_->geo_loc_);
            min_area_exp = child -> ExpandedArea(min_bb);
        } else {
            BoundingBox* temp_min_bb = child -> AdjustedBB(new_entry->host_info_->geo_loc_);
            float temp_min_area_exp = child -> ExpandedArea(temp_min_bb);
            // select node with minimum area expansion
            if (temp_min_area_exp < min_area_exp) {
                min_exp_child = child;
                delete min_bb;
                min_bb = NULL;
                min_bb = temp_min_bb;
                min_area_exp = temp_min_area_exp;
            }
        }
    }
    // all children of current node are locations
    if(min_exp_child == NULL)
        return this;

    delete min_exp_child -> bb_;
    min_exp_child -> bb_ = min_bb;

    // traverse down the selected node to find the leaf entry
    return min_exp_child -> ChooseLeaf(new_entry);
}

void RTreeNode::Insert(RTree *rtree, RTreeNode* new_entry){
    RTreeNode* leaf_node = ChooseLeaf(new_entry);
    
	if(!leaf_node->FullChildren()) {
		leaf_node->AddChild(rtree, new_entry);
	} else {
		leaf_node->SplitAddChild(rtree, new_entry);
	}
}

bool RTreeNode::IsLocInRegion(GeoLoc &loc) {
    bool in_region = true;
    for(int i = 0; i < DIM; i++){
        in_region = in_region && (
                            (loc[i] >= bb_->bottom_left_[i]) &&
                            (loc[i] <= bb_->top_right_[i])
                            );
    }
    return in_region;
}

void RTreeNode::SearchNodes(GeoLoc &loc, std::map<int, std::vector<RTreeNode*>> &results) {
    if(children_.size() != 0){
        if(IsLocInRegion(loc)){
            for(auto child_it = children_.begin(); child_it != children_.end(); ++child_it){
                RTreeNode *child = *child_it;
                if(child->children_.size() == 0){
                    results[id_].push_back(child);
                } else {
                    child->SearchNodes(loc, results);
                }
            }
        } else {
            return;
        }
    }
}

void RTreeNode::SearchNodesMinArea(GeoLoc &loc, RTreeNode **min_area_node, float &min_area) {
    if(children_.size() != 0){
        if(IsLocInRegion(loc)){
            RTreeNode *child = children_.front();
            if(child->children_.size() == 0){
                float area = BoundedArea();
                if(area < min_area){
                    min_area = area;
                    *min_area_node = this;
                }
            } else{
                for(auto child_it = children_.begin(); child_it != children_.end(); ++child_it){
                    RTreeNode *child = *child_it;
                    child->SearchNodesMinArea(loc, min_area_node, min_area);
                }
            }
        } else {
            return;
        }
    }
}

// void RTreeNode::SearchSingleNode(GeoLoc &loc, std::map<int, std::vector<RTreeNode*>> &results){
//     RTreeNode *min_area_node  = NULL;
//     float min_area = FLT_MAX;
//     SearchNodesMinArea(loc, &min_area_node, min_area);
//     if(min_area_node != NULL){
//         int id = min_area_node->id_;
//         for(auto child_it = min_area_node->children_.begin(); child_it != min_area_node->children_.end(); ++child_it){
//             RTreeNode *child = *child_it;
//             results[id].push_back(child);
//         }
//     } else {
//         std::cout << "Min area node is empty" << std::endl;
//     }
// }

void RTreeNode::SearchSingleNode(GeoLoc &loc, std::vector<RTreeNode*> &results){
    RTreeNode *min_area_node  = NULL;
    float min_area = FLT_MAX;
    SearchNodesMinArea(loc, &min_area_node, min_area);
    if(min_area_node != NULL){
        results.push_back(min_area_node);
    } else {
        // std::cout << "Min area node is empty" << std::endl;
    }
}

float RTreeNode::LatLonDisplacement(GeoLoc &loc1, GeoLoc &loc2) {
    float x = (loc1[0] - loc2[0]) * (loc1[0] - loc2[0]);
	float y = (loc1[1] - loc2[1]) * (loc1[1] - loc2[1]);

	return x + y;
    // float lat1 = loc1[0] * PI / 180;
    // float lat2 = loc2[0] * PI / 180;

    // float delta_lat = (loc2[0] - loc1[0]) * PI / 180;
    // float delta_lon = (loc2[1] - loc1[1]) * PI / 180;

    // float const1 =  sin(delta_lat/2) * sin(delta_lat/2) +
    //                 cos(lat1) * cos(lat2) *
    //                 sin(delta_lon/2) * sin(delta_lon/2);
    
    // float const2 = 2 * atan2(sqrt(const1), sqrt(1-const1));

    // float lat_lon_disp = R * const2;

    // return lat_lon_disp;
}

float MinDist(float x, float y, float x1, float y1, float x2, float y2){
    float denominator = sqrt(pow(y1-y2, 2) + pow(x2-x1, 2));
    float numerator = (y1 - y2) * x + (x2 - x1) * y + (y2 - y1) * x1 - (x2 - x1) * y1;
    return std::fabs(numerator/denominator);
}

float RTreeNode::GetMinDist(GeoLoc &loc){
    GeoLoc top_left, bottom_right;
    top_left.push_back(bb_->bottom_left_[0]);
    top_left.push_back(bb_->top_right_[1]);
    bottom_right.push_back(bb_->top_right_[0]);
    bottom_right.push_back(bb_->bottom_left_[1]);

    //bottom left and top left
    float mindist_s1 = MinDist(loc[0], loc[1], bb_->bottom_left_[0], bb_->bottom_left_[1], top_left[0], top_left[1]);
    // top left and top right
    float mindist_s2 = MinDist(loc[0], loc[1], top_left[0], top_left[1], bb_->top_right_[0], bb_->top_right_[1]);
    // top right and bottom right
    float mindist_s3 = MinDist(loc[0], loc[1], bb_->top_right_[0], bb_->top_right_[1], bottom_right[0], bottom_right[1]);
    // bottom right and bottom left
    float mindist_s4 = MinDist(loc[0], loc[1], bottom_right[0], bottom_right[1], bb_->bottom_left_[0], bb_->bottom_left_[1]);

    float min_dist;;
    min_dist = std::min(std::min(std::min(mindist_s1, mindist_s2), mindist_s3), mindist_s4);

    return min_dist;
}

// void RTreeNode::FindAllNearestBB(GeoLoc &loc, float& min_dist, std::map<int, std::vector<RTreeNode*>> &results, float lb_dist){

//     if(children_.size() != 0){
//         RTreeNode *min_dist_node = NULL;
//         if(IsLocInRegion(loc)){
//             RTreeNode *child = children_.front();
//             if(child->children_.size() == 0){
//                 // for(auto it = children_.begin(); it != children_.end(); ++it) {
//                 //     RTreeNode* child = *it;
//                 //     if(child -> children_.size() == 0)
//                 //         results[id_].push_back(child);                
//                 // }
//                 results[id_].push_back(this);
//             } else{
//                 for(auto it = children_.begin(); it != children_.end(); ++it) {
//                     RTreeNode* child = *it;
//                     float dist = child->GetMinDist(loc);
//                     if(dist > lb_dist && dist < min_dist){
//                         min_dist = dist;
//                         min_dist_node = child;
//                     }
//                 }
//             }
//         } else{
//             RTreeNode *child = children_.front();
//             if(child->children_.size() == 0){
//                 results[id_].push_back(this);
//             }
//             for(auto it = children_.begin(); it != children_.end(); ++it) {
//                 RTreeNode* child = *it;
//                 // if(child -> children_.size() == 0){
//                 //     results[id_].push_back(child);
//                 // }else{
//                     float dist = child->GetMinDist(loc);
//                     if(dist > lb_dist && dist < min_dist){
//                         min_dist = dist;
//                         min_dist_node = child;
//                     }
//                 // }
//             }
//         }
        
//         if(min_dist_node != NULL){
//             min_dist_node->FindAllNearestBB(loc, min_dist, results, lb_dist);
//         }
//     }
// }

void RTreeNode::FindAllNearestBB(GeoLoc &loc, float min_dist, std::vector<RTreeNode*> &results, float lb_dist){

    if(children_.size() != 0){
        RTreeNode *min_dist_node = NULL;
        if(IsLocInRegion(loc)){
            RTreeNode *child = children_.front();
            if(child->children_.size() == 0){
                results.push_back(this);
            } else{
                for(auto it = children_.begin(); it != children_.end(); ++it) {
                    RTreeNode* child = *it;
                    float dist = child->GetMinDist(loc);
                    if(dist > lb_dist && dist < min_dist){
                        min_dist = dist;
                        min_dist_node = child;
                    }
                }
            }
        } else{
            RTreeNode *child = children_.front();
            if(child->children_.size() == 0){
                results.push_back(this);
            } else {
                for(auto it = children_.begin(); it != children_.end(); ++it) {
                    RTreeNode* child = *it;
                    float dist = child->GetMinDist(loc);
                    if(dist > lb_dist && dist < min_dist){
                        min_dist = dist;
                        min_dist_node = child;
                    }
                }
            }
        }
        
        if(min_dist_node != nullptr){
            min_dist_node->FindAllNearestBB(loc, min_dist, results, lb_dist);
        }
    }
}

// void RTreeNode::SearchInConcentricCircles(GeoLoc &loc, float lb_dist, float ub_dist, std::map<int, std::vector<RTreeNode*>> &results){

//     if(children_.size() != 0){
//         for(auto it = children_.begin(); it != children_.end(); ++it) {
//             RTreeNode* child = *it;
//             if(child -> children_.size() == 0){
//                 float dist = LatLonDisplacement(loc, child->host_info_->geo_loc_);
//                 if(dist > lb_dist && dist < ub_dist)
//                     results[id_].push_back(child);
//             }
//             else{
//                 float dist = child->GetMinDist(loc);
//                 if((dist > lb_dist && dist < ub_dist) || IsLocInRegion(loc)){
//                     child->SearchInConcentricCircles(loc, lb_dist, ub_dist, results);
//                 }
//             }
//         }
//     }
// }

void RTreeNode::SearchInConcentricCircles(GeoLoc &loc, float lb_dist, float ub_dist, std::vector<RTreeNode*> &results){

    if(children_.size() != 0){
        for(auto it = children_.begin(); it != children_.end(); ++it) {
            RTreeNode* child = *it;
            if(child -> children_.size() == 0){
                results.push_back(this);
                return;
            }
            else{
                float dist = child->GetMinDist(loc);
                if((dist > lb_dist && dist < ub_dist) || IsLocInRegion(loc)){
                    child->SearchInConcentricCircles(loc, lb_dist, ub_dist, results);
                }
            }
        }
    }
}

RTree::RTree(){
    rtree_root_ = new RTreeNode(NULL);
}

RTree::~RTree(){
    
}