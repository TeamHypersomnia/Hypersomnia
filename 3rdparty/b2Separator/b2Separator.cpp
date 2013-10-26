//
// b2Separator.cpp
// Thermite
//
// Created by Jarad Delorenzo on 1/7/13.
//
//
using namespace std;
#include "b2Separator.h"
#define MAX_VALUE 2147483647

void b2Separator::Separate(b2Body* pBody, b2FixtureDef* pFixtureDef, vector<b2Vec2>* pVerticesVec, int scale) {
	int i, n = pVerticesVec->size(), j, m;
	vector<b2Vec2> vec;
	b2Vec2 *vertices;
	vector<vector<b2Vec2> > figsVec;
	b2PolygonShape* polyShape = new b2PolygonShape();

	for (i = 0; i < n; i++) {
		vec.push_back(b2Vec2(pVerticesVec->at(i).x*scale, pVerticesVec->at(i).y*scale));
	}

	calcShapes(vec, figsVec);
	n = figsVec.size();

	for (i = 0; i < n; i++) {
		vec = figsVec[i];
		m = vec.size();
		vertices = new b2Vec2[m];
		for (j = 0; j < m; j++) {
			vertices[j] = b2Vec2(vec[j].x / scale, vec[j].y / scale);
		}
		polyShape->Set(vertices, m);
		delete [] vertices;
		pFixtureDef->shape = polyShape;
		pBody->CreateFixture(pFixtureDef);
	}
}
/**
* Checks whether the vertices in can be properly distributed into the new fixtures (more specifically, it makes sure there are no overlapping segments and the vertices are in clockwise order).
* It is recommended that you use this method for debugging only, because it may cost more CPU usage.
* @param verticesVec The vertices to be validated.
* @return An integer which can have the following values:
* 0 if the vertices can be properly processed.
* 1 If there are overlapping lines.
* 2 if the points are <b>not</b> in clockwise order.
* 3 if there are overlapping lines and the points are not in clockwise order.
* */
int b2Separator::Validate(const vector<b2Vec2> &verticesVec) {
	int i, n = verticesVec.size(), ret = 0;
	float j, j2, i2, i3, d;
	bool fl, fl2 = false;

	for (i = 0; i < n; i++) {
		i2 = (i < n - 1) ? i + 1 : 0;
		i3 = (i>0) ? i - 1 : n - 1;

		fl = false;
		for (j = 0; j < n; j++) {
			if ((j != i) && (j != i2)) {
				if (!fl) {
					d = det(verticesVec[i].x, verticesVec[i].y, verticesVec[i2].x, verticesVec[i2].y, verticesVec[j].x, verticesVec[j].y);
					if ((d > 0)) {
						fl = true;
					}
				}

				if ((j != i3)) {
					j2 = (j < n - 1) ? j + 1 : 0;
					if (hitSegment(verticesVec[i].x, verticesVec[i].y, verticesVec[i2].x, verticesVec[i2].y, verticesVec[j].x, verticesVec[j].y, verticesVec[j2].x, verticesVec[j2].y)) {
						ret = 1; // TODO: This may be wrong!!!
					}
				}
			}
		}

		if (!fl) {
			fl2 = true;
		}
	}

	if (fl2) {
		if (ret == 1) {
			ret = 3;
		}
		else {
			ret = 2;
		}

	}
	return ret;
}

void b2Separator::calcShapes(const vector<b2Vec2> &pVerticesVec, vector<vector<b2Vec2> > &result) {
	vector<b2Vec2> vec;
	int i, n, j, minLen;
	float d, t, dx, dy;
	int i1, i2, i3;
	b2Vec2 p1, p2, p3;
	int j1, j2;
	b2Vec2 v1, v2;
	int k = 0, h = 0;
	vector<b2Vec2> *vec1, *vec2;
	b2Vec2 *pV, hitV(0, 0);
	bool isConvex;
	vector<vector<b2Vec2> > figsVec;
	queue<vector<b2Vec2> > queue;

	queue.push(pVerticesVec);

	while (!queue.empty()) {
		vec = queue.front();
		n = vec.size();
		isConvex = true;

		for (i = 0; i < n; i++) {
			i1 = i;
			i2 = (i < n - 1) ? i + 1 : i + 1 - n;
			i3 = (i < n - 2) ? i + 2 : i + 2 - n;

			p1 = vec[i1];
			p2 = vec[i2];
			p3 = vec[i3];

			d = det(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
			if ((d < 0)) {
				isConvex = false;
				minLen = MAX_VALUE;

				for (j = 0; j < n; j++) {
					if ((j != i1) && (j != i2)) {
						j1 = j;
						j2 = (j < n - 1) ? j + 1 : 0;

						v1 = vec[j1];
						v2 = vec[j2];

						pV = hitRay(p1.x, p1.y, p2.x, p2.y, v1.x, v1.y, v2.x, v2.y);

						if (pV) {
							b2Vec2 v = *pV;
							dx = p2.x - v.x;
							dy = p2.y - v.y;
							t = dx*dx + dy*dy;

							if ((t < minLen)) {
								h = j1;
								k = j2;
								hitV = v;
								minLen = t;
							}
						}
					}
				}

				if (minLen == MAX_VALUE) {
					//TODO: Throw Error !!!
				}

				vec1 = new vector<b2Vec2>();
				vec2 = new vector<b2Vec2>();

				j1 = h;
				j2 = k;
				v1 = vec[j1];
				v2 = vec[j2];

				if (!pointsMatch(hitV.x, hitV.y, v2.x, v2.y)) {
					vec1->push_back(hitV);
				}
				if (!pointsMatch(hitV.x, hitV.y, v1.x, v1.y)) {
					vec2->push_back(hitV);
				}

				h = -1;
				k = i1;
				while (true) {
					if ((k != j2)) {
						vec1->push_back(vec[k]);
					}
					else {
						if (((h < 0) || h >= n)) {
							//TODO: Throw Error !!!
						}
						if (!isOnSegment(v2.x, v2.y, vec[h].x, vec[h].y, p1.x, p1.y)) {
							vec1->push_back(vec[k]);
						}
						break;
					}

					h = k;
					if (((k - 1) < 0)) {
						k = n - 1;
					}
					else {
						k--;
					}
				}

				reverse(vec1->begin(), vec1->end());

				h = -1;
				k = i2;
				while (true) {
					if ((k != j1)) {
						vec2->push_back(vec[k]);
					}
					else {
						if (((h < 0) || h >= n)) {
							//TODO: Throw Error !!!
						}
						if (((k == j1) && !isOnSegment(v1.x, v1.y, vec[h].x, vec[h].y, p2.x, p2.y))) {
							vec2->push_back(vec[k]);
						}
						break;
					}

					h = k;
					if (((k + 1) > n - 1)) {
						k = 0;
					}
					else {
						k++;
					}
				}

				queue.push(*vec1);
				queue.push(*vec2);
				queue.pop();

				break;
			}
		}

		if (isConvex) {
			figsVec.push_back(queue.front());
			queue.pop();
		}
	}
	result = figsVec;
}

b2Vec2* b2Separator::hitRay(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	float t1 = x3 - x1;
	float t2 = y3 - y1;
	float t3 = x2 - x1;
	float t4 = y2 - y1;
	float t5 = x4 - x3;
	float t6 = y4 - y3;
	float t7 = t4*t5 - t3*t6;

	//DBZ Error. Undefined hit segment.
	if (t7 == 0) return NULL;

	float a = (((t5*t2) - t6*t1) / t7);
	float px = x1 + a*t3;
	float py = y1 + a*t4;
	bool b1 = isOnSegment(x2, y2, x1, y1, px, py);
	bool b2 = isOnSegment(px, py, x3, y3, x4, y4);

	if (b1 && b2) {
		return new b2Vec2(px, py);
	}
	return NULL;
}

b2Vec2* b2Separator::hitSegment(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	float t1 = x3 - x1;
	float t2 = y3 - y1;
	float t3 = x2 - x1;
	float t4 = y2 - y1;
	float t5 = x4 - x3;
	float t6 = y4 - y3;
	float t7 = t4*t5 - t3*t6;

	//DBZ Error. Undefined hit segment.
	if (t7 == 0) return NULL;

	float a = (((t5*t2) - t6*t1) / t7);
	float px = x1 + a*t3;
	float py = y1 + a*t4;
	bool b1 = isOnSegment(px, py, x1, y1, x2, y2);
	bool b2 = isOnSegment(px, py, x3, y3, x4, y4);

	if (b1 && b2) {
		return new b2Vec2(px, py);
	}
	return NULL;
}

bool b2Separator::isOnSegment(float px, float py, float x1, float y1, float x2, float y2) {
	bool b1 = ((x1 + 0.1 >= px && px >= x2 - 0.1) || (x1 - 0.1 <= px && px <= x2 + 0.1));
	bool b2 = ((y1 + 0.1 >= py && py >= y2 - 0.1) || (y1 - 0.1 <= py && py <= y2 + 0.1));
	return (b1 && b2 && isOnLine(px, py, x1, y1, x2, y2));
}

bool b2Separator::pointsMatch(float x1, float y1, float x2, float y2) {
	float dx = (x2 >= x1) ? x2 - x1 : x1 - x2;
	float dy = (y2 >= y1) ? y2 - y1 : y1 - y2;
	return ((dx < 0.1f) && dy < 0.1f);
}

bool b2Separator::isOnLine(float px, float py, float x1, float y1, float x2, float y2) {
	if (x2 - x1 > 0.1f || x1 - x2 > 0.1f) {
		float a = (y2 - y1) / (x2 - x1);
		float possibleY = a*(px - x1) + y1;
		float diff = (possibleY > py ? possibleY - py : py - possibleY);
		return (diff < 0.1f);
	}
	return (px - x1 < 0.1f || x1 - px < 0.1f);
}

float b2Separator::det(float x1, float y1, float x2, float y2, float x3, float y3) {
	return x1*y2 + x2*y3 + x3*y1 - y1*x2 - y2*x3 - y3*x1;
}

//                private function err():void {
//                        throw new Error("A problem has occurred. Use the Validate() method to see where the problem is.");
//                }