// ---------------------------------------------------------------------------------------------------------
// 
// Animation
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/SortedVec.h>
class Object;

// Animation keys
class RSE_API AnimKey
{
public:
	int frame;
};

inline bool operator < (const AnimKey& lhs, const AnimKey& rhs)
{
	return lhs.frame < rhs.frame;
};


class RSE_API PosKey : public AnimKey
{
public:
	Point3 pos;
};

class RSE_API RotKey : public AnimKey
{
public:
	Quat rot;
};


// A single track of animation for an object
class RSE_API ObjectAnimTrack
{
public:
	typedef SortedVec<AnimKey> AnimData;
	typedef SortedVec<PosKey> PosData;
	typedef SortedVec<RotKey> RotData;

	ObjectAnimTrack() :
		m_pTarget(0)
	{
	}

	void setTarget(Object* p) { m_pTarget = p; }
	Object* target() { return m_pTarget; }

	void addPosKey(const PosKey& key) { m_posData.insert(key); }
	int minPosKey() const { if (m_posData.empty()) return 0; return m_posData[0].frame; }
	int maxPosKey() const { if (m_posData.empty()) return 0; return m_posData[m_posData.size() - 1].frame; }

	void addRotKey(const RotKey& key) { m_rotData.insert(key); }
	int minRotKey() const { if (m_rotData.empty()) return 0; return m_rotData[0].frame; }
	int maxRotKey() const { if (m_rotData.empty()) return 0; return m_rotData[m_rotData.size() - 1].frame; }

	void setFrame(float frame);

private:
	void transformTarget(float frame);
	template <class T> bool transformWith(float frame, SortedVec<T>& data, T& out);

	Object*	m_pTarget;

	PosData		m_posData;
	RotData		m_rotData;
};


// A collection of animation tracks that affect a hierarchy of bones
class RSE_API Animation
{
public:
	typedef stdext::hash_map<std::string, IntRange> SequenceList;

	Animation();

	ObjectAnimTrack& addTrack(Object* pTarget);
	void addSequence(const std::string& name, IntRange t) { m_sequences[name] = t; }
	uint size() { return (uint)m_sequences.size(); }
	void update(float delta);
	
	float length() const { return m_length / 60.0f; }
	float time() const { return m_time; }

	void calcLength();
	void calcLength(const ObjectAnimTrack& t);

	float frameToTime(int frame) { return frame / m_fps; }
	float timeToFrame(float time) { return time * m_fps; }

	void setFps(float f) { m_fps = f; }
	float fps() { return m_fps; }

	void setTime(float t) { m_time = t; }
	void setLooping(bool b) { m_bLooping = b; }

	void play(const std::string& name);

private:
	typedef std::vector<ObjectAnimTrack> TrackList;

	SequenceList	m_sequences;
	TrackList		m_tracks;
	float			m_time;
	int				m_length;
	float			m_fps;
	bool			m_bLooping;
};
