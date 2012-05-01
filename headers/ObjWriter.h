#ifndef _OBJ_WRITER_H
#define _OBJ_WRITER_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <QtOpenGL>
#include "Primitives.h"

using namespace std;

#define PLY_PATH "./out.ply"
#define OBJ_PATH "./out.obj"
#define XYZ_PATH "./out.xyz"

class ObjWriter {

public:
	static bool exportAsPly(std::vector<uint16_t>, std::vector<int>, int selectedObj, int frontCutoff, int rearCutoff); 
	static bool exportAsObj(std::vector<vertex> vertices, std::vector<tri_face> faces);
	static bool exportAsXyz(std::vector<uint16_t>, std::vector<int>, int selectedObj, int frontCutoff, int rearCutoff); 

private:
};

#endif /* _OBJ_WRITER_H */
