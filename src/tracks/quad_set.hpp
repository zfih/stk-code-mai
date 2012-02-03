//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_QUAD_SET_HPP
#define HEADER_QUAD_SET_HPP

#include <vector>
#include <string>

#include "tracks/quad.hpp"
#include "utils/vec3.hpp"

class XMLNode;

/**
  * \ingroup tracks
  */
class QuadSet
{
private:
    /** The 2d bounding box, used for hashing. */
    Vec3                m_min;
    Vec3                m_max;
    /** The list of all quads. */
    std::vector<Quad*>  m_all_quads;
    void load    (const std::string &filename);
    void getPoint(const XMLNode *xml, const std::string &attribute_name, 
                  Vec3 *result) const;

public:
    static const int QUAD_NONE=-1;

                 QuadSet       (const std::string& filename);
                ~QuadSet       ();
    void         reverse_all_quads();
    int          getCurrentQuad(const Vec3& p, int oldQuad)        const;
    const Quad&  getQuad(int n) const {return *(m_all_quads[n]); }

    /** Return the minimum and maximum coordinates of this quad set. */
    void         getBoundingBox(Vec3 *min, Vec3 *max) 
                               { *min=m_min; *max=m_max;   }
    /** Returns the number of quads. */
    unsigned int getNumberOfQuads() const 
                                {return (unsigned int)m_all_quads.size(); }
    /** Returns the center of quad n. */
    const Vec3&  getCenterOfQuad(int n) const 
                                {return m_all_quads[n]->getCenter();      }
    /** Returns the n-th. quad. */
    const Quad&  getQuad(int n)  {return *(m_all_quads[n]);                }
};   // QuadSet
#endif
