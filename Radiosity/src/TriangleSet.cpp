/*
 * Copyright (C) 2007 TODO
 *
 * This file is part of rrv (Radiosity Renderer and Visualizer).
 *
 * rrv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * rrv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rrv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TriangleSet.h"
#include <stack>

/**
 * @param  triangle
 */
void TriangleSet::add(const Triangle &triangle )
{
	container_.push_back(triangle);
}

/**
 * @param  triangle
 */
void TriangleSet::add(const TriangleSet &tset)
{
	for(size_t i=0; i< tset.count(); i++) {
		this->add(tset.at(i));
	}
}

/**
 */
size_t TriangleSet::count() const
{
	return container_.size();
}

/**
 * @return TriangleSet*
 * @param  triangle
 */
boost::shared_ptr<TriangleSet> TriangleSet::divide(const Triangle &triangle, int &initializeCount)
{
	TriangleSet* afterDivide = new TriangleSet;

	// Compute triangle line's centres
	const Vertex *v = triangle.vertex;
	Vertex c[3];
	int cCount[3];
	for (int i=0; i<3; i++)
	{
		c[(i+2)%3] = centre(v[i], v[(i+1)%3]);
		cCount[(i+2)%3] = initializeCount++; 
	}

	// Add vertex triangles
	Triangle t = triangle;
	for (int i=0; i<3; i++) {
		t.vertex[0] = v[i];
		t.vertex[1] = c[(i+2)%3];
		t.vertex[2] = c[(i+1)%3];
		t.vertexIndex[0] = triangle.vertexIndex[i];
		t.vertexIndex[1] = cCount[(i+2)%3];
		t.vertexIndex[2] = cCount[(i+1)%3];
		afterDivide->add(t);
	}

	// Add middle triangle
	for (int i=0; i<3; i++)
	{
		t.vertex[i] = c[i];
		t.vertexIndex[i] = cCount[i];
	}

	afterDivide->add(t);

	return boost::shared_ptr<TriangleSet>(afterDivide);
}

boost::shared_ptr<TriangleSet> TriangleSet::divide(const TriangleSet &tSet, int &initializeCount)
{
	TriangleSet* afterDivide = new TriangleSet;

	for(int i = 0; i < tSet.count(); ++i) {
		afterDivide->add(*divide(tSet.at(i), initializeCount));
	}

	return boost::shared_ptr<TriangleSet>(afterDivide);
}

boost::shared_ptr<TriangleSet> TriangleSet::divideBySize(const TriangleSet &tSet, float size)
{
	TriangleSet *afterDivide = new TriangleSet;
	int initializeCount = tSet.getTotalVertexCount();
	for(int i = 0; i < tSet.count(); ++i){
		recuriveDivideBySize(*afterDivide, tSet.at(i), size, initializeCount);
	}
	afterDivide->setTotalVertexCount(initializeCount);
	return boost::shared_ptr<TriangleSet>(afterDivide);
}

void TriangleSet::recuriveDivideBySize(TriangleSet &afterDivide, const Triangle &t, float size, 
	int &initializeCount)
{
	if(TriangleSet::size<1>(t) <= size) {
		afterDivide.add(t);
		return;
	}

	boost::shared_ptr<TriangleSet> temp = divide(t, initializeCount);
	for(int i = 0; i < temp->count(); ++i){
		recuriveDivideBySize(afterDivide, temp->at(i), size, initializeCount);
	}
}