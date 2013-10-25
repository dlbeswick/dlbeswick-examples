// ---------------------------------------------------------------------------------------------------------
// 
// Animation
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "Game/Object.h"
#include "Animation.h"
#include "AppBase.h"

// Interpolation funcs
void Interpolate(float time, const PosKey& start, const PosKey& end, PosKey& var)
{
	Interpolate(time, start.pos, end.pos, var.pos);
}

void Interpolate(float time, const RotKey& start, const RotKey& end, RotKey& var)
{
	Interpolate(time, start.rot, end.rot, var.rot);
}


// ObjectAnimTrack

// setTime
void ObjectAnimTrack::setFrame(float frame)
{
	if (!m_pTarget)
		return;
	
	transformTarget(frame);
}

// transformTarget
void ObjectAnimTrack::transformTarget(float frame)
{
	if (!m_pTarget)
		return;

	PosKey newPos;
	if (transformWith(frame, m_posData, newPos))
		m_pTarget->setPos(newPos.pos);

	RotKey newRot;
	if (transformWith(frame, m_rotData, newRot))
		m_pTarget->setRot(newRot.rot);
}

// transformWith
template <class T>
bool ObjectAnimTrack::transformWith(float frame, SortedVec<T>& data, T& out)
{
	if (data.empty())
	{
		return false;
	}

	T testKey;
	testKey.frame = (int)frame;

	// find the previous and next keys for the given time
	int upper = data.upper_bound(testKey);
	int lower;

	upper = std::min((int)data.size() - 1, upper);
	if (upper > 0)
		lower = upper - 1;
	else
		lower = upper;

	T& prevKey = data[lower];
	T& nextKey = data[upper];

	if (prevKey.frame > frame)
		return false;

	float interpTime;
	float length = (float)nextKey.frame - prevKey.frame;
	if (length == 0 || frame > nextKey.frame)
	{
		interpTime = 1;
	}
	else
	{
		interpTime = (frame - prevKey.frame) / length;
	}

	Interpolate(interpTime, prevKey, nextKey, out);

	return true;
}

// Animation

// construct
Animation::Animation() :
	m_time(0)
{
}

// update
void Animation::update(float delta)
{
	float finalTime = frameToTime(m_length);

	m_time += delta;

	// handle looping
	if (m_time > finalTime)
	{
		if (!m_bLooping)
		{
			m_time = finalTime;
		}
		else
		{
			while (m_time > finalTime)
				m_time -= finalTime;
		}
	}

	// update animation tracks
	for (uint i = 0; i < m_tracks.size(); i++)
	{
		m_tracks[i].setFrame(timeToFrame(m_time));
	}
}

// addTrack
ObjectAnimTrack& Animation::addTrack(Object* pTarget)
{
	m_tracks.insert(m_tracks.end(), ObjectAnimTrack());
	m_tracks.back().setTarget(pTarget);
	return m_tracks.back();
}

// calcLength
void Animation::calcLength(const ObjectAnimTrack& t)
{
	int min = std::min(t.minPosKey(), t.minRotKey());
	int max = std::max(t.maxRotKey(), t.maxRotKey());

	m_length = std::max(m_length, max - min);
}

// calcLength
void Animation::calcLength()
{
	for (uint i = 0; i < m_tracks.size(); i++)
	{
		calcLength(m_tracks[i]);
	}
}