#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H


struct boundRect {
	int imax;
	int imin;
	int jmax;
	int jmin;
};

struct vertex {
	float x;
	float y;
	float z;
};

struct tri_face {
	int v1;
	int v2;
	int v3;
};



#endif /* _PRIMITIVES_H */

