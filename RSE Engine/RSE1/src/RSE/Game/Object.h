#ifndef RSE1_OBJECT_H
#define RSE1_OBJECT_H

#include "RSE/UI/Controls/EditableProperties.h"
#include "Standard/Exception/StreamRTTI.h"
#include "Standard/MultiDelegate.h"
#include "Standard/PendingList.h"
#include "Standard/PtrGCHost.h"

class RSE_API ExceptionObject : public ExceptionStreamRTTI
{
public:
	ExceptionObject(class Object& who);
};

// Object
// Base class for all entites that maintain a position in the world.
// TBD: make class abstract, move variable regarding position to a derived convenience class if necessary.
class RSE_API Object : public EditableProperties, public PtrGCHost
{
	USE_RTTI(Object, Base);

public:
	typedef PendingListNullRemover<PtrGC<Object> > ObjectList;
	typedef TMultiDelegate<Object> ObjectMultiDelegate;

	Object();
	virtual ~Object();

	virtual void update(float delta);
	virtual void draw();

	virtual const std::string& name() const;
	virtual void setName(const std::string& n);

	virtual bool visible() const;
	virtual void setVisible(bool b);

	virtual const PtrGC<Object>& parent() const;
	virtual void setParent(const PtrGC<Object>& p);
	virtual void setParent(class Level& l);
	virtual const ObjectList& children() const;

	virtual const Point3& pos() const;
	virtual const Quat& rot() const;
	virtual const Point3& pivot() const;
	virtual const Point3& scale() const;
	virtual void setPos(const Point3& pos);
	virtual void setRot(const Quat& rot);
	virtual void setScale(const Point3& scale);
	virtual void setPivot(const Point3& pivot);

	void xformDirty() const;
	bool xformIsDirty() const;
	virtual const Matrix4& worldXForm() const;
	virtual const Matrix4& worldBaseXForm() const;
	virtual const Point3& worldPos() const;
	virtual const Quat& worldRot() const;
	virtual const Point3& worldScale() const;

	Level& level() const;
	virtual bool hasLevel() const;
	virtual void setLevel(Level* l);

	virtual Point3 extent() const;
	virtual Point3 geomCenter() const;

	ObjectMultiDelegate onXFormDirty;

protected:
	virtual bool needPostNewObjectConstructCall() const;

	// Called when setParent is invoked on the Object.
	virtual void constructObject(Object& parent);

	// Called after constructObject. The object is guaranteed to have a parent at that point.
	virtual void constructed();

	virtual void addChild(const PtrGC<Object>& p);
	virtual void removeChild(const PtrGC<Object>& p);

	virtual void levelChanging(Level* l);

	bool _constructedObject;
	std::string	m_name;
	bool m_bVisible;
	PtrGC<Object> m_pParent;
	ObjectList m_children;

private:
	static std::string		_unnamed;
	Level*					m_level;
	Point3					m_pos;
	Point3					m_pivot;
	Point3					m_scale;
	Quat					m_rot;
	mutable Dirty<Point3>	m_worldPos;
	mutable Dirty<Quat>		m_worldRot;
	mutable Dirty<Point3>	m_worldScale;
	mutable Dirty<Matrix4>	m_worldXForm;
	mutable Dirty<Matrix4>	m_worldBaseXForm;
};

#endif
