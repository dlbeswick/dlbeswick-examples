%module mediatag
%{
#include "ID3.h"
#include "MediaTagSox.h"
#include "Standard/api.h"
#include "Standard/Exception.h"
%}

#undef read
#undef write
#undef close

%include std_string.i

%exception {
 try {
   $action
 }
 catch (const Exception& e) {
   rb_raise(rb_eStandardError, e.what());
 }
}

class MediaTag
{
public:
	virtual ~MediaTag() {}

	virtual std::string artist() const = 0;
	virtual std::string album() const = 0;
	virtual std::string title() const = 0;
	virtual std::string trackOrder() const = 0;
	virtual bool hasVolumeNormalisationTrack() const = 0;
	virtual float volumeNormalisationTrack() const = 0;
	virtual std::string filePath() const = 0;
};

class ID3 : public MediaTag
{
public:
	ID3(const char* fName);
	~ID3();
	
	std::string artist() const;
	std::string album() const;
	std::string filePath() const;
	std::string title() const;
	std::string trackOrder() const;
	bool hasVolumeNormalisationTrack() const;
	float volumeNormalisationTrack() const;
};

class MediaTagSox : public MediaTag
{
public:
	MediaTagSox(const std::string& fName);
	~MediaTagSox();

	virtual std::string artist() const;
	virtual std::string album() const;
	std::string filePath() const;
	virtual std::string title() const;
	virtual std::string trackOrder() const;
	virtual bool hasVolumeNormalisationTrack() const;
	virtual float volumeNormalisationTrack() const;
};
