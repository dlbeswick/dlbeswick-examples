#include "ID3.h"
#include "id3tag.h"
#include <assert.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <limits>
#include "Standard/api.h"
#include "Standard/Exception.h"

ID3::ID3(const std::string& fName) :
	_fname(fName),
	_volumeNormalisationTrack(0),
	_hasVolumeNormalisationTrack(false)
{
	id3_file* file = id3_file_open(_fname.c_str(), ID3_FILE_MODE_READONLY);
	if (!file)
		EXCEPTIONSTREAM(ExceptionMediaTag(*this), "Couldn't open file.");

	id3_tag* tag = id3_file_tag(file);
	if (!tag)
		EXCEPTIONSTREAM(ExceptionMediaTag(*this), "Couldn't get id3 tag from file.");

	_artist = string(*tag, "TPE1");
	_album = string(*tag, "TALB");
	_title = string(*tag, "TIT2");
	_trackOrder = string(*tag, "TRCK");
	
	float replaygain_track_gain = extractReplaygainDBValue(txxx(*tag, "replaygain_track_gain"));
	_hasVolumeNormalisationTrack = replaygain_track_gain != FLT_MAX;

	if (_hasVolumeNormalisationTrack)
	_volumeNormalisationTrack = replaygain_track_gain;

	id3_file_close(file);
}

ID3::~ID3()
{
}

std::string ID3::artist() const
{
	return _artist;
}

std::string ID3::album() const
{
	return _album;
}

std::string ID3::title() const
{
	return _title;
}

std::string ID3::trackOrder() const
{
	return _trackOrder;
}

std::string ID3::string(id3_tag& tag, const char* name) const
{
	id3_frame* frame = id3_tag_findframe(&tag, name, 0);
	if (!frame)
		return "";

	id3_field* field = id3_frame_field(frame, 1);
	if (!field)
		return "";

	const id3_ucs4_t* wstr = 0;

	// tbd: need to also handle latin1 here
	if (field->type == ID3_FIELD_TYPE_STRINGLIST)
		wstr = id3_field_getstrings(field, 0);
	else if (field->type == ID3_FIELD_TYPE_STRING)
		wstr = id3_field_getstring(field);
	else
		EXCEPTIONSTREAM(ExceptionMediaTag(*this), "Can't handle ID3 field type: (" << field->type << ")");

	if (!wstr)
		return "";

	id3_utf8_t* ustr = id3_ucs4_utf8duplicate(wstr);
	std::string str = (char*)ustr;
	free(ustr);

	return str;
}

std::string ID3::txxx(id3_tag& tag, const char* description) const
{
	for (int i = 0; ; ++i)
	{
		id3_frame* frame = id3_tag_findframe(&tag, "TXXX", i);
		if (!frame)
			return "";

		id3_field* field;
		
		field = id3_frame_field(frame, 0);
		if (!field)
			continue;

		id3_field_textencoding encoding = id3_field_gettextencoding(field);
		if (encoding != ID3_FIELD_TEXTENCODING_UTF_8)
			continue;

		const id3_ucs4_t* wstr = 0;

		field = id3_frame_field(frame, 1);
		if (!field)
			return "";

		// tbd: need to also handle latin1 here?
		if (field->type == ID3_FIELD_TYPE_STRINGLIST)
			wstr = id3_field_getstrings(field, 0);
		else if (field->type == ID3_FIELD_TYPE_STRING)
			wstr = id3_field_getstring(field);

		if (!wstr)
			continue;

		id3_utf8_t* desc_str_utf8 = id3_ucs4_utf8duplicate(wstr);

		bool descMatches = strcmp((char*)desc_str_utf8, description) == 0;

		free(desc_str_utf8);

		if (!descMatches)
			continue;

		wstr = 0;

		field = id3_frame_field(frame, 2);
		if (!field)
			return "";

		// tbd: need to also handle latin1 here?
		if (field->type == ID3_FIELD_TYPE_STRINGLIST)
			wstr = id3_field_getstrings(field, 0);
		else if (field->type == ID3_FIELD_TYPE_STRING)
			wstr = id3_field_getstring(field);

		if (!wstr)
			continue;

		id3_utf8_t* ustr = id3_ucs4_utf8duplicate(wstr);
		std::string str = (char*)ustr;
		free(ustr);

		return str;
	}

	return "";
}

float ID3::volumeNormalisationTrack() const
{
	return _volumeNormalisationTrack;
}

bool ID3::hasVolumeNormalisationTrack() const
{
	return _hasVolumeNormalisationTrack;
}

std::string ID3::filePath() const
{
	return _fname;
}
