// ---------------------------------------------------------------------------------------------------------
// 
// Spline
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Math.h"
#include "Dirty.h"
#include <vector>

// HermiteSpline
class HermiteSpline
{
public:
	HermiteSpline() {};

	HermiteSpline(const Point3& start, const Point3& end, const Point3& startTangent, const Point3& endTangent) :
		m_start(start),
		m_end(end),
		m_startTangent(startTangent),
		m_endTangent(endTangent)
	{
	}

	const Point3& start() const { return m_start; }
	const Point3& end() const { return m_end; }
	const Point3& startTangent() const { return m_startTangent; }
	const Point3& endTangent() const { return m_endTangent; }

	Point3 operator()(float t) const
	{
		float t2 = t * t;
		float t3 = t2 * t;

		float h1 = (2.0f * t3) - (3.0f * t2) + 1;
		float h2 = (-2.0f * t3) + (3.0f * t2);
		float h3 = t3 - (2.0f * t2) + t;
		float h4 = t3 - t2;

		return h1 * m_start + h2 * m_end + h3 * m_startTangent + h4 * m_endTangent;
	}

	Point3 derivative(float t) const
	{
		float t2 = t * t;

		float h1 = (6.0f * t2) - (6.0f * t);
		float h2 = (6.0f * t) - (6.0f * t2);
		float h3 = (3.0f * t2) - (4.0f * t) + 1;
		float h4 = (3.0f * t2) - (2.0f * t);

		return h1 * m_start + h2 * m_end + h3 * m_startTangent + h4 * m_endTangent;
	}

	float inverse(float s) const
	{
		int steps = 100;
		float epsilon = 0.001f;
		float t = s / arcLength();
		float lastT = 0.5f;
		
		// newton root finder
		do
		{
			float f = arcLength(0, t) - s;
			float df = derivative(t).length();

			t = t - (f / df);
			if (fabs(t - lastT) < epsilon)
				break;
			lastT = t;

			steps--;
		}
		while (1 && steps);

		return t;
	}

	float arcLength() const
	{
		if (m_arcLength.dirty())
		{
			m_arcLength = arcLength(0, 1);
		}

		return *m_arcLength;
	}

	float arcLength(float t0, float t1) const
	{
		float length = 0;

		// simple euler integration
		int steps = 100;
		float delta = (t1 - t0) / steps;
		float t = t0;

		for (int i = 0; i < steps; i++)
		{
			length += (derivative(t) * delta).length();
			t += delta;
		}

		return length;
	}

protected:
	Point3 m_start;
	Point3 m_end;
	Point3 m_startTangent;
	Point3 m_endTangent;

	mutable Dirty<float> m_arcLength;
};


// CatmullRomSpline
class CatmullRomSpline : public HermiteSpline
{
public:
	CatmullRomSpline() {};

	CatmullRomSpline(const Point3& start, const Point3& end, const Point3& beforeStart, const Point3& afterEnd) :
		HermiteSpline(start, end, 0.5f * (end - beforeStart), 0.5f * (afterEnd - end))
	{
	}
};


// SplinePath
class SplinePath
{
public:
	SplinePath() :
		m_totalLength(0)
	{
	}

	void add(const Point3& p)
	{
		m_points.push_back(p);
		updateSplines();
	}

	const Point3& at(int idx) const
	{
		return m_points[idx];
	}

	int pointSize() const
	{
		return m_points.size();
	}

	int splineSize() const
	{
		return m_points.size() - 1;
	}

	Point3 eval(float t) const
	{
		if (!m_totalLength)
			return Point3(0,0,0);

		// get the spline that the point is on at time t
		clamp(t);
		float curDistance = m_totalLength * t;
		float distance = curDistance;
		
		int idx = 0;
		for (uint i = 0; i < m_lengths.size(); i++)
		{
			distance -= m_lengths[i];
			if (distance < 0)
			{
				idx = i;
				distance += m_lengths[idx]; // distance is now the remaining distance in this spline
				break;
			}
		}

		return evalSpline(idx, distance / (m_lengths[idx]));
	}

	Point3 evalArc(float t) const
	{
		if (!m_totalLength)
			return Point3(0,0,0);

		// get the spline that the point is on at time t
		clamp(t);
		float curDistance = m_totalLength * t;
		float distance = curDistance;
		
		int idx = 0;
		for (uint i = 0; i < m_lengths.size(); i++)
		{
			distance -= m_lengths[i];
			if (distance < 0)
			{
				idx = i;
				distance += m_lengths[idx]; // distance is now the remaining distance in this spline
				break;
			}
		}

		return evalSplineArc(idx, distance);
	}

	float arcLength() const
	{
		return m_totalLength;
	}

protected:
	// get a spline index from a time
	int getIdx(float& t)
	{
		return 0;
	}

	// gets the time for the current spline
	float getSplineTime(float t, int idx)
	{
		return (t * (m_points.size() - 1)) - (t * idx);
	}

	// generate splines when a point is added
	void updateSplines()
	{
		if (m_points.size() > 1)
		{
			int newIdx = m_points.size() - 2;
			createSpline(newIdx);
			float length = evalArcLength(newIdx);
			m_lengths.push_back(length);
			m_totalLength += length;
		}
	}

	// overloadable functions
	virtual void clearSplines()
	{
		// overload
	}

	// create a spline starting a the point with index 'idx'
	virtual void createSpline(int idx)
	{
		// overload
	}

	virtual Point3 evalSpline(int idx, float t) const
	{
		// overload
		return Point3(0,0,0);
	}

	virtual Point3 evalSplineArc(int idx, float d) const
	{
		// overload
		return Point3(0,0,0);
	}

	virtual float evalArcLength(int idx)
	{
		// overload
		return 0;
	}

	float m_totalLength;
	std::vector<float> m_lengths;
	std::vector<Point3> m_points;
};

// CatmullRomPath
class CatmullRomPath : public SplinePath
{
protected:
	virtual void clearSplines()
	{
		m_splines.clear();
	}

	virtual void createSpline(int idx)
	{
		if ((int)m_splines.size() - 1 < idx)
			m_splines.resize(idx + 1);

		int preIdx = idx - 1;
		int endIdx = idx + 1;
		int postIdx = idx + 2;

		clamp(preIdx, 0, (int)m_points.size() - 1);
		clamp(endIdx, 0, (int)m_points.size() - 1);
		clamp(postIdx, 0, (int)m_points.size() - 1);

		m_splines[idx] = CatmullRomSpline(m_points[idx], m_points[endIdx], m_points[preIdx], m_points[postIdx]);
	}

	virtual Point3 evalSpline(int idx, float t) const
	{
		const CatmullRomSpline& s = m_splines[idx];

		return s(t);
	}

	virtual Point3 evalSplineArc(int idx, float d) const
	{
		const CatmullRomSpline& s = m_splines[idx];

		return s(s.inverse(d));
	}

	virtual float evalArcLength(int idx)
	{
		return m_splines[idx].arcLength();
	}

	std::vector<CatmullRomSpline> m_splines;
};
