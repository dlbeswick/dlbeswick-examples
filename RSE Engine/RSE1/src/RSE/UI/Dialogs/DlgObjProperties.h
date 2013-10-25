// ------------------------------------------------------------------------------------------------
//
// DlgObjProperties
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE1_DLGOBJPROPERTIES_H
#define RSE1_DLGOBJPROPERTIES_H

#include "UI/Dialog.h"
#include "UI/Controls/EditableProperties.h"

// A dialog for editing a list of property editors. If the object to whom the property editors refer
// can be deleted arbitrarily, then consider DlgObjPropertiesPtrGC.
class RSE_API DlgObjProperties : public Dialog
{
	USE_RTTI(DlgObjProperties, Dialog);
public:
	DlgObjProperties();
	DlgObjProperties(PropertyEditorList& l);

	virtual void setSize(const Point2& reqSize);

	void addProp(const PtrGC<UIPropertyEditorBase>& p);
	void concatenate(PropertyEditorList& l);
	void clear();
	void refresh();
	void set(PropertyEditorList& l);

	bool changed(const std::string& propTitle) const;

	Delegate onCommit;

	const PropertyEditorList& properties() { return m_props; }

	PtrGC<DlgObjProperties> incrementName() const; // create a new dialog with the name incremented

protected:
	class FunctorMaxSeries;

	virtual void construct();

	virtual void draw();

	void internalOnCommit(UIPropertyEditorBase& what);
	virtual void postEditorCommit(UIPropertyEditorBase& what);
	void layoutProps(const Point2& reqSize);

	PropertyEditorList m_props;
	std::vector<UIElement*> m_labels;
	bool m_bNeedLayout;
	static PendingListNullRemover<PtrGC<DlgObjProperties> > m_dlgs;
	int m_series;

	stdext::hash_set<std::string> m_changed;

	class UILayoutTable* m_layout;
};

// PtrGCHostType should specify a class that inherits from both PtrGCHost and EditableProperties.
template <class PtrGCHostType>
class DlgObjPropertiesPtrGC : public DlgObjProperties
{
public:
	DlgObjPropertiesPtrGC(const PtrGC<PtrGCHostType>& e = 0)
	{
		if (e)
			set(e);
	}

	// bGetProperties supports legacy code in ParticleSystem editor
	void set(const PtrGC<PtrGCHostType>& e, bool bGetProperties = true)
	{
		m_obj = e;

		if (bGetProperties)
		{
			clear();

			if (m_obj)
			{
				PropertyEditorList l;
				m_obj->editableProperties(l);
				concatenate(l);
			}
		}
	}

	const PtrGC<PtrGCHostType>& obj() 
	{ 
		return m_obj; 
	}

	virtual void postEditorCommit(UIPropertyEditorBase& what)
	{
		if (m_obj)
			m_obj->onEditableCommit();
	}

protected:
	PtrGC<PtrGCHostType> m_obj;
};

#endif
