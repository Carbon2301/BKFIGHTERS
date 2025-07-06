#pragma once
#include "../Utilities/Math.h"

struct Vertex 
{
	Vector3 pos;
	Vector3 color;  
	Vector2 uv;  // Thêm UV coordinates cho texture mapping
	
	// Default constructor - khởi tạo từng member riêng biệt
	Vertex() {
		pos.x = 0.0f; pos.y = 0.0f; pos.z = 0.0f;
		color.x = 1.0f; color.y = 1.0f; color.z = 1.0f; 
		uv.x = 0.0f; uv.y = 0.0f;
	}
	
	// Constructor with parameters - khởi tạo từng member riêng biệt  
	Vertex(GLfloat px, GLfloat py, GLfloat pz, 
		   GLfloat cr, GLfloat cg, GLfloat cb,
		   GLfloat u, GLfloat v) {
		pos.x = px; pos.y = py; pos.z = pz;
		color.x = cr; color.y = cg; color.z = cb;
		uv.x = u; uv.y = v;
	}
	
	// Copy constructor - copy từng field riêng biệt
	Vertex(const Vertex& other) {
		pos.x = other.pos.x; pos.y = other.pos.y; pos.z = other.pos.z;
		color.x = other.color.x; color.y = other.color.y; color.z = other.color.z;
		uv.x = other.uv.x; uv.y = other.uv.y;
	}
	
	// Assignment operator
	Vertex& operator=(const Vertex& other) {
		if (this != &other) {
			pos.x = other.pos.x; pos.y = other.pos.y; pos.z = other.pos.z;
			color.x = other.color.x; color.y = other.color.y; color.z = other.color.z;
			uv.x = other.uv.x; uv.y = other.uv.y;
		}
		return *this;
	}
};