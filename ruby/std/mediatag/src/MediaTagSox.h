#ifndef MEDIATAG_SOX_H
#define MEDIATAG_SOX_H

#undef read
#undef write
#undef close
#include <string>
#include "MediaTag.h"

class MediaTagSox : public MediaTag
{
public:
	MediaTagSox(const std::string& path);

	virtual std::string artist() const;
	virtual std::string album() const;
	virtual std::string filePath() const;
	virtual std::string title() const;
	virtual std::string trackOrder() const;
	virtual bool hasVolumeNormalisationTrack() const;
	virtual float volumeNormalisationTrack() const;

protected:
	virtual void setComment(void* comments, std::string& field, const std::string& field_name);

	std::string _artist;
	std::string _album;
	std::string _title;
	std::string _trackOrder;
	std::string _path;
	bool _hasVolumeNormalisationTrack;
	float _volumeNormalisationTrack;
};

#endif
