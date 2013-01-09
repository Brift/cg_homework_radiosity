#ifndef RADIOSITYENGINE_H
#define RADIOSITYENGINE_H
#include <GL/glut.h>
#include <boost/shared_ptr.hpp>
#include "TriangleSet.h"
#include "objLoader.h"
#include "RadiosityRenderer.h"
#include "TriangleSetExt.h"


class RadiosityEngine
{
public:
	RadiosityEngine(objLoader *objData, double dTriangleArea = 0.001){
		convertDataStructure(objData, dTriangleArea);
		getCameraInfo(objData);
	}
	void computeNSteps(int steps);

	boost::shared_ptr<TriangleSet> m_triangleSet;
	bool m_bExiseCamera;
	Vertex m_cameraPos;
	Vertex m_cameralookAt;
	Vertex m_cameraUpDir;
	boost::shared_ptr<RadiosityRenderer> m_render;
	std::vector<Color> m_afterIntepolateColor;
	boost::shared_ptr<TriangleSetExt> m_triangleSetExt;

private:
	void convertDataStructure(objLoader *objData, double dTriangleArea = 0.001);
	void getCameraInfo(objLoader *objData);
	void convertVector(Vertex &vec, const obj_vector *v);
	void normalizeData(objLoader *objData);
	void computeOneSetp();
	void exposure(float exposurePara);
	void interpolate();

	void averageVertexColor();
	void interpolatationAndExposure(float exposurePara);
};
#endif