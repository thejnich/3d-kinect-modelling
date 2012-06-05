#include "ObjWriter.h"

bool ObjWriter::writePointsToFile(vector<vertex> *vertices, ofstream* file) {

	if(!file->is_open()){
		printf("error writing data to file\n");
		return false;
	}

	vertex p;
	for (int i = 0; i < (int)vertices->size(); ++i) {
		p = (*vertices)[i];
		*file << fixed << p.x << " " << fixed << p.y << " " << fixed << p.z << "\n"; 
	}
	return true;
}

/*
	add points(xyz) to vertices, based on depth map and selected object provided
*/
void ObjWriter::createPointsFromDepth(vector<vertex> *vertices, vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff) {
		
	float xWorld = -1.0f, yWorld = 1.0f, zWorld;

	for (int i = 0; i < 640 * 480; i++) {
		if( (depth[i] > frontCutoff) && (depth[i] < rearCutoff) && (objects[i] == selectedObj)) {
			// multiply by -1 because depth is oposite in these file formats compared with rendering in openGL
			zWorld = -1.f*(float)depth[i]/100.0f;
			vertex v = {xWorld, yWorld, zWorld};
			vertices->push_back(v);
		}

		if (i % 640 == 0) {
			yWorld -= 1.f / 320.f;
			xWorld = -1.f;
		} else {
			xWorld += 1.f / 240.f;
		}
	}

}

void ObjWriter::createPointsFromDepth(vector<vertex> *vertices, vector<uint16_t> depth, vector<uint8_t> filter) {
		
	float xWorld = -1.0f, yWorld = 1.0f, zWorld;

	for (int i = 0; i < 640 * 480; i++) {
		if( filter[3*(i+640)]==255) {
			// multiply by -1 because depth is oposite in these file formats compared with rendering in openGL
			zWorld = -1.f*(float)depth[i]/100.0f;
			vertex v = {xWorld, yWorld, zWorld};
			vertices->push_back(v);
		}

		if (i % 640 == 0) {
			yWorld -= 1.f / 320.f;
			xWorld = -1.f;
		} else {
			xWorld += 1.f / 240.f;
		}
	}

}


void ObjWriter::writePlyHeader(ofstream *file, int numVertices) {
	*file << "ply\n";
	*file << "format ascii 1.0\n";
	file->precision(3);
	
	*file << "element vertex " << numVertices << "\n";
	*file << "property float x\n";
	*file << "property float y\n";
	*file << "property float z\n";
	*file << "end_header\n";
}

void ObjWriter::writePcdHeader(ofstream *file, int numVertices) {
	*file << "# .PCD v.7 - Point Cloud Data file format\n";
	*file << "VERSION .7\n";
	*file << "FIELDS x y z\n";
	*file << "SIZE 4 4 4\n";
	*file << "TYPE F F F\n";
	*file << "COUNT 1 1 1\n";
	file->precision(3);
	*file << "WIDTH " << numVertices << "\n";
	*file << "HEIGHT 1\n";
	*file << "VIEWPOINT 0 0 0 1 0 0 0\n";
	*file << "POINTS " << numVertices << "\n";
	*file << "DATA ascii\n";
}

bool ObjWriter::exportAsXyz(vector<uint16_t> depth, vector<uint8_t> filter) {
	
	ofstream xyzFile(XYZ_PATH);
	if(!xyzFile.is_open()) {
		printf("Error opening output file at %s\n", XYZ_PATH);
		return false;
	}

	vector<vertex> vertices;
	createPointsFromDepth(&vertices, depth, filter);

	if(!writePointsToFile(&vertices, &xyzFile))
		return false;
	xyzFile.close();
	printf("done\n");
	return true;
}

bool ObjWriter::exportAsPcd(vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff) {
	if(selectedObj <= 0)
		return false;

	ofstream pcdFile(PCD_PATH);
	if(!pcdFile.is_open()) {
		printf("Error openint output file at %s\n", PCD_PATH);
		return false;
	}

	vector<vertex> vertices;
	createPointsFromDepth(&vertices, depth, objects, selectedObj, frontCutoff, rearCutoff);

	writePcdHeader(&pcdFile, vertices.size());

	if(!writePointsToFile(&vertices, &pcdFile))
		return false;
	pcdFile.close();
	printf("done\n");
	return true;
}

bool ObjWriter::exportAsPly(vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff) {

	if(selectedObj <= 0 )
		return false;

	ofstream plyFile(PLY_PATH);
	if(!plyFile.is_open()) {
		printf("Error opening output file at %s\n", PLY_PATH);
		return false;
	}

	vector<vertex> vertices;
	createPointsFromDepth(&vertices, depth, objects, selectedObj, frontCutoff, rearCutoff);	

	writePlyHeader(&plyFile, vertices.size());

	if(!writePointsToFile(&vertices, &plyFile))
		return false;
	plyFile.close();
	printf("done\n");
	return true;

}

bool ObjWriter::exportAsObj(std::vector<vertex> vertexList, std::vector<tri_face> faceList)
{
	ofstream objFile(OBJ_PATH);
	if(!objFile.is_open()) {
		printf("Error opening output file at %s\n", OBJ_PATH);
		return false;
	}

	objFile.precision(3);
	objFile << "# File created by 3d-kinect-modelling\n";
	objFile << "# vertices\n";
	
	for (int i = 0; i < (int)vertexList.size(); ++i) {
		objFile << "v " << fixed << vertexList[i].x << " " << fixed << vertexList[i].y<< " " << fixed << vertexList[i].z << endl;
	}

	objFile << "# faces\n";

	for (int i = 0; i < (int)faceList.size(); ++i) {
		objFile << "f " << faceList[i].v1 +1 << " " <<  faceList[i].v2+1 << " " <<  faceList[i].v3+1 << endl;
	}

	objFile.close();
	printf("done\n");
	return true;
}
 
