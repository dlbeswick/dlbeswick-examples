#ifndef RSE_PATHRESOURCE_H
#define RSE_PATHRESOURCE_H

#include <Standard/Path.h>

class RSE_API PathResource : public Path
{
public:
	PathResource(const std::string& stringRep = "");
};

#endif