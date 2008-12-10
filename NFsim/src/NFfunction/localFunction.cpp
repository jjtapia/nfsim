/*
 * localFunction.cpp
 *
 *  Created on: Dec 4, 2008
 *      Author: msneddon
 */


#include "NFfunction.hh"






using namespace std;
using namespace NFcore;
using namespace mu;











string LocalFunction::getName() const {
	return this->name;
}
string LocalFunction::getNiceName() const {
	return nicename;
}
string LocalFunction::getExpression() const {
	return originalExpression;
}
string LocalFunction::getParsedExpression() const {
	return parsedExpression;
}



LocalFunction::LocalFunction(System *s,
					string name,
					string originalExpression,
					string parsedExpression,
					vector <string> &args,
					vector <string> &varRefNames,
					vector <string> &varObservableNames,
					vector <Observable *> & varObservables,
					vector <int> &varRefScope,
					vector <string> paramNames)
{
	cout<<"Attempting to create local function: "<<name<<endl;

	//Some checks to make sure we are doing things ok
	if(args.size()>1) {
		cerr<<"For efficiency, local functions currently support a maximum of 1 argument."<<endl;
		cerr<<"Quitting now."<<endl;
		exit(1);
	}
	if(args.size()<1) {
		cerr<<"When creating a local Function, ERROR!! there were no args, so it is a global function."<<endl;
		cerr<<"Quitting now."<<endl;
		exit(1);
	}

	//Do the basics first...
	this->name = name;
	this->originalExpression=originalExpression;
	this->parsedExpression=parsedExpression;


	//Move the vectors into our neat little arrays
	this->n_args=args.size();
	this->argNames = new string[n_args];
	for(unsigned int i=0; i<n_args; i++) {
		this->argNames[i]=args.at(i);
	}

	this->n_varRefs=varRefNames.size();
	this->varRefNames = new string[n_varRefs];
	this->varObservableNames = new string[n_varRefs];
	this->varLocalObservables = new Observable *[n_varRefs];
	this->varRefScope = new int[n_varRefs];
	for(unsigned int i=0; i<n_varRefs; i++) {
		this->varRefNames[i] = varRefNames.at(i);
		this->varObservableNames[i] = varObservableNames.at(i);
		this->varLocalObservables[i] = varObservables.at(i);
		this->varRefScope[i] = varRefScope.at(i);
	}

	this->n_params=paramNames.size();
	this->paramNames = new string[n_params];
	for(unsigned int i=0; i<n_params; i++) {
		this->paramNames[i] = paramNames.at(i);
	}

	//now assemble the nicename
	nicename = this->name + "(";
	for(unsigned int i=0;i<n_args; i++) {
		if(i==0) nicename+=argNames[i];
		else nicename+=","+argNames[i];
	}
	nicename+=")";

	p=0;


	//Identify the type II molecules - those molecules that when changed
	//cause this function to change
	vector <TemplateMolecule *> tmList;
	vector <MoleculeType *> addedMoleculeTypes;

	cout<<"Now remembering type II molecules..."<<endl;
	bool hasAdded = false;
	for(unsigned int i=0; i<n_varRefs; i++) {
		if(varRefScope[i]==-1) {//handle global scopes
			TemplateMolecule::traverse(s->getObservableByName(this->varObservableNames[i])->getTemplateMolecule(),tmList);
		} else {
			TemplateMolecule::traverse(this->varLocalObservables[i]->getTemplateMolecule(),tmList);
		}
		//cout<<"traversed obs "<<i<<" and found: "<<tmList.size()<<" templates\n";

		for(unsigned int t=0; t<tmList.size(); t++) {
			MoleculeType *mt = tmList.at(t)->getMoleculeType();

			//Make sure we haven't added this molecule type before
			hasAdded = false;
			for(unsigned int m=0; m<addedMoleculeTypes.size(); m++) {
				if(addedMoleculeTypes.at(m)==mt) {
					hasAdded=true; break;
				}
			}
			if(!hasAdded) {
				addedMoleculeTypes.push_back(mt);
			//	cout<<"remembering: "<<mt->getName()<<endl;
			}
			//else {
			//	cout<<"ignoring: "<<mt->getName()<<endl;
			//}
		}
		tmList.clear();
	}

	//Now actually remember them (and make sure they remember us...)
	for(unsigned int m=0; m<addedMoleculeTypes.size(); m++) {
		int index = addedMoleculeTypes.at(m)->addLocalFunc_TypeII(this);
		this->typeII_mol.push_back(addedMoleculeTypes.at(m));
		this->typeII_localFunctionIndex.push_back(index);
	}

}










void LocalFunction::prepareForSimulation(System *s) {

	cout<<"preparing local function: "<<this->nicename<<endl;

	//Finally, we can create the local function
	try {
		p=FuncFactory::create();

		//Give the local observable to the function so it can be used
		for(unsigned int i=0; i<n_varRefs; i++) {
			if(this->varRefScope[i]==-1) { //for global variables, use the global observable
				s->getObservableByName(this->varObservableNames[i])->addReferenceToMyself(this->varRefNames[i],p);
			} else { //for local observables, use this function's observable
				this->varLocalObservables[i]->addReferenceToMyself(this->varRefNames[i],p);
			}
		}

		//Point the function to the counter
//		for(unsigned int i=0; i<n_sc; i++) {
//			p->DefineVar(sc[i]->name,&(sc[i]->value));
//		}
//
		//Set the constant values
		for(unsigned int i=0; i<this->n_params; i++) {
			p->DefineConst(this->paramNames[i],s->getParameter(paramNames[i]));
		}

		//Finally, we can set the expression
		p->SetExpr(this->parsedExpression);

	//Catch anything that goes astray
	} catch (mu::Parser::exception_type &e) {
		cout<<"Error creating local function "<<name<<" in class LocalFunction!!  This is what happened:"<<endl;
		cout<< "  "<<e.GetMsg() << endl;
		cout<<"Quitting."<<endl;
		exit(1);
	}
}


double LocalFunction::getValue(Molecule *m, int scope)
{
//	cout<<"getting local function value: "<<this->nicename<<endl;
//	cout<<"using molecule: "<<m->getUniqueID()<<" with scope: "<<scope<<endl;

	int SPECIES = 0;
	int MOLECULE = 1;

	if(scope==SPECIES) {
//			cout<<"Species scope"<<endl;
//
			for(unsigned int ti=0; ti<typeI_mol.size(); ti++) {
				if(m->getMoleculeType()==typeI_mol.at(ti)) {
					return m->getLocalFunctionValue(typeI_localFunctionIndex.at(ti));
				}
			}

	} else if(scope==MOLECULE) {
	//		cout<<"Molecule scope."<<endl;

			//For each of our variables
			for(unsigned int i=0; i<n_varRefs; i++) {
				if(varLocalObservables[i]!=0) {   //If it is local
					varLocalObservables[i]->clear();  //clear it first
					if(varLocalObservables[i]->isObservable(m)) {  //recompute observable
						varLocalObservables[i]->straightAdd();
					}
				}
			}


			//Recalculate the function
			double newValue = FuncFactory::Eval(p);

			return newValue;


		} else {
			cout<<"Internal error in LocalFunction::evaluateOn()! trying to evaluate a function with unknown scope."<<endl;
			exit(1);

		}



}



//This only accepts one molecule, because there can only be one argument
//if we can have multiple arguments, this must be extended to have an
//array of molecules (as in a composite function evaluation)
double LocalFunction::evaluateOn(Molecule *m, int scope) {

	//cout<<"evaluating local function: "<<this->nicename<<endl;
	//this->printDetails(m->getMoleculeType()->getSystem());
	//cout<<"using molecule: "<<m->getUniqueID()<<" with scope: "<<scope<<endl;

	if(scope==LocalFunction::SPECIES) {
		//cout<<"evaluating on Species scope"<<endl;

		list <Molecule *> molList;
		list <Molecule *>::iterator molIter;
		m->traverseBondedNeighborhood(molList,ReactionClass::NO_LIMIT);


		//First, clear out the observables
		for(unsigned int i=0; i<n_varRefs; i++) {
			if(varLocalObservables[i]!=0) {
				varLocalObservables[i]->clear();
			}
		}


		//recompute the observables
		for(molIter=molList.begin(); molIter!=molList.end(); molIter++) {
			for(unsigned int i=0; i<n_varRefs; i++) {
				if(varLocalObservables[i]!=0) {   //If it is local
					if(varLocalObservables[i]->isObservable((*molIter))) {  //recompute observable
						varLocalObservables[i]->straightAdd();
					}
				}
			}
		}

		//evaluate the function
		double newValue = FuncFactory::Eval(p);


		//Here we have to notify the type I molecules that this function has changed
		//Update the molecules (Type I) that needed this function evaluated...
		for(molIter=molList.begin(); molIter!=molList.end(); molIter++) {
			for(unsigned int ti=0; ti<typeI_mol.size(); ti++) {
				if((*molIter)->getMoleculeType()==typeI_mol.at(ti)) {
					(*molIter)->setLocalFunctionValue(newValue,this->typeI_localFunctionIndex.at(ti));
					(*molIter)->updateDORRxnValues();
				}
			}
		}

		return newValue;

	} else if(scope==LocalFunction::MOLECULE) {
		//cout<<"evaluating on Molecule scope."<<endl;

		//For each of our variables
		for(unsigned int i=0; i<n_varRefs; i++) {
			if(varLocalObservables[i]!=0) {   //If it is local
				varLocalObservables[i]->clear();  //clear it first
				if(varLocalObservables[i]->isObservable(m)) {  //recompute observable
					varLocalObservables[i]->straightAdd();
				}
			}
		}


		//Recalculate the function
		double newValue = FuncFactory::Eval(p);

		//Update the function values
		for(unsigned int ti=0; ti<typeI_mol.size(); ti++) {
			if(m->getMoleculeType()==typeI_mol.at(ti)) {
				m->setLocalFunctionValue(newValue,this->typeI_localFunctionIndex.at(ti));
				m->updateDORRxnValues();
			}
		}

	//	this->printDetails(m->getMoleculeType()->getSystem());
		return newValue;


	} else {
		cout<<"Internal error in LocalFunction::evaluateOn()! trying to evaluate a function with unknown scope."<<endl;
		exit(1);

	}






//	if(evaluationLevel==0) {
//		list <Molecule *> molList;
//		list <Molecule *>::iterator molIter;
//
//		//Get the species.  If we are using complex bookkeeping, we should take advantage
//		//of that here, although I don't here yet.
//		m->traverseBondedNeighborhood(molList,ReactionClass::NO_LIMIT);
//		for(unsigned int i=0; i<n_obs; i++) obs[i]->clear();
//		for(unsigned int i=0; i<n_sc; i++) sc[i]->reset();
//
//		//Update the observables and counters, as something has changed
//		for(molIter=molList.begin(); molIter!=molList.end(); molIter++) {
//			for(unsigned int i=0; i<n_obs; i++)
//				if(obs[i]->isObservable(*molIter)) obs[i]->straightAdd();
//			for(unsigned int i=0; i<n_sc; i++) {
//				//cout<<"adding to sc"<<endl;
//				sc[i]->add(*molIter);
//			}
//		}
//
//		//evaluate the function
//		double newValue = FuncFactory::Eval(p);
//
//		//Update the molecules (Type I) that needed this function evaluated...
//		for(molIter=molList.begin(); molIter!=molList.end(); molIter++) {
//			for(unsigned int ti=0; ti<typeI_mol.size(); ti++) {
//				if((*molIter)->getMoleculeType()==typeI_mol.at(ti)) {
//					(*molIter)->setLocalFunctionValue(newValue,this->typeI_localFunctionIndex.at(ti));
//					(*molIter)->updateDORRxnValues();
//				}
//			}
//		}
//		return newValue;
//
//	//Evaluation level of 1 means that we search only this molecule when
//	//evaluating the function - the steps are the same as for the entire species, except we evaluate
//	//only on the molecule
//	} else if(evaluationLevel==1) {
//
//
//		for(unsigned int i=0; i<n_obs; i++) obs[i]->clear();
//		for(unsigned int i=0; i<n_sc; i++) sc[i]->reset();
//
//		for(unsigned int i=0; i<n_obs; i++) {
//			if(obs[i]->isObservable(m)) obs[i]->straightAdd();
//		}
//
//
//
//		for(unsigned int i=0; i<n_sc; i++) {
//			sc[i]->add(m);
//		}
//
//		double newValue = FuncFactory::Eval(p);
//		for(unsigned int ti=0; ti<typeI_mol.size(); ti++) {
//			if(m->getMoleculeType()==typeI_mol.at(ti)) {
//				m->setLocalFunctionValue(newValue,this->typeI_localFunctionIndex.at(ti));
//				m->updateDORRxnValues();
//			}
//		}
//		return newValue;
//
//
//	} else {
//		cout<<"Internal error in LocalFunction::evaluateOn()! trying to evaluate a function with a bad evaluation level."<<endl;
//		exit(1);
//	}
//
//	//Should never get here...
//	return 0;



	return -1;
}



LocalFunction::~LocalFunction() {
/*	for(unsigned int i=0; i<n_obs; i++)
		delete obs[i];
	delete [] obs;

	for(unsigned int i=0; i<n_sc; i++)
		delete sc[i];
	delete [] sc;

	delete [] paramNames;
	delete [] paramValues;*/
	if(p!=NULL) delete p;
}




/*void LocalFunction::setEvaluationLevel(int eLevel) {

	if(eLevel<0 || eLevel>1) {
		cout<<"Error when setting evaluation level of function: "<<getNiceName();
		cout<<"\nEvaluation level given was:"<<eLevel<<" but currently only supports levels of 0 or 1."<<endl;
		exit(1);
	}

	this->evaluationLevel = eLevel;
}*/


//This function is generally called by a DOR reaction class once the
//DOR reaction class has established that the value of this function
//is required for some moleculetype...
void LocalFunction::addTypeIMoleculeDependency(MoleculeType *mt) {

	//First, make sure we haven't added this bad boy yet
	for(unsigned int i=0; i<this->typeI_mol.size(); i++) {
		if(typeI_mol.at(i)==mt) return;
	}

	//First, add myself to the moleculeType
	int index = mt->addLocalFunc_TypeI(this);
	this->typeI_mol.push_back(mt);
	this->typeI_localFunctionIndex.push_back(index);
}

/*
int LocalFunction::getIndexOfTypeIFunctionValue(Molecule *m) {

	for(unsigned int i=0; i<this->typeI_mol.size(); i++) {
		if(typeI_mol.at(i)==m->getMoleculeType()) return this->typeI_localFunctionIndex.at(i);
	}
	cout<<"Error when getting the index of a Type I function value in LocalFunction:"<<endl;
	cout<<"Could not find the molecule type: '"<<m->getMoleculeType()->getName()<<"' as a type I molecule of this function: "<<this->getNiceName()<<endl;
	exit(1);
}
*/


void LocalFunction::updateParameters(System *s)
{
	cout<<"Updating parameters for function: "<<name<<endl;
	for(unsigned int i=0; i<n_params; i++) {
		p->DefineConst(paramNames[i],s->getParameter(paramNames[i]));
	}
}

void LocalFunction::printDetails(System *s)
{
	cout<<"Local Function: "+this->nicename+"\n";
	cout<<" = "<<this->originalExpression<<endl;
	cout<<" parsed expression = "<<this->parsedExpression<<endl;

	cout<<"   -Variable References:"<<endl;
	for(unsigned int i=0; i<n_varRefs; i++) {
		if(varRefScope[i]==-1) {
			cout<<"         "<<varObservableNames[i]<<" (scope=global): ";
			cout<<s->getObservableByName(varRefNames[i])->getCount()<<endl;
		} else {
			cout<<"         "<<varObservableNames[i]<<" (scope=";
			cout<<argNames[varRefScope[i]]<<") last evaluated to: ";
			cout<<varLocalObservables[i]->getCount()<<endl;
		}
	}

	if(n_params>0) {
		cout<<"   -Constant Parameters:"<<endl;
		for(unsigned int i=0; i<n_params; i++) {
			cout<<"         "<<paramNames[i]<<" = " << s->getParameter(paramNames[i])<<endl;
		}
	}

	cout<<"   -Type II Molecules (this function depends on these molecules):"<<endl;
	for(unsigned int i=0; i<typeII_mol.size(); i++) {
		cout<<"         "<<typeII_mol.at(i)->getName()<<endl;
	}

	cout<<"   -Type I Molecules (molecules in a dor rxn that depend on this function):"<<endl;
	for(unsigned int i=0; i<typeI_mol.size(); i++) {
		cout<<"         "<<typeI_mol.at(i)->getName()<<endl;
	}


	if(p!=0)
		cout<<"   Function last evaluated to: "<<FuncFactory::Eval(p)<<endl;
}
