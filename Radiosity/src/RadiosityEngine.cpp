#include "RadiosityEngine.h"
#include <cmath>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef _DEBUG
#include <fstream>
std::ofstream fout("debug.log"); 
#endif

#define SHOWTIME	
void RadiosityEngine::computeOneSetp() 
{
	m_render->computeOneStep();
	//exposure(5.0f);
	//interpolate();
	interpolatationAndExposure(50.0f);

#ifdef _DEBUG
	fout << "total count is : " << m_triangleSet->count() << std::endl;
	for(int i = 0; i < m_triangleSet->count(); ++i){
		fout << m_triangleSet->at(i);
	}
#endif
}

void RadiosityEngine::computeNSteps(int steps)
{
#ifdef _DEBUG
	fout << "total count is : " << m_triangleSet->count() << std::endl;
	for(int i = 0; i < m_triangleSet->count(); ++i){
		fout << m_triangleSet->at(i);
	}
	std::cout << "end loging" << std::endl;
#endif

#ifdef SHOWTIME
	time_t start = clock();
#endif
	m_render->computeNStep(steps);
	interpolatationAndExposure(50.0f);
#ifdef SHOWTIME
	std::cout << "total time : "<< (double)(clock() - start) / 1000 << "s" << std::endl;
#endif
}

void RadiosityEngine::convertDataStructure(objLoader *objData, double dTriangleArea /*0.001*/)
{
	//normalize data into range 0.0-1.0
	normalizeData(objData);

	//convert datastructure into trianle structure
	m_triangleSet = boost::shared_ptr<TriangleSet>(new TriangleSet);
	Triangle t;
	for(int i = 0; i < objData->faceCount; ++i) {
		const obj_face *face = objData->faceList[i];
		const double *diffuseVector = objData->materialList[face->material_index]->diff;
		const char* meatrialName = objData->materialList[face->material_index]->name;
		for(int j = 0; j < 3; ++j) {
			int vertexIndex = face->vertex_index[j];
			t.vertexIndex[j] = vertexIndex;
			const obj_vector *v = objData->vertexList[vertexIndex];
			convertVector(t.vertex[j], v);
			if(std::string(meatrialName) == "light\n") {
				t.emission = Color(1.0, 1.0, 1.0);
				t.reflectivity = Color(0.0, 0.0, 0.0);
			}else{
				t.reflectivity = Color(diffuseVector[0], diffuseVector[1], diffuseVector[2]);
				t.emission = Color(0.0, 0.0, 0.0);
			}
			t.radiosity = t.emission;
		}
		m_triangleSet->add(t);
	}

	//get the total num of global vertex number
	m_triangleSet->setTotalVertexCount(objData->vertexCount);

	//divide the all the triangles by size 
	m_triangleSet = TriangleSet::divideBySize(*m_triangleSet.get(), dTriangleArea);

	//create the radiosity render for calculating radiosity
	m_render = boost::shared_ptr<RadiosityRenderer>(new RadiosityRenderer(*m_triangleSet.get(),
		0.0f, 1024 * 1048576));
}

void RadiosityEngine::normalizeData(objLoader *objData)
{
	GLuint  i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;
	GLfloat cx, cy, cz, w, h, d;
	GLfloat scale;

	/* get the max/mins */
	maxx = minx = objData->vertexList[0]->e[0];
	maxy = miny = objData->vertexList[0]->e[1];
	maxz = minz = objData->vertexList[0]->e[2];

	for (i = 1; i < objData->vertexCount; ++i) {
		obj_vector *point = objData->vertexList[i];
		if (maxx < point->e[0])
			maxx = point->e[0];
		if (minx > point->e[0])
			minx = point->e[0];

		if (maxy < point->e[1])
			maxy = point->e[1];
		if (miny > point->e[1])
			miny = point->e[1];

		if (maxz < point->e[2])
			maxz = point->e[2];
		if (minz > point->e[2])
			minz = point->e[2];
	}

	/* calculate model width, height, and depth */
	w = maxx - minx;
	h = maxy - miny;
	d = maxz - minz;

	/* calculate center of the model */
	cx = (maxx + minx) / 2.0f;
	cy = (maxy + miny) / 2.0f;
	cz = (maxz + minz) / 2.0f;

	/* calculate unitizing scale factor */
	GLfloat maxValue = std::max(std::max(w,h), d);
	scale = 2.0f / maxValue;

	/* translate around center then scale */
	for (i = 0; i < objData->vertexCount; ++i) {
		obj_vector *point = objData->vertexList[i];
		point->e[0] -= cx;
		point->e[1] -= cy;
		point->e[2] -= cz;
		point->e[0] *= scale;
		point->e[1] *= scale;
		point->e[2] *= scale;
	}

}

void RadiosityEngine::convertVector(Vertex &vec, const obj_vector *v)
{
	vec.x = v->e[0];
	vec.y = v->e[1];
	vec.z = v->e[2];
}

void RadiosityEngine::getCameraInfo( objLoader * objData )
{
	if(objData->camera != NULL) {
		m_bExiseCamera = true;
		convertVector(m_cameraPos, objData->vertexList[ objData->camera->camera_pos_index ]);
		convertVector(m_cameralookAt, objData->vertexList[ objData->camera->camera_look_point_index ]);
		convertVector(m_cameraUpDir, objData->normalList[ objData->camera->camera_up_norm_index ]);
	}
	else{
		m_bExiseCamera = false;
	}
}

void RadiosityEngine::exposure(float exposurePara)
{
	//int tCount = m_triangleSet->count();
	//for(int i = 0; i < tCount; ++i) {
	//	Triangle& triangle = m_triangleSet->at(i);
	//	Color &rad = triangle.radiosity;
	//	float maxValue = std::max(rad.r, std::max(rad.g, rad.b));
	//	float minValue = std::min(rad.r, std::min(rad.g, rad.b));
	for(int i = 0; i < m_afterIntepolateColor.size(); ++i) {
		Color &rad = m_afterIntepolateColor[i];
		float sum = rad.r + rad.g + rad.b;
		float light = sum / 3.0f;
		rad *= (1 - exp(-light * exposurePara)) * 3 / sum;
	}
}

void RadiosityEngine::interpolate()
{
	//assign the memory 
	m_afterIntepolateColor.resize(m_triangleSet->getTotalVertexCount());

	//first interpolation
	averageVertexColor();

	//average triangle three vertex color
	for(int triIndex = 0; triIndex < m_triangleSet->count(); ++triIndex){
		Triangle &triangle = m_triangleSet->at(triIndex);
		Color sum(0.0f, 0.0f, 0.0f);
		for(int j = 0; j < 3; ++j) {
			sum += m_afterIntepolateColor[triangle.vertexIndex[j]]; 
		}
		sum /= 3.0f;
		triangle.radiosity = sum;
	}

	//second interpolation
	averageVertexColor();

}

void RadiosityEngine::averageVertexColor()
{
	int vertexCount = m_triangleSet->getTotalVertexCount();

	//temporary store the color map
	std::vector<std::vector<const Color*> > tempColor;
	tempColor.resize(vertexCount);

	//loop all the triangles
	for(int triIndex = 0; triIndex < m_triangleSet->count(); ++triIndex){
		const Triangle &triangle = m_triangleSet->at(triIndex);
		const Color* rad = &triangle.radiosity;

		//put three vertex into color map
		for(int j = 0; j < 3; ++j) {
			tempColor[triangle.vertexIndex[j]].push_back(rad);
		}
	}

	//calculate the average color of each global color
	for(int i = 0; i < vertexCount; ++i) {
		Color sum(0.0f, 0.0f, 0.0f);
		int totalCount = tempColor[i].size();
		for(int j = 0; j < totalCount; ++j) {
			sum += *tempColor[i][j];
		}
		if(totalCount > 1)
			sum /= (float)totalCount;

		//store into member data 
		m_afterIntepolateColor[i] = sum;
	}
}

namespace {
	inline std::string vertexToString(const Vertex &v) {
		std::ostringstream oss;
		oss << v.x << "," << v.y << "," << v.z;
		return oss.str();
	}
}

void RadiosityEngine::interpolatationAndExposure(float exposurePara)
{
	// Create and fill TriangleSetExt
	m_triangleSetExt = boost::shared_ptr<TriangleSetExt>(new TriangleSetExt);

	for(int i = 0; i < m_triangleSet->count(); ++i)
	{
		m_triangleSetExt->add(&TriangleExt(m_triangleSet->at(i)));
	}

	// Build map Vertex -> list of colors
	typedef std::vector<Color *> TColorPtrList;
	typedef std::map<std::string, TColorPtrList> TVertexMap;
	TVertexMap vertexMap;
	for(size_t i=0; i<m_triangleSetExt->count(); i++) {
		TriangleExt &te = m_triangleSetExt->operator[](i);

		for(int j=0; j<3; j++) {
			TColorPtrList &cpList = vertexMap[vertexToString(te.vertex[j])];
			cpList.push_back(te.vertexColor+j);
		}
	}

	// 颜色的平均值放入到color中，因为是保存的Color的指针，所以对于TriangleExt中的
	// 数值也会改变
	// Interpolate vertex colors from triangle colors
	TVertexMap::iterator i;
	for(i = vertexMap.begin(); i!= vertexMap.end(); i++) {
		TColorPtrList &cpList= i->second;
		TColorPtrList::iterator j;

		//下面的if语句都算是优化吧，能不计算就不用计算（如果计算的复杂度相对于一个
		//if语句是小的），那就不用计算啦。

		Color sum(0.0, 0.0, 0.0);
		int n=0;
		for(j= cpList.begin(); j!= cpList.end(); j++) {
			Color &color= **j;
			if (Color(0.0,0.0,0.0)==color)
				// Ignore black triangles
				continue;
			sum += color;
			n++;
		}
		if (n>1)
			sum *= 1.0f/n;

		for(j= cpList.begin(); j!= cpList.end(); j++) {
			Color &color= **j;
			color = sum;
		}
	}

	// Save 1st level interpolation
	for(size_t i=0; i<m_triangleSetExt->count(); i++) {
		TriangleExt &te = m_triangleSetExt->operator[](i);

		for(int j=0; j<3; j++)
			te.vertexColorRaw[j] = te.vertexColor[j];
	}

	// Interpolate triangle colors from vertex colors
	for(size_t i=0; i<m_triangleSetExt->count(); i++) {
		TriangleExt &te = m_triangleSetExt->operator[](i);

		Color sum(0.0, 0.0, 0.0);
		for(int j=0; j<3; j++) {
			sum += te.vertexColor[j];
		}
		sum *= 1.0f/3;
		te.radiosityLast = sum;
		for(int n=0; n<3; n++)
			te.vertexColor[n] = sum;
	}

	// Interpolate vertex colors from triangle colors again
	for(i= vertexMap.begin(); i!= vertexMap.end(); i++) {
		TColorPtrList &cpList= i->second;
		TColorPtrList::iterator j;
		Color sum(0.0, 0.0, 0.0);
		for(j= cpList.begin(); j!= cpList.end(); j++) {
			Color &color= **j;
			sum += color;
		}
		sum *= 1.0f/cpList.size();
		for(j= cpList.begin(); j!= cpList.end(); j++) {
			Color &color= **j;
			color = sum;
		}
	}


	//for exposure
	int tCount = m_triangleSetExt->count();
	for(int i = 0; i < tCount; ++i) {
		TriangleExt& triangle = m_triangleSetExt->at(i);
		for(int j = 0; j < 3; ++j) {
			Color &rad = triangle.vertexColor[j];
			float sum = rad.r + rad.g + rad.b;
			float light = sum / 3.0f;
			rad *= (1 - exp(-light * exposurePara)) * 3 / sum;
		}
	}
}