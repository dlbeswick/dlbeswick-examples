#ifndef MEDIATAG_ID3_H
#define MEDIATAG_ID3_H

#undef read
#undef write
#undef close
#include <string>
#include "MediaTag.h"

struct id3_tag;

class ID3 : public MediaTag
{
public:
	ID3(const std::string& fName);
	~ID3();

	virtual std::string artist() const;
	virtual std::string album() const;
	std::string filePath() const;
	virtual std::string title() const;
	virtual std::string trackOrder() const;
	virtual bool hasVolumeNormalisationTrack() const;
	virtual float volumeNormalisationTrack() const;

protected:
	std::string string(id3_tag& tag, const char* name) const;
	std::string txxx(id3_tag& tag, const char* description) const;

	std::string _artist;
	std::string _album;
	std::string _title;
	std::string _trackOrder;
	std::string _fname;
	bool _hasVolumeNormalisationTrack;
	float _volumeNormalisationTrack;
};

#endif
