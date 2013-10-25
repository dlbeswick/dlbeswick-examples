// ------------------------------------------------------------------------------------------------
//
// DlgObjProperties
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgObjProperties.h"
#include "UI/UILayout.h"
#include "UI/Controls/UIPropertyEditor.h"

IMPLEMENT_RTTI(DlgObjProperties);

PendingListNullRemover<PtrGC<DlgObjProperties> > DlgObjProperties::m_dlgs;

class FindTitle : public std::binary_function<PtrGC<UIPropertyEditorBase>, std::string, bool>
{
public:
	result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const
	{
		return lhs->title() == rhs;
	}
};


DlgObjProperties::DlgObjProperties()
{
}

DlgObjProperties::DlgObjProperties(PropertyEditorList& l)
{
	set(l);
}

void DlgObjProperties::construct()
{
	setFont("smallarial");
	m_bNeedLayout = false;
	m_dlgs.add(this);
	m_series = 0;
	
	m_layout = new UILayoutTable;
	m_layout->setCols(2);
	m_layout->setCol(0, 0.35f);
	m_layout->setCol(1, 0.65f);
	m_layout->cellAlign = ALIGN_LEFT;

	Super::construct();

	addChild(m_layout);
}

void DlgObjProperties::concatenate(PropertyEditorList& l)
{
	for (uint i = 0; i < l.size(); ++i)
		addProp(l[i]);
}

void DlgObjProperties::set(PropertyEditorList& l)
{
	clear();
	concatenate(l);
}

void DlgObjProperties::addProp(const PtrGC<UIPropertyEditorBase>& p)
{
	p->setOnModify(delegate(&DlgObjProperties::internalOnCommit));
	m_props.push_back(p);
	
	UITextBox* label = new UITextBox(p->title());
	label->setEnabled(false);
	label->fitToText();
	m_labels.push_back(label);

	m_bNeedLayout = true;
}

void DlgObjProperties::internalOnCommit(UIPropertyEditorBase& what)
{
	what.commit();

	onCommit();

	what.refresh();
}

void DlgObjProperties::postEditorCommit(UIPropertyEditorBase& what)
{
}

void DlgObjProperties::layoutProps(const Point2& reqSize)
{
	// add property editors to layout element
	for (uint i = 0; i < m_props.size(); ++i)
	{
		m_layout->addChild(m_labels[i]);
		m_labels[i]->setHelp(m_props[i]->help());
		m_props[i]->fitToChildren();
		m_layout->addChild(m_props[i]);
	}

	// force layout recalc. this must be done in order to find an accurate size.
	m_layout->forceRecalc();

	// size client area to property editors
	float y = m_layout->size().y;

	clientArea().setSize(Point2(reqSize.x, std::max(y, reqSize.y)));
	m_bNeedLayout = false;
	refresh();
}

void DlgObjProperties::clear()
{
	for (uint i = 0; i < m_props.size(); ++i)
	{
		m_props[i].destroy();	
		PtrGC<UIElement>(m_labels[i]).destroy();	
	}
	
	m_props.clear();
	m_labels.clear();
}

void DlgObjProperties::refresh()
{
	for (uint i = 0; i < m_props.size(); ++i)
	{
		m_props[i]->refresh();
	}
}

void DlgObjProperties::draw()
{
	if (m_bNeedLayout)
	{
		layoutProps(size());
	}

	Dialog::draw();
}

void DlgObjProperties::setSize(const Point2& reqSize)
{
	layoutProps(reqSize);
	Super::setSize(reqSize);
}

class DlgObjProperties::FunctorMaxSeries : public std::binary_function<PtrGC<DlgObjProperties>, int, void>
{
public:
	result_type operator () (first_argument_type& lhs, second_argument_type& rhs)
	{
		if (rhs + 1 == lhs->m_series)
			++rhs;
	}
};

PtrGC<DlgObjProperties> DlgObjProperties::incrementName() const
{
	int maxSeries = 0;
	
	std::vector<PtrGC<DlgObjProperties> > x;
	std::for_each(x.begin(), x.end(), refbind2nd(FunctorMaxSeries(), maxSeries));

	DlgObjProperties* d = new DlgObjProperties;
	d->m_series = maxSeries + 1;
	d->setName(name() + std::string(" ") + d->m_series);
	
	return d;
}

bool DlgObjProperties::changed(const std::string& propTitle) const
{
	PropertyEditorList::const_iterator i = std::find_if(m_props.begin(), m_props.end(), refbind2nd(FindTitle(), propTitle));
	if (i == m_props.end())
		return false;

	return (*i)->changed();
}
