#include "MediaTag.h"
#include <limits>

float MediaTag::extractReplaygainDBValue(const char* fieldContents)
{
	float result;

	if (fieldContents)
	{
		if (sscanf(fieldContents, "%f dB", &result) > 0)
		{
			return result;
		}
	}

	return FLT_MAX;
}

float MediaTag::extractReplaygainDBValue(const std::string& fieldContents)
{
	return extractReplaygainDBValue(fieldContents.c_str());
}
