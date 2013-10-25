#include "MediaTagSox.h"
#include "Standard/Log.h"
#include <sox.h>
#include <limits>

MediaTagSox::MediaTagSox(const std::string& path) :
	_path(path),
	_volumeNormalisationTrack(0),
	_hasVolumeNormalisationTrack(false)
{
	sox_format_t* format = 0;

	try
	{
		format = sox_open_read(path.c_str(), 0, 0, 0);

		if (!format)
			EXCEPTIONSTREAM(ExceptionMediaTag(*this), "sox_open_read failed.");

		setComment(format->oob.comments, _artist, "ARTIST");
		setComment(format->oob.comments, _album, "ALBUM");
		setComment(format->oob.comments, _title, "TITLE");
		setComment(format->oob.comments, _trackOrder, "TRACKNUMBER");

		float replaygain_track_gain = extractReplaygainDBValue(
			sox_find_comment(format->oob.comments, "replaygain_track_gain")
		);

		_hasVolumeNormalisationTrack = replaygain_track_gain != FLT_MAX;

		if (_hasVolumeNormalisationTrack)
		_volumeNormalisationTrack = replaygain_track_gain;

		sox_close(format);
	}
	catch (...)
	{
		sox_close(format);
		throw;
	}
}

void MediaTagSox::setComment(void* comments, std::string& field, const std::string& field_name)
{
	const char* result = sox_find_comment((sox_comments_t)comments, field_name.c_str());
	if (result)
		field = result;
}

float MediaTagSox::volumeNormalisationTrack() const
{
	return _volumeNormalisationTrack;
}

bool MediaTagSox::hasVolumeNormalisationTrack() const
{
	return _hasVolumeNormalisationTrack;
}

std::string MediaTagSox::filePath() const
{
	return _path;
}

std::string MediaTagSox::artist() const
{
	return _artist;
}

std::string MediaTagSox::album() const
{
	return _album;
}

std::string MediaTagSox::title() const
{
	return _title;
}

std::string MediaTagSox::trackOrder() const
{
	return _trackOrder;
}

