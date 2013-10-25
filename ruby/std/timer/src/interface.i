%module timer 

%{
#undef read
#undef write
#undef close

#undef T_DATA
#include "Standard/api.h"
#include "Standard/Timer.h"
%}

class Timer
{
public:
	Timer(double frameClamp = 0.2);

	void setFrameClamp(double d);

	void restart();

	void endFrame();

	void setScale(double d);

	void advance(double d);

	double realTime() const;

	double time() const;

	double frame() const;
	double realFrame() const;
	double scale() const;
};