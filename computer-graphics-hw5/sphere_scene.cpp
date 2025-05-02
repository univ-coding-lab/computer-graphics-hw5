//
//  sphere_scene.c
//  Rasterizer
//
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int     gNumVertices = 0;    // Number of 3D vertices.
int     gNumTriangles = 0;    // Number of triangles.
int* gIndexBuffer = NULL; // Vertex indices for the triangles.
unsigned char image[512][512][3];
float z_buffer[512][512];

struct Vec3 {
	float x, y, z;
};
Vec3* gVertexBuffer = nullptr;

Vec3 model_transform(const Vec3& v) {
	return { v.x * 2, v.y * 2, v.z * 2 - 7 };
}

Vec3 perspective_transform(const Vec3& v) {
	float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, n = -0.1f, f = -1000.0f;

	float x = (2 * n * v.x) / (r - l);
	float y = (2 * n * v.y) / (t - b);
	float z = (f + n + 2 * f * n / v.z) / (f - n);
	return { x, y, z };
}

Vec3 viewport_transform(const Vec3& v) {
	int nx = 512, ny = 512;
	return {
		(v.x + 1.0f) * 0.5f * nx,
		(1.0f - v.y) * 0.5f * ny,
		v.z
	};
}

Vec3 transform_vertex(const Vec3& v) {
	Vec3 model = model_transform(v);
	Vec3 clip = perspective_transform(model);
	Vec3 ndc = { clip.x / -model.z, clip.y / -model.z, clip.z };
	Vec3 screen = viewport_transform(ndc);
	return screen;
}

void clear_buffers() {
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			image[y][x][0] = image[y][x][1] = image[y][x][2] = 0;
			z_buffer[y][x] = 1e9f;
		}
	}
}

void draw_triangle(Vec3 v0, Vec3 v1, Vec3 v2) {
	// Bounding box
	float min_x = std::min(std::min(v0.x, v1.x), v2.x);
	float max_x = std::max(std::max(v0.x, v1.x), v2.x);
	float min_y = std::min(std::min(v0.y, v1.y), v2.y);
	float max_y = std::max(std::max(v0.y, v1.y), v2.y);

	int minX = std::max(0, (int)std::floor(min_x));
	int maxX = std::min(511, (int)std::ceil(max_x));
	int minY = std::max(0, (int)std::floor(min_y));
	int maxY = std::min(511, (int)std::ceil(max_y));

	float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);

	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			float px = x + 0.5f;
			float py = y + 0.5f;

			float alpha = ((v1.y - v2.y) * (px - v2.x) + (v2.x - v1.x) * (py - v2.y)) / denom;
			float beta = ((v2.y - v0.y) * (px - v2.x) + (v0.x - v2.x) * (py - v2.y)) / denom;
			float gamma = 1.0f - alpha - beta;

			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				float z = alpha * v0.z + beta * v1.z + gamma * v2.z;
				if (z < z_buffer[y][x]) {
					z_buffer[y][x] = z;
					image[y][x][0] = image[y][x][1] = image[y][x][2] = 255;
				}
			}
		}
	}
}

void render_scene() {
	for (int i = 0; i < gNumTriangles; ++i) {
		int k0 = gIndexBuffer[3 * i + 0];
		int k1 = gIndexBuffer[3 * i + 1];
		int k2 = gIndexBuffer[3 * i + 2];

		Vec3 v0 = transform_vertex(gVertexBuffer[k0]);
		Vec3 v1 = transform_vertex(gVertexBuffer[k1]);
		Vec3 v2 = transform_vertex(gVertexBuffer[k2]);

		draw_triangle(v0, v1, v2);
	}
}

void save_image(const char* filename) {
	FILE* fp = fopen(filename, "wb");
	fprintf(fp, "P6\n512 512\n255\n");
	fwrite(image, 1, 512 * 512 * 3, fp);
	fclose(fp);
}

void create_scene()
{
	gVertexBuffer = new Vec3[gNumVertices];
	int width = 32;
	int height = 16;

	float theta, phi;
	int t;

	gNumVertices = (height - 2) * width + 2;
	gNumTriangles = (height - 2) * (width - 1) * 2;

	// TODO: Allocate an array for gNumVertices vertices.

	gIndexBuffer = new int[3 * gNumTriangles];

	t = 0;
	for (int j = 1; j < height - 1; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			theta = (float)j / (height - 1) * M_PI;
			phi = (float)i / (width - 1) * M_PI * 2;

			float   x = sinf(theta) * cosf(phi);
			float   y = cosf(theta);
			float   z = -sinf(theta) * sinf(phi);

			// TODO: Set vertex t in the vertex array to {x, y, z}.

			gVertexBuffer[t++] = { x, y, z };
		}
	}

	// TODO: Set vertex t in the vertex array to {0, 1, 0}.

	gVertexBuffer[t++] = { 0.0f, 1.0f, 0.0f };

	// TODO: Set vertex t in the vertex array to {0, -1, 0}.

	gVertexBuffer[t++] = { 0.0f, -1.0f, 0.0f };

	t = 0;
	for (int j = 0; j < height - 3; ++j)
	{
		for (int i = 0; i < width - 1; ++i)
		{
			gIndexBuffer[t++] = j * width + i;
			gIndexBuffer[t++] = (j + 1) * width + (i + 1);
			gIndexBuffer[t++] = j * width + (i + 1);
			gIndexBuffer[t++] = j * width + i;
			gIndexBuffer[t++] = (j + 1) * width + i;
			gIndexBuffer[t++] = (j + 1) * width + (i + 1);
		}
	}
	for (int i = 0; i < width - 1; ++i)
	{
		gIndexBuffer[t++] = (height - 2) * width;
		gIndexBuffer[t++] = i;
		gIndexBuffer[t++] = i + 1;
		gIndexBuffer[t++] = (height - 2) * width + 1;
		gIndexBuffer[t++] = (height - 3) * width + (i + 1);
		gIndexBuffer[t++] = (height - 3) * width + i;
	}

	// The index buffer has now been generated. Here's how to use to determine
	// the vertices of a triangle. Suppose you want to determine the vertices
	// of triangle i, with 0 <= i < gNumTriangles. Define:
	//
	// k0 = gIndexBuffer[3*i + 0]
	// k1 = gIndexBuffer[3*i + 1]
	// k2 = gIndexBuffer[3*i + 2]
	//
	// Now, the vertices of triangle i are at positions k0, k1, and k2 (in that
	// order) in the vertex array (which you should allocate yourself at line
	// 27).
	//
	// Note that this assumes 0-based indexing of arrays (as used in C/C++,
	// Java, etc.) If your language uses 1-based indexing, you will have to
	// add 1 to k0, k1, and k2.
}

int main() {
	create_scene();
	clear_buffers();
	render_scene();
	save_image("output.ppm");
	return 0;
}

