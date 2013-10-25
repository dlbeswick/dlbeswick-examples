#include "pch.h"
#include "ObjectComposite.h"

IMPLEMENT_RTTI(ObjectComposite);

void ObjectComposite::composite(Object& obj)
{
	if (_components.contains(&obj))
		EXCEPTIONSTREAM(ExceptionObjectComposite(*this), "CompositeObject already contains the component.");

	obj.setParent(this);
	_components.add(&obj);

	if (!_master)
		setMaster(obj);
}

PtrGC<Object> ObjectComposite::master() const
{
	if (_master)
		return _master;

	return 0;
}

void ObjectComposite::setMaster(Object& component)
{
	if (!_components.contains(&component))
		EXCEPTIONSTREAM(ExceptionObjectComposite(*this), "Supplied master object is not a component of the CompositeObject.");

	if (_master)
		_master->onXFormDirty.remove(this);

	_master = &component;

	_master->onXFormDirty.add(*this, &ObjectComposite::onMasterXFormDirty);
	_master->setParent(parent());
}

void ObjectComposite::onMasterXFormDirty()
{
	for (ObjectList::iterator i = _components.begin(); i != _components.end(); ++i)
	{
		if (*i != _master)
			(*i)->xformDirty();
	}
}

const Matrix4& ObjectComposite::worldXForm() const
{
	if (!master())
		return Matrix4::IDENTITY;

	return _master->worldXForm();
}

const Matrix4& ObjectComposite::worldBaseXForm() const
{
	if (!master())
		return Matrix4::IDENTITY;

	return master()->worldBaseXForm();
}

const Point3& ObjectComposite::worldPos() const
{
	if (!master())
		return Point3::ZERO;

	return master()->worldPos();
}

const Quat& ObjectComposite::worldRot() const
{
	if (!master())
		return Quat::IDENTITY;

	return master()->worldRot();
}

const Point3& ObjectComposite::worldScale() const
{
	if (!master())
		return Point3::ZERO;

	return master()->worldScale();
}

void ObjectComposite::setParent(const PtrGC<Object>& p)
{
	Super::setParent(p);
	_master->setParent(p);
}

void ObjectComposite::setParent(Level& l)
{
	Super::setParent(l);
}

void ObjectComposite::setPos(const Point3& pos)
{
	if (master())
		master()->setPos(pos);
}

void ObjectComposite::setRot(const Quat& rot)
{
	if (master())
		master()->setRot(rot);
}

void ObjectComposite::setScale(const Point3& scale)
{
	if (master())
		master()->setScale(scale);
}

void ObjectComposite::setPivot(const Point3& pivot)
{
	if (master())
		master()->setPivot(pivot);
}


