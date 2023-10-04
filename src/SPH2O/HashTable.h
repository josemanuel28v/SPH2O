#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <algorithm>

#include "../Common/numeric_types.h"

using std::vector;
using std::unordered_map;

struct Hash
{
    size_t operator()(Vector3i v) const
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

        struct Bucket                                   // Estructura que contiene las particulas y los vecinos de cada celda. Es el value de la pareja {key, value}
        {                                               
            vector<pointInfo> points;                     
            vector<pointInfo> neighbors;                    
            vector<pointInfo> boundary_points;            
            vector<pointInfo> boundary_neighbors;           
        };                                  
                                                        
        unordered_map<Vector3i, Bucket, Hash> table;       // Tabla hash que utiliza como clave una celda y como valor el bucket (que contiene las particulas y los vecinos de esa celda) 
        vector<Vector3i> keys;

        Real h;                                       // SupportRadius

        HashTable() {}
        HashTable(unsigned size)
        {
            reserve(size);
        }

        void insertFluidParticle(Vector3r pos, uint id, uint pointSetId)
        {
            Vector3i key = floor(pos / h);

            pointInfo value = {id, pointSetId};
            //uint value = id;

            table[key].points.push_back(value);
        }

        void insertBoundaryParticle(Vector3r pos, uint id, uint pointSetId)
        {
            Vector3i key = floor(pos / h);

            pointInfo value = {id, pointSetId};
            //uint value = id;

            table[key].boundary_points.push_back(value);
        }
        
        void neighborhoodSearch()
        {
            #ifndef _OPENMP
            for (auto& cell: table)
                for (int x = - 1; x <= 1; x++)
                    for (int y = - 1; y <= 1; y++)
                        for (int z = - 1; z <= 1; z++)
                        {
                            Vector3i dir(x, y, z);
                            Vector3i neigh_key = cell.first + dir;
                            auto neigh_cell = table.find(neigh_key);
                            
                            if (neigh_cell != table.end())
                            {
                                cell.second.neighbors.insert(cell.second.neighbors.end(), 
                                                          neigh_cell->second.points.begin(), 
                                                          neigh_cell->second.points.end()); 

                                cell.second.boundary_neighbors.insert(cell.second.boundary_neighbors.end(),       // De esta manera se consigue tener las boundary particles que son vecinas de una celda en un vector distinto de neighbors
                                                          neigh_cell->second.boundary_points.begin(), 
                                                          neigh_cell->second.boundary_points.end()); 
                            }
                        }
            #else
            // Asi mejora respecto a arriba pero es necesario utilizar como tabla hash un array donde cada elemento sea la celda en la que esta cada particula
            vector<Vector3i> keys;
            for (auto& cell: table)
            {
                keys.push_back(cell.first);
            }

            #pragma omp parallel for
            for (int i = 0; i < keys.size(); ++i)
            {
                Vector3i key = keys[i];
                auto& cell = table[key];
                for (int x = -1; x <= 1; x++)
                {
                    for (int y = -1; y <= 1; y++)
                    {
                        for (int z = -1; z <= 1; z++)
                        {
                            Vector3i dir(x, y, z);
                            Vector3i neigh_key = key + dir;
                            auto neigh_cell = table.find(neigh_key);

                            if (neigh_cell != table.end())
                            {
                                cell.neighbors.insert(cell.neighbors.end(),
                                    neigh_cell->second.points.begin(),
                                    neigh_cell->second.points.end());

                                // No se puede hacer una vez al principio aunque sean estaticas porque al principio solo se buscaran
                                // vecinos para las celdas en las que haya boundary particles ya que aun no hay particulas de fluido
                                cell.boundary_neighbors.insert(cell.boundary_neighbors.end(),       // De esta manera se consigue tener las boundary particles que son vecinas de una celda en un vector distinto de neighbors
                                    neigh_cell->second.boundary_points.begin(),
                                    neigh_cell->second.boundary_points.end());
                            }
                        }
                    }
                }
            }
            #endif
        }

        void clear()
        {
            for (auto& cell: table)
            {
                cell.second.points.clear();
                cell.second.neighbors.clear();
                cell.second.boundary_neighbors.clear();
            }
        }

        void clearB()
        {
            for (auto& cell: table)
            {
                cell.second.boundary_points.clear();
            }
        }

        void reserve(size_t size)
        {
            table.reserve(size);
        }

        uint size()
        {
            return static_cast<uint>(table.size());
        }

        void setSmoothingLength(Real h)
        {
            this->h = h;
        }
};

#endif