#ifndef TRANSFORMATION_HH_
#define TRANSFORMATION_HH_


#include "../NFreactions.hh"

namespace NFcore
{

	class SpeciesCreator;
	class Mapping;
	class Transformation;

	class TransformationFactory {
		public:
			/*!
			 	Generates a state change Transformation for transforming the given state at the given
			 	index into the new state value.
			    @author Michael Sneddon
			 */
			static Transformation * genStateChangeTransform(unsigned int cIndex, int newValue);

			/*!
			 	Generates a binding transformation for one of the two binding sites in the binding Transform.  You will
			 	have to tell it where the other Transformation object lives (TransformationSet has this information).
			    @author Michael Sneddon
			 */
			static Transformation * genBindingTransform1(unsigned int bSiteIndex, unsigned int otherReactantIndex, unsigned int otherMappingIndex);
			static Transformation * genBindingSeparateComplexTransform1(unsigned int bSiteIndex, unsigned int otherReactantIndex, unsigned int otherMappingIndex);

			/*!
			 	Generates the second half of a binding transform.  The other site already knows about this site, so all you
			 	need here is the index of the binding site that must be bonded.
			    @author Michael Sneddon
			 */
			static Transformation * genBindingTransform2(unsigned int bSiteIndex);

			/*!
			 	Generates an unbinding transformation for a particular binding site.  Only
			 	one of the binding sites must be specified as a Transformation - the other
			 	is automatically taken care of.
			    @author Michael Sneddon
			 */
			static Transformation * genUnbindingTransform(unsigned int bSiteIndex);

			/*!
			 	Generates an Add Molecule transformation.  Currently, this is not yet
			 	implemented.
			    @author Michael Sneddon
			 */
			static Transformation * genAddMoleculeTransform(SpeciesCreator *sc);

			/*!
			 	Generates a removal of a molecule from the system.  Currently this is
			 	not yet implemented.
			    @author Michael Sneddon
			 */
			static Transformation * genRemoveMoleculeTransform();

			/*!
			 	Generates an empty transformation.  This is used in cases where there is
			 	a reactant that is not transformed in a reaction, but that still needs
			 	to be counted and marked so that the rate of the reaction is correct.
			    @author Michael Sneddon
			 */
			static Transformation * genEmptyTransform();

			/*!
			 	Generates an IncrementState transformation.
			    @author Michael Sneddon
			 */
			static Transformation * genIncrementStateTransform(unsigned int cIndex);

			/*!
			 	Generates an DecrementState transformation.
			    @author Michael Sneddon
			*/
			static Transformation * genDecrementStateTransform(unsigned int cIndex);





			/*!	Indicates a state change transformation or mapping onto a state	*/
			static const unsigned int STATE_CHANGE = 0;

			/*!	Indicates a binding transformation or mapping onto a binding site	*/
			static const unsigned int BINDING = 1;

			/*!	Indicates an unbinding transformation or mapping onto a binding site	*/
			static const unsigned int UNBINDING = 2;

			/*!	Indicates a removal transform or mapping onto an entire Molecule	*/
			static const unsigned int REMOVE = 3;

			/*!	Indicates an addition transform	*/
			static const unsigned int ADD = 4;

			/*!	Indicates no transformation is needed (or is the second partner
			    in a binding transform and so should be skipped when applying transforms	*/
			static const unsigned int EMPTY = 5;

			/*!	*/
			static const unsigned int INCREMENT_STATE = 6;

			/*!		*/
			static const unsigned int DECREMENT_STATE = 7;



			////////////////////////////////
			static Transformation * genLocalFunctionReference(string PointerName, int type, TemplateMolecule *tm);
			static const unsigned int LOCAL_FUNCTION_REFERENCE = 8;
			////////////////////////////////

	};


	class Transformation {
		public:
			Transformation(int type) {this->type=type;};
			virtual ~Transformation() {};
			int getType() const { return type; }
			virtual void apply(Mapping *m, MappingSet **ms) = 0;
			virtual int getComponentIndex() const = 0;
		protected:
			int type;
	};


	class LocalFunctionReference : public Transformation {
		public:
			LocalFunctionReference(string PointerName, int scope, TemplateMolecule *tm);
			virtual ~LocalFunctionReference() {};
			virtual void apply(Mapping *m, MappingSet **ms) {};
			virtual int getComponentIndex() const { return -1; };

			TemplateMolecule *getTemplateObject() const {return tm;};
			string getPointerName() const { return PointerName; };
			int getFunctionScope() const {return scope; };

			static const unsigned int SPECIES_FUNCTION=0;
			static const unsigned int SINGLE_MOLECULE_FUNCTION=1;
		protected:
			string PointerName;
			int scope;
			TemplateMolecule *tm;
	};



	class EmptyTransform : public Transformation {
		public:
			EmptyTransform() : Transformation(TransformationFactory::EMPTY){ this->cIndex=-1; };
			EmptyTransform(int cIndex) : Transformation(TransformationFactory::EMPTY){ this->cIndex=cIndex; };
			virtual ~EmptyTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms) {};
			virtual int getComponentIndex() const { return cIndex; };
		protected:
			int cIndex;
	};


	class StateChangeTransform : public Transformation {
		public:
			StateChangeTransform(int cIndex, int newValue);
			virtual ~StateChangeTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {return cIndex;};
		protected:
			int cIndex;
			int newValue;
	};

	class BindingTransform : public Transformation {
		public:
			BindingTransform(int cIndex, int otherReactantIndex, int otherMappingIndex);
			virtual ~BindingTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {return cIndex;};
		protected:
			int cIndex;
			int otherReactantIndex;
			int otherMappingIndex;
	};

	class BindingSeparateComplexTransform : public BindingTransform {
			public:
				BindingSeparateComplexTransform(int cIndex, int otherReactantIndex, int otherMappingIndex) :
					BindingTransform(cIndex, otherReactantIndex, otherMappingIndex) {};
				virtual ~BindingSeparateComplexTransform() {};
				virtual void apply(Mapping *m, MappingSet **ms);
				virtual int getComponentIndex() const {return cIndex;};
	};

	class UnbindingTransform : public Transformation {
		public:
			UnbindingTransform(int cIndex);
			virtual ~UnbindingTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {return cIndex;};
		protected:
			int cIndex;
	};

	class AddMoleculeTransform : public Transformation {
		public:
			AddMoleculeTransform(SpeciesCreator *sc);
			virtual ~AddMoleculeTransform();
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {cerr<<"You should not get a component index from an AddMoleculeTransform!!"<<endl; return -1;};
		protected:
			SpeciesCreator *sc;
	};

	class RemoveMoleculeTransform : public Transformation {
		public:
			RemoveMoleculeTransform() : Transformation(TransformationFactory::REMOVE) {};
			virtual ~RemoveMoleculeTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {cout<<"You should not get a component index from a RemoveMoleculeTransform!!"<<endl; exit(1); return -1;};
	};




	class IncrementStateTransform : public Transformation {
		public:
			IncrementStateTransform(unsigned int stateIndex);
			virtual ~IncrementStateTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {return cIndex;};
		protected:
			int cIndex;
	};

	class DecrementStateTransform : public Transformation {
		public:
			DecrementStateTransform(unsigned int stateIndex);
			virtual ~DecrementStateTransform() {};
			virtual void apply(Mapping *m, MappingSet **ms);
			virtual int getComponentIndex() const {return cIndex;};
		protected:
			int cIndex;
	};



}







#endif /*TRANSFORMATION_HH_*/
