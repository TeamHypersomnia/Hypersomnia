#pragma once
//
// b2Separator.h
// Thermite
//
// Created by Jarad Delorenzo on 1/7/13.
//
//

#include <Box2D/Box2D.h>
#include <vector>
#include <queue>
#include <algorithm>


/*
* Convex Separator for Box2D Flash
*
* This class has been written by Antoan Angelov.
* It is designed to work with Erin Catto's Box2D physics library.
*
* Everybody can use this software for any purpose, under two restrictions:
* 1. You cannot claim that you wrote this software.
* 2. You can not remove or alter this notice.
*
*/

class b2Separator {

public:

	b2Separator() {}

	/**
	* Separates a non-convex polygon into convex polygons and adds them as fixtures to the <code>body</code> parameter.<br/>
	* There are some rules you should follow (otherwise you might get unexpected results) :
	* <ul>
	* <li>This class is specifically for non-convex polygons. If you want to create a convex polygon, you don't need to use this class - Box2D's <code>b2PolygonShape</code> class allows you to create convex shapes with the <code>setAsArray()</code>/<code>setAsVector()</code> method.</li>
	* <li>The vertices must be in clockwise order.</li>
	* <li>No three neighbouring points should lie on the same line segment.</li>
	* <li>There must be no overlapping segments and no "holes".</li>
	* </ul> <p/>
	* @param body The b2Body, in which the new fixtures will be stored.
	* @param fixtureDef A b2FixtureDef, containing all the properties (friction, density, etc.) which the new fixtures will inherit.
	* @param verticesVec The vertices of the non-convex polygon, in clockwise order.
	* @param scale <code>[optional]</code> The scale which you use to draw shapes in Box2D. The bigger the scale, the better the precision. The default value is 30.
	* @see b2PolygonShape
	* @see b2PolygonShape.SetAsArray()
	* @see b2PolygonShape.SetAsVector()
	* @see b2Fixture
	**/

	void Separate(b2Body* pBody, b2FixtureDef* pFixtureDef, std::vector<b2Vec2>* pVerticesVec, int scale);
	/**
	* Checks whether the vertices in <code>verticesVec</code> can be properly distributed into the new fixtures (more specifically, it makes sure there are no overlapping segments and the vertices are in clockwise order).
	* It is recommended that you use this method for debugging only, because it may cost more CPU usage.
	* <p/>
	* @param verticesVec The vertices to be validated.
	* @return An integer which can have the following values:
	* <ul>
	* <li>0 if the vertices can be properly processed.</li>
	* <li>1 If there are overlapping lines.</li>
	* <li>2 if the points are <b>not</b> in clockwise order.</li>
	* <li>3 if there are overlapping lines <b>and</b> the points are <b>not</b> in clockwise order.</li>
	* </ul>
	* */

	int Validate(const std::vector<b2Vec2>& verticesVec);

	void calcShapes(const std::vector<b2Vec2> &pVerticesVec, std::vector<std::vector<b2Vec2> > &result);
	b2Vec2* hitRay(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	b2Vec2* hitSegment(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	bool isOnSegment(float px, float py, float x1, float y1, float x2, float y2);
	bool pointsMatch(float x1, float y1, float x2, float y2);
	bool isOnLine(float px, float py, float x1, float y1, float x2, float y2);
	float det(float x1, float y1, float x2, float y2, float x3, float y3);
};