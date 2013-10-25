#ifndef RSE1_OBJECTCOMPOSITE_H
#define RSE1_OBJECTCOMPOSITE_H

#include "RSE/Game/Object.h"
#include "Standard/ExceptionStream.h"

class RSE_API ExceptionObjectComposite : public ExceptionObject
{
public:
	ExceptionObjectComposite(Object& who) : ExceptionObject(who) {}
};

class RSE_API ObjectComposite : public Object
{
	USE_RTTI(ObjectComposite, Object);
public:
	virtual void composite(Object& obj);

	virtual PtrGC<Object> master() const;
	virtual void setMaster(Object& component);

	virtual void setParent(const PtrGC<Object>& p);
	virtual void setParent(Level& l);

	virtual void setPos(const Point3& pos);
	virtual void setRot(const Quat& rot);
	virtual void setScale(const Point3& scale);
	virtual void setPivot(const Point3& pivot);

	virtual const Matrix4& worldXForm() const;
	virtual const Matrix4& worldBaseXForm() const;
	virtual const Point3& worldPos() const;
	virtual const Quat& worldRot() const;
	virtual const Point3& worldScale() const;

protected:
	virtual void onMasterXFormDirty();

	ObjectList _components;

private:
	PtrGC<Object> _master;
};

#endif
