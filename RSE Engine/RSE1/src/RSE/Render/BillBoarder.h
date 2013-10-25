// ---------------------------------------------------------------------------------------------------------
// 
// SBillBoarder
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_SBILLBOARDER_H
#define RSE_SBILLBOARDER_H

#include "RSE/RSE.h"
#include "Standard/Math.h"
#include "Standard/Singleton.h"
class Camera;

class RSE_API SBillBoarder : public Singleton<SBillBoarder>
{
public:
	SBillBoarder();

	void doXForm(const Point3 &translation = Point3(0, 0, 0));

	void setWorldCam(Camera *pCam)		{ m_pWorldCam = pCam; }
	Camera &getWorldCam()				{ return *m_pWorldCam; }

private:
	Camera	*m_pWorldCam;
};

SINGLETON(BillBoarder);


#endif