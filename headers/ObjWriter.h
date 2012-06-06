#ifndef _OBJ_WRITER_H
#define _OBJ_WRITER_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <QtOpenGL>
#include "Primitives.h"

// CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/property_map.h>
#include <CGAL/remove_outliers.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>

// types
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;

using namespace std;

// file paths
#define PLY_PATH "./out.ply"
#define OBJ_PATH "./out.obj"
#define XYZ_PATH "./out.xyz"
#define PCD_PATH "./out.pcd"

class ObjWriter {

public:
	static bool exportAsPly(std::vector<uint16_t>, std::vector<int>, int selectedObj, int frontCutoff, int rearCutoff); 
	static bool exportAsObj(std::vector<vertex> vertices, std::vector<tri_face> faces);
	static bool exportAsXyz(std::vector<uint16_t>, std::vector<uint8_t>); 
	static bool exportAsPcd(vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff); 

private:
	static bool writePointsToFile(std::vector<vertex> *vertices, ofstream *file); 
	static bool writePointsToFile(std::vector<Point> *vertices, ofstream *file); 
	static void createPointsFromDepth(vector<vertex> *vertices, vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff); 
	static void createPointsFromDepth(vector<vertex> *vertices, vector<uint16_t> depth, vector<uint8_t> filter); 
	static void writePlyHeader(ofstream *file, int numVertices); 
	static void writePcdHeader(ofstream *file, int numVertices); 
};

#endif /* _OBJ_WRITER_H */
