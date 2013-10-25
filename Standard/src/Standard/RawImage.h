// ------------------------------------------------------------------------------------------------
//
// RawImage
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Image.h"


class RawImage : public Image
{
public:
	virtual void create(int width, int height, COMMONFORMAT format)
	{
		m_size.x = width;
		m_size.y = height;

		Image::create(width, height, format);
	}

	virtual void read(const std::string& description, ibinstream& s) {};
	virtual void write(obinstream& s) const {};
	virtual DMath::Point2i size() const { return m_size; }

protected:
	Point2i m_size;
};