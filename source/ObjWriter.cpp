#include "ObjWriter.h"

bool ObjWriter::exportAsPly(vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff) {

	if(selectedObj <= 0 )
		return false;

	ofstream plyFile(PLY_PATH);
	if(!plyFile.is_open()) {
		printf("Error opening output file at %s\n", PLY_PATH);
		return false;
	}

	vector<vertex> vertices;

	plyFile << "ply\n";
	plyFile << "format ascii 1.0\n";
	plyFile.precision(3);
	
	float xWorld = -1.0f, yWorld = 1.0f, zWorld;

	printf("fc: %d\nrc: %d\n", frontCutoff, rearCutoff);
	for (int i = 0; i < 640 * 480; i++) {
		if( (depth[i] > frontCutoff) && (depth[i] < rearCutoff) && (objects[i] == selectedObj)) {
			zWorld = (float)depth[i]/100.0f;
			vertex v = {xWorld, yWorld, zWorld};
			vertices.push_back(v);
		}

		if (i % 640 == 0) {
			yWorld -= 1.f / 320.f;
			xWorld = -1.f;
		} else {
			xWorld += 1.f / 240.f;
		}
	}


	plyFile << "element vertex " << vertices.size() << "\n";
	plyFile << "property float x\n";
	plyFile << "property float y\n";
	plyFile << "property float z\n";
	plyFile << "end_header\n";

	vertex p;
	for (int i = 0; i < (int)vertices.size(); ++i) {
		p = vertices[i];
		plyFile << fixed << p.x << " " << fixed << p.y << " " << fixed << p.z << "\n"; 
	}

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
 
