// ---------------------------------------------------------------------------------------------------------
// 
// EditableProperties
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "EditableProperties.h"
#include "UIPropertyEditor.h"

IMPLEMENT_RTTI(EditableProperties);

void EditableProperties::editableProperties(PropertyEditorList& l)
{
	m_queryingEditable = true;

	std::vector<EditableStreamVar*> editVars;
	editStreamVars(editVars);

	for (std::vector<EditableStreamVar*>::iterator varIt = editVars.begin(); varIt != editVars.end(); ++varIt)
	{
		UIPropertyEditorBase* prop = (*varIt)->editor();
		l.push_back(prop);
	}

	m_queryingEditable = false;
}

void EditableProperties::getStreamVars(StreamVars& v)
{
	Streamable::getStreamVars(v);

	std::vector<EditableStreamVar*> editVars;
	editStreamVars(editVars);

	for (std::vector<EditableStreamVar*>::iterator editVarIt = editVars.begin(); editVarIt != editVars.end(); ++editVarIt)
	{
		v.push_back((*editVarIt)->streamVar());
	}
}

EditableStreamVar::EditableStreamVar(StreamVarBase* streamVar, UIPropertyEditorBase* editor) :
	m_streamVar(streamVar), 
	m_editor(editor)
{
}
