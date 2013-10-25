// ------------------------------------------------------------------------------------------------
//
// ISampleInstance
// A currently playing instance of an ISample object.
//
// ------------------------------------------------------------------------------------------------
#pragma once



class RSE_API ISampleInstance
{
public:
	virtual ~ISampleInstance() {}

	virtual void pause() = 0;
	virtual void play() = 0;
	virtual bool playing() = 0;
	virtual void stop() = 0;

	virtual void set3D(const Point3& pos, const Point3& vel) = 0;
};