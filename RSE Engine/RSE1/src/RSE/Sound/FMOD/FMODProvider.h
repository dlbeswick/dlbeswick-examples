// ------------------------------------------------------------------------------------------------
//
// FMODProvider
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Sound/ISoundProvider.h"
#include "Standard/Base.h"
#include "Standard/SmartPtr.h"

class Camera;

#if IS_MSVC
	class hash_compare_stricmp
	{
	public:
		enum
			{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

		size_t operator()(const std::string& _Str) const
		{	// hash _Keyval to size_t value
			size_t _Val = _HASH_SEED;
			size_t _Size = _Str.size();
			if (0 < _Size)
				{	// add one or more elements
				size_t _Stride = (_Size / 16) + 1;
				_Size -= _Stride;	// protect against _Size near _Str.max_size()
				for(size_t _Idx = 0; _Idx <= _Size; _Idx += _Stride)
					_Val += (size_t)tolower(_Str[_Idx]);
				}
			return (_Val);
		}

		bool operator()(const std::string& _Keyval1, const std::string& _Keyval2) const
		{	// test if _Keyval1 ordered before _Keyval2
			return stricmp(_Keyval1.c_str(), _Keyval2.c_str()) < 0;
		}
	};
#else
	class hash_compare_istr
	{
	public:
		bool operator()(const std::string& _Keyval1, const std::string& _Keyval2) const
		{	// test if _Keyval1 ordered before _Keyval2
			return stricmp(_Keyval1.c_str(), _Keyval2.c_str()) < 0;
		}
	};

	class hash_val_istr
	{
	public:
		std::size_t operator()(std::string key) const
		{	// test if _Keyval1 ordered before _Keyval2
			std::transform(key.begin(), key.end(), key.begin(), tolower);
			return std::hash<std::string>()(key);
		}
	};
#endif

class RSE_API FMODProvider : public ISoundProvider
{
	USE_RTTI(FMODProvider, Base);
public:
	FMODProvider();
	~FMODProvider();

	virtual void construct();

	virtual void update(float delta);

	virtual void setListener(const SoundListener* listener);
	virtual void setGlobalVolume(float v);
	virtual void setSoundVolume(float v);
	virtual void setMusicVolume(float v);

	virtual float globalVolume() const;
	virtual float soundVolume() const;
	virtual float musicVolume() const;

	virtual ISample& sample(const PathResource& path);
	virtual PtrGC<IMusic> music(const PathResource& path);

	virtual void onListenerDestroyed(const SoundListener& listener);

	static std::string error(int e = INT_MAX);

protected:
	SmartPtr<ibinstream> soundStream(const PathResource& path);
	SmartPtr<ibinstream> musicStream(const PathResource& path);
	
#if IS_MSVC
	typedef stdext::hash_map<std::string, ISample*, hash_compare_stricmp> Samples;
#else
	typedef std::unordered_map<std::string, ISample*, hash_val_istr, hash_compare_istr> Samples;
#endif

	Samples m_samples;

	typedef PendingListNullRemover<PtrGC<IMusic> > Music;
	Music m_music;

	float m_globalVolume;
	float m_soundVolume;
	float m_musicVolume;
	Matrix4 m_referenceFrame;
	Matrix4 m_listenerToReference;

	const SoundListener* m_listener;
};
