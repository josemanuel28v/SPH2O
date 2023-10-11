#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <algorithm>

#include "Common/numeric_types.h"

struct Hash
{
    size_t operator()(const Vector3i& v) const
    {
        return v.x * 73856093 + v.y * 19349663 + v.z * 83492791;
    }
};

class HashTable
{
    public:

        struct pointInfo
        {
            uint id;
            uint pointSetId;
        };

        struct Bucket                                   
        {                                               
            std::vector<pointInfo> points;                     
            std::vector<pointInfo> neighbors;                    
            std::vector<pointInfo> boundary_points;            
            std::vector<pointInfo> boundary_neighbors;           
        };                                  
                                                        
        std::unordered_map<Vector3i, Bucket, Hash> table;       
        std::vector<Vector3i> keys;

        Real h;                                      

        HashTable() {}
        HashTable(const uint size) { reserve(size); }
        void clear();
        void clearB();
        void reserve(const uint size) { table.reserve(size); }
        uint size() { return static_cast<uint>(table.size()); }
        void setSmoothingLength(const Real h) { this->h = h; }
        void insertFluidParticle(const Vector3r& pos, const uint id, const uint pointSetId);
        void insertBoundaryParticle(const Vector3r& pos, const uint id, const uint pointSetId);
        void neighborhoodSearch();
};

#endif