

#include "transformation.hh"

using namespace NFcore;


NFcore::Transformation::Transformation()
{
	type=Transformation::SKIP;
	newStateValue = -1;
	stateORsiteIndex=0;
	otherReactantIndex=0;
	otherMappingIndex=0;
}
NFcore::Transformation::~Transformation()
{
	type=Transformation::SKIP;
	newStateValue = -1;
	stateORsiteIndex=0;
	otherReactantIndex=0;
	otherMappingIndex=0;
}





NFcore::Transformation * Transformation::genEmptyTransform()
{
	return new Transformation();
}
NFcore::Transformation * Transformation::genStateChangeTransform(unsigned int stateIndex, int newStateValue)
{
	Transformation *t = new Transformation();
	t->type = Transformation::STATE_CHANGE;
	t->stateORsiteIndex=stateIndex;
	t->newStateValue = newStateValue;
	return t;
}
NFcore::Transformation * NFcore::Transformation::genBindingTransform1(unsigned int bSiteIndex, unsigned int otherReactantIndex, unsigned int otherMappingIndex)
{	
	Transformation *t = new Transformation();
	t->type = Transformation::BINDING;
	t->stateORsiteIndex=bSiteIndex;
	t->otherReactantIndex = otherReactantIndex;
	t->otherMappingIndex = otherMappingIndex;
	return t;
}
NFcore::Transformation * NFcore::Transformation::genBindingTransform2(unsigned int bSiteIndex)
{
	Transformation *t = new Transformation();
	t->type = Transformation::SKIP;
	t->stateORsiteIndex=bSiteIndex;
	return t;
}
NFcore::Transformation * NFcore::Transformation::genUnbindingTransform(unsigned int bSiteIndex)
{
	Transformation *t = new Transformation();
	t->type = Transformation::UNBINDING;
	t->stateORsiteIndex=bSiteIndex;
	return t;
}
NFcore::Transformation * NFcore::Transformation::genAddMoleculeTransform()
{
	cerr<<"Warning genAddMoleculeTransform() not yet working!!"<<endl;
	return 0;
}
NFcore::Transformation * NFcore::Transformation::genRemoveMoleculeTransform()
{
	cerr<<"Warning genRemoveMoleculeTransform() not yet working!!"<<endl;
	return 0;
}
