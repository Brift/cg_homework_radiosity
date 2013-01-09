#ifndef PATCHRANDOMACCESSENUMERATOR_H
#define PATCHRANDOMACCESSENUMERATOR_H
#include "TriangleSet.h"
class PatchRandomAccessEnumerator
{
public:
	PatchRandomAccessEnumerator(TriangleSet &tSet){
		m_triData = &tSet;
	}
	Triangle& operator[](int index){
		return m_triData->at(index);
	}
	int count() const {
		return m_triData->count();
	}
private:
	 TriangleSet *m_triData;
};
#endif