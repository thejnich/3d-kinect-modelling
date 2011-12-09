#ifndef _OBJ_WRITER_H
#define _OBJ_WRITER_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <QtOpenGL>

using namespace std;

#define OUT_PATH "./out.ply"

struct vertex {
	float x;
	float y;
	float z;
};

class ObjWriter {

public:
	static bool exportAsObj(std::vector<uint16_t>, std::vector<int>, int selectedObj, int frontCutoff, int rearCutoff); 

private:
};

#endif /* _OBJ_WRITER_H */
