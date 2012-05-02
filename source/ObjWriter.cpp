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

bool ObjWriter::exportAsXyz(vector<uint16_t> depth, vector<int> objects, int selectedObj, int frontCutoff, int rearCutoff) {
	
	if(selectedObj <= 0)
		return false;

	ofstream xyzFile(XYZ_PATH);
	if(!xyzFile.is_open()) {
		printf("Error opening output file at %s\n", XYZ_PATH);
		return false;
	}

	vector<vertex> vertices;
	createPointsFromDepth(&vertices, depth, objects, selectedObj, frontCutoff, rearCutoff);

	if(!writePointsToFile(&vertices, &xyzFile))
		return false;
	xyzFile.close();
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


	plyFile << "ply\n";
	plyFile << "format ascii 1.0\n";
	plyFile.precision(3);
	
	vector<vertex> vertices;
	createPointsFromDepth(&vertices, depth, objects, selectedObj, frontCutoff, rearCutoff);	


	plyFile << "element vertex " << vertices.size() << "\n";
	plyFile << "property float x\n";
	plyFile << "property float y\n";
	plyFile << "property float z\n";
	plyFile << "end_header\n";

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
 
