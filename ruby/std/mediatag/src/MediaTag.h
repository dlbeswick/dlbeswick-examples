#ifndef MEDIATAG_MEDIATAG_H
#define MEDIATAG_MEDIATAG_H

#undef read
#undef write
#undef close
#include <string>
#include "Standard/ExceptionStream.h"

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

protected:
	// returns FLT_MAX if the value couldn't be extracted.
	float extractReplaygainDBValue(const std::string& fieldContents);
	float extractReplaygainDBValue(const char* fieldContents);
};

class ExceptionMediaTag : public ExceptionStream
{
public:
	ExceptionMediaTag(const MediaTag& who)
	{
		addContext(who.filePath());
	}
};

#endif
