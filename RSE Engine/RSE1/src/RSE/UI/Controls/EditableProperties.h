// ---------------------------------------------------------------------------------------------------------
// 
// Classes and interfaces for exposing objects to the "object property editor"
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/RSE.h"
#include "Standard/Base.h"
#include "Standard/PtrGC.h"

class UIPropertyEditorBase;

typedef std::vector<PtrGC<UIPropertyEditorBase> > PropertyEditorList;

// Interface for editable classes
class RSE_API EditableProperties : public Base
{
	USE_RTTI(EditableProperties, Base);
public:
	EditableProperties() :
	  m_queryingEditable(false)
	{}

	virtual void editableProperties(PropertyEditorList& l);

	virtual void onEditableCommit()
	{
	}

protected:
	virtual bool isQueryingEditable() const { return m_queryingEditable; }

	virtual void editStreamVars(std::vector<class EditableStreamVar*>& e)
	{
	}

	virtual void getStreamVars(StreamVars& v);

	void editableFromStreamable(PropertyEditorList& l);

	bool m_queryingEditable;
};

// An object that holds a reference to either a stream var or an editable var. Either one or the other will be null.
// Intended to reduce duplication between streaming and Editable Properties definitions.
class RSE_API EditableStreamVar
{
public:
	EditableStreamVar(StreamVarBase* streamVar, UIPropertyEditorBase* editor);

	UIPropertyEditorBase* editor() { return m_editor; }
	StreamVarBase* streamVar() { return m_streamVar; }

protected:
	StreamVarBase* m_streamVar;
	UIPropertyEditorBase* m_editor;
};

template <class T> EditableStreamVar* makeEditableStreamVar(bool isQueryingEditable, const std::string& name, T& var, const std::string& textVarName, const std::string& help) 
{ 
	if (isQueryingEditable)
		return new EditableStreamVar(0, property(name, var, help)); 
	else
		return new EditableStreamVar(makeStreamVar(textVarName, var), 0); 
}

#define EDITSTREAMVAR(var, help)				e.push_back(makeEditableStreamVar(isQueryingEditable(), #var, var, #var, help))
#define EDITSTREAMNAME(varname, var, help)		e.push_back(makeEditableStreamVar(isQueryingEditable(), varname, var, #var, help))

// Only valid within EDITABLE block.
#define EDITPROP(variable)	l.push_back(variable) // supply a UIEditProperty

// helpers
#define USE_EDITSTREAM \
	protected: virtual void editStreamVars(std::vector<EditableStreamVar*>& e);
#define EDITSTREAM \
	void METADATA::editStreamVars(std::vector<EditableStreamVar*>& e) \
	{ \
		Super::editStreamVars(e);
#define END_EDITSTREAM \
	}

#define USE_EDITABLE \
	public: virtual void editableProperties(PropertyEditorList& l);
#define EDITABLE \
	void METADATA::editableProperties(PropertyEditorList& l) \
	{ \
		Super::editableProperties(l);
#define END_EDITABLE \
	}
