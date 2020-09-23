#include <iostream>
#include "SDL2/SDL.h"
#include <cstdint>
#include <cmath>

template <int R>
class Vector {
	public:
	double data[R];
	Vector() {}
	Vector(double d_[R]) {
		for(int i = 0; i < R; i++) {
			data[i] = d_[i];
		}
	}
	double norm2() {
		double sum = 0;
		for(int i = 0; i < R; i++) {
			sum += data[i] * data[i];
		}
		return sum;
	}
	double norm() {
		return sqrt(norm2());
	}
	Vector<R> operator+(Vector<R>& other) {
		double temp[R];
		for(int i = 0; i < R; i++) {
			temp[i] = data[i] + other.data[i];
		}
		return Vector<R>(temp);
	}
	Vector<R> operator-(Vector<R>& other) {
		double temp[R];
		for(int i = 0; i < R; i++) {
			temp[i] = data[i] - other.data[i];
		}
		return Vector<R>(temp);
	}
	Vector<R> operator/(double d) {
		double temp[R];
		for(int i = 0; i < R; i++) {
			temp[i] = data[i] / d;
		}
		return Vector<R>(temp);
	}
	double dot(Vector<R>& other) {
		double temp = 0;
		for(int i = 0; i < R; i++) {
			temp += data[i] * other.data[i];
		}
		return temp;
	}
	Vector<R> normalized() {
		return Vector<R>(data) / norm();
	}
	double& operator[](int index) {
		return data[index];
	}
	void print() {
		std::cout << "VECTOR BEGIN\n";
		for(int i = 0; i < R; i++) {
			std::cout << data[i] << "\n";
		}
		std::cout << "VECTOR END\n";
	}
};

Vector<3> cross(const Vector<3>& v1, const Vector<3>& v2) {
		double temp[3];
		temp[0] = v1.data[1] * v2.data[2] - v2.data[1] * v1.data[2];
		temp[1] = v1.data[2] * v2.data[0] - v2.data[2] * v1.data[0];
		temp[2] = v1.data[0] * v2.data[1] - v2.data[0] * v1.data[1];
		return Vector<3>(temp);
}

Vector<2> v2(double a, double b) {
	double temp[2];
	temp[0] = a;
	temp[1] = b;
	return Vector<2>(temp);
}

Vector<3> v3(double a, double b, double c) {
	double temp[3];
	temp[0] = a;
	temp[1] = b;
	temp[2] = c;
	return Vector<3>(temp);
}

template <int S>
class Matrix {
	public:
	double data[S][S];
	Matrix() {}
	void AxbAddScaledRow(int from, int target, Vector<S>& b, double scale) {
		for(int c = 0; c < S; c++) {
			data[target][c] += scale * data[from][c];
		}
		b[target] += scale * b[from];
	}
	void AxbScaleRow(int index, Vector<S>& b, double scale) {
		for(int c = 0; c < S; c++) {
			data[index][c] *= scale;
		}
		b[index] *= scale;
	}
	//solves Ax = b returning true if such an x exists or false if not
	bool solveAxb(Vector<S>& x, Vector<S> b) {
		for(int r = 0; r < S; r++) {
			if(data[r][r] == 0) {
				bool hasNonZero = false;
				for(int r2 = r + 1; r2 < S; r2++) {
					if(data[r2][r] != 0) {
						hasNonZero = true;
						AxbAddScaledRow(r2, r, b, 1);
						break;
					}
				}
				if(!hasNonZero) {
					return false;
				}
			}
			AxbScaleRow(r, b, 1 / data[r][r]);
			for(int r2 = 0; r2 < S; r2++) {
				if(r != r2) {
					AxbAddScaledRow(r, r2, b, -data[r2][r]);
				}
			} 
		}
		x = b;
		return true;
	}
};

class Face {
	public:
	Vector<3> p1;
	Vector<3> p2;
	Vector<3> p3;
	Face(Vector<3> p1_, Vector<3> p2_, Vector<3> p3_) {
		p1 = p1_;
		p2 = p2_;
		p3 = p3_;
	}
	Vector<3> getNormal() {
		return cross(p2 - p1, p3 - p1).normalized();
	}
};

class Raycast {
	Vector<3> base;
	Vector<3> direction;
	Raycast(Vector<3> b_, Vector<3> d_) {
		base = b_;
		direction = d_;
	}
};

class Pixel {
	public:
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	Pixel(uint8_t r, uint8_t g, uint8_t b) {
		red = r;
		green = g;
		blue = b;
	}
};

Pixel getPixelColor(int x, int y) {
	int dx = x - 320;
	int dy = y - 240;
	if(dx * dx + dy * dy < 2500) {
		return Pixel(0xff, 0xff, 0xff);
	} else {
		return Pixel(0, 0, 0);
	}
}

int main() {
	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("RayTracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
	if(!window) {
		std::cout << "No window today :(\n";
		return 1;
	}
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	bool shouldEnd = false;
	SDL_Event windowEvent;
	Matrix<2> A;
	A.data[0][0] = A.data[1][1] = 1;
	A.data[0][1] = A.data[1][0] = 0;
	Vector<2> x;
	A.solveAxb(x, v2(1, 1));
	x.print();
	while(!shouldEnd) {
		while(SDL_PollEvent(&windowEvent) != 0) {
			if(windowEvent.type == SDL_QUIT) {
				shouldEnd = true;
			}
		}
		for(int x = 0; x < 640; x++) {
			for(int y = 0; y < 480; y++) {
				Pixel p = getPixelColor(x, y);
				uint32_t color = (p.red << surface->format->Rshift) | (p.green << surface->format->Gshift) | (p.blue << surface->format->Bshift);
				((uint32_t*)(surface->pixels))[y * 640 + x] = color;
			}
		}
		SDL_UpdateWindowSurface(window);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
