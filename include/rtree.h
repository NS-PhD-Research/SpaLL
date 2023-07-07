#ifndef RTREE_H_
#define RTREE_H_

#include <iostream>
#include <list>
#include <climits>
#include <fstream>
#include <cfloat>
#include <algorithm>
#include <cmath>

#include "../include/node.h"

#define M 40
#define m 20
#define DIM 2 

typedef std::vector<float> GeoLoc;

class RTree;

struct BoundingBox {
	GeoLoc top_right_, bottom_left_;
};

class RTreeNode {
    public:
		static int instance_count;
        int                 n_children_;// Number of child nodes
        int         id_;        // Node ID
        bool                is_leaf_;   // denotes if node is leaf        
        BoundingBox*       bb_;        // Inner node bounding box
        std::list<RTreeNode*>   children_;  // Inner node children
        RTreeNode*               parent_;     // Parent node

		Host*               host_info_;


		RTreeNode(Host *host);
		~RTreeNode();

		void AdjustBB(GeoLoc &loc);
		void AdjustBB(BoundingBox &bb);
		BoundingBox* AdjustedBB(GeoLoc &loc);
		float ExpandedArea(BoundingBox* bb);
		float BoundedArea();
		float CombinedAreaLoc(GeoLoc &loc);
		float CombinedBoundingBoxArea(GeoLoc &loc);
		float CombinedBoundingBoxArea(BoundingBox* bb);
		float IncreasedArea(RTreeNode* entry);

		bool FullChildren();

		void RemoveChildEntry(RTreeNode* child, bool change_bb = true);
		RTreeNode* PickNext(RTreeNode* s_node);
		void PickSeeds(RTreeNode* new_entry, RTreeNode** first_seed, RTreeNode** second_seed);
		RTreeNode* SplitNode(RTreeNode* new_entry);
		void AdjustTree(RTree *rtree, RTreeNode* snode_1, RTreeNode* snode_2);
		void AddChild(RTree *rtree, RTreeNode* new_entry);
		void SplitAddChild(RTree *rtree, RTreeNode* new_entry);
		RTreeNode* ChooseLeaf(RTreeNode* new_entry);
		void Insert(RTree *rtree, RTreeNode* new_node);

		bool IsLocInRegion(GeoLoc &loc);
		void SearchNodes(GeoLoc &loc, std::map<int, std::vector<RTreeNode*>> &results);
		void SearchNodesMinArea(GeoLoc &loc, RTreeNode **min_area_node, float &min_area);
		// void SearchSingleNode(GeoLoc &loc, std::map<int, std::vector<RTreeNode*>> &results);
		void SearchSingleNode(GeoLoc &loc, std::vector<RTreeNode*> &results);

		float LatLonDisplacement(GeoLoc &loc1, GeoLoc &loc2);
		float GetMinDist(GeoLoc &loc);
		// void SearchNearestBB(GeoLoc &loc, float& min_dist, std::map<int, std::vector<RTreeNode*>> &results, float lb_dist);
		// void FindAllNearestBB(GeoLoc &loc, float& min_dist, std::map<int, std::vector<RTreeNode*>> &results, float lb_dist);
		// void FindAllNearestBB(GeoLoc &loc, float& min_dist, std::vector<RTreeNode*> &results, float lb_dist);
		void FindAllNearestBB(GeoLoc &loc, float min_dist, std::vector<RTreeNode*> &results, float lb_dist);
		
		// void SearchInConcentricCircles(GeoLoc &loc, float lb_dist, float ub_dist, std::map<int, std::vector<RTreeNode*>> &results);
		void SearchInConcentricCircles(GeoLoc &loc, float lb_dist, float ub_dist, std::vector<RTreeNode*> &results);
};

class RTree {
	private:
		RTreeNode *rtree_root_;
	public:
		RTree();
		~RTree();
		void SetRoot(RTreeNode* root) {rtree_root_ = root;}
		RTreeNode* GetRoot() { return rtree_root_;}
};

#endif
