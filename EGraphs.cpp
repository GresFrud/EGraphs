#include <iostream>
#include <map>
#include <vector>


namespace Extensions
{
	class Function
	{
		
		public: std::string Name;
		std::vector<Function*>* UsedBy;
		Function* Parent;
		std::vector<Function*>* Inputs;

	public: Function(std::vector<Function*>* inputs, std::string name)
		{
			this->Name = name;
			this->UsedBy = new std::vector<Function*>();
			this->Parent = this;
			this->Inputs = inputs;
			for (Function* func : *inputs)
			{
				func->UsedBy->push_back(this);
			}
		}

		void ManualDestroy()
		{
			if (this->UsedBy->empty())
			{
				for (Function* func : *this->Inputs)
				{
					func->Inputs->erase(std::remove(func->Inputs->begin(), func->Inputs->end(), this), func->Inputs->end());
				}
				delete this->UsedBy;
				delete this->Inputs;
				delete this;
			}
		}

		~Function()
		{
			/*if (this->UsedBy->empty())
			{
				for (Function* func : *this->Inputs)
				{
					func->Inputs->erase(std::remove(func->Inputs->begin(), func->Inputs->end(), this), func->Inputs->end());
				}
				delete this->UsedBy;
				delete this->Inputs;
			}*/
		}

	public:
		std::string getName()
		{
			return this->Name;
		}

		Function* GetRoot()
		{
			if (this->Parent == this)
			{
				return this;
			}
			Function* root = this->Parent->GetRoot();
			this->Parent = root;
			return root;
		}

		bool operator==(const Function& other) const
		{
			if (this->Inputs->size() != other.Inputs->size() || this->Name != other.Name)
			{
				return false;
			}
			for (int i = 0; i < this->Inputs->size(); i++)
			{
				if (!(*(*(this->Inputs))[i] == *(*(other.Inputs))[i]))
				{
					return false;
				}
			}
			return true;
		}

		bool operator!=(const Function& other) const
		{
			return !(*this == other);
		}

		bool IsEquivalent(const Function& other)
		{

		}
	};

	bool TryGetRealFunction(Function function, std::map<std::string, std::vector<Function*>*> functions, Function& outFunction)
	{
		if (functions.find(function.getName()) == functions.end())
		{
			return false;
		}
		for (Function* realFunction : *functions[function.getName()])
		{
			if (*realFunction == function)
			{
				outFunction = *realFunction;
				return true;
			}
		}
		return false;
	}

	class EGraph
	{

		std::map<std::string, std::vector<Function*>*> _functions;
		std::map<Function, std::vector<Function*>> _class;
		std::vector<Function*> _in_equalities;
		std::vector<Function*> _quantified_variables;

	public: EGraph()
		{
			this->_quantified_variables = std::vector<Function*>();
			this->_functions = std::map<std::string, std::vector<Function*>*>{};
			this->_in_equalities = std::vector<Function*>();
		}

		~EGraph()
		{
			std::map<std::string, std::vector<Function*>*>::iterator it;
			for (it = this->_functions.begin(); it != this->_functions.end(); it++)
			{
				for (Function* f : *it->second)
				{
					delete f->Inputs;
					delete f->UsedBy;
					delete f;
				}
				delete it->second;
			}
			for (Function* f : this->_in_equalities)
			{
				delete f->Inputs;
				delete f->UsedBy;
				delete f;
			}
		}

		void SplitIntoClasses()
		{

		}

		void AddQuantifiedVariable(std:: string name)
		{
			_quantified_variables.push_back(AddTerm(name));
		}

		Function* AddTerm(std::string name)
		{
			if (this->_functions.find(name) == this->_functions.end())
			{
				Function* term = new Function(new std::vector<Function*>{}, name);
				this->_functions[name] = new std::vector<Function*>{ term }; // if term didn't exist, make it
			}
			return this->_functions[name]->at(0);
		}

		Function* AddFunction(std::vector<Function*>* inputs, std::string name)
		{
			if (this->_functions.find(name) == this->_functions.end())
			{
				Function* func = new Function(inputs, name);
				this->_functions[name] = new std::vector<Function*>{ func }; // if function didn't exist, make it
				return this->_functions[name]->at(0);
			}
			Function* temporary = new Function(inputs, name);
			for (Function* func : *this->_functions[name])
			{
				if (*temporary == *func)
				{
					temporary->ManualDestroy();
					return func;
				}
			}
			this->_functions[name]->push_back(temporary);
			return temporary;
		}

		void AddEquality(Function first, Function second)
		{
			Function realFirst = first;
			if (TryGetRealFunction(first, this->_functions, realFirst))
			{
				// delete &first;
			}

			Function realSecond = second;
			if (TryGetRealFunction(second, this->_functions, realSecond))
			{
				// delete &second;
			}
			std::vector<Function*>* equality = new std::vector<Function*>{ &realFirst, &realSecond };
			Function* eq = new Function(equality, "eq");
			this->_in_equalities.push_back(eq);
			realSecond.GetRoot()->Parent = realFirst.GetRoot();
		}

		bool IsGround(Function* function)
		{
			// this happens when the function is a Term
			if (function->Inputs->size() == 0)
			{
				for (Function* variable : _quantified_variables)
				{
					// when function is found in the list of quantified variables, return false
					if (*variable == *function)
					{
						return false;
					}
				}
				return true;
			}
			// when it is not a Term, check all its inputs
			for (Function* input : *function->Inputs)
			{
				// if at least one is not ground, return false
				if (!IsGround(input))
				{
					return false;
				}
			}
			return true;
		}

		std::map<Function, Function>* FindDefs()
		{
			// Initialize representative function
			std::map<Function, Function>* repr = new std::map<Function, Function>();
			std::vector<Function*> groundTerms = std::vector<Function*>();

			// process every ground term
			for (auto pair = _functions.begin(); pair != _functions.end(); pair++)
			{
				for (Function* function : *pair->second)
				{
					if (IsGround(function))
					{
						groundTerms.push_back(function);
					}
				}
			}
			repr = AssignRepresentatives(repr, groundTerms);

			// process leftover terms
			std::vector<Function*> leftoverTerms = std::vector<Function*>();
			for (auto pair = _functions.begin(); pair != _functions.end(); pair++)
			{
				for (Function* function : *pair->second)
				{
					leftoverTerms.push_back(function);
				}
			}
			repr = AssignRepresentatives(repr, leftoverTerms);

			return repr;
		}

		std::map<Function, Function>* AssignRepresentatives(std::map<Function, Function>* repr, std::vector<Function*> toBeAssigned)
		{
			// iterate through functions and terms while possible
			for (int i = 0; i < toBeAssigned.size(); i++)
			{
				Function* function = toBeAssigned[i];
				// if they have a representative, skip
				if (repr->find(*function) != repr->end())
				{
					continue;
				}
				for (Function* func : _class[*function])
				{
					// set the representative of everything equivalent to self
					(*repr)[*func] = *function;
					for (Function* parent : *func->UsedBy)
					{
						bool isGround = true;
						// check if parent became ground term
						for (Function* child : *parent->Inputs)
						{
							if (repr->find(*child) == repr->end())
							{
								isGround = false;
								break;
							}
						}
						// assign it later if yes
						if (isGround)
						{
							toBeAssigned.push_back(parent);
						}
					}
				}
			}
			return repr;
		}

		void AddPredicate(std::vector<Function*>* functions, std::string name)
		{
			for (int i = 0; i < functions->size(); i++)
			{
				Function* oldFunction = (*functions)[i];
				if (TryGetRealFunction(*oldFunction, this->_functions, *(*functions)[i]))
				{
					// delete
				}
			}
			this->_in_equalities.push_back(new Function(functions, name));
		}
	};
}

namespace Tests
{
	#include <cassert>
	using namespace Extensions;


	void TermInequalityTest()
	{
		EGraph graph = EGraph();
		Function* a = graph.AddTerm("a");
		Function* b = graph.AddTerm("b");

		assert(*a != *b);
	}

	void FunctionEqualityTest()
	{
		EGraph graph = EGraph();
		Function* a = graph.AddTerm("a");
		Function* b = graph.AddTerm("b");
		Function* a2 = graph.AddTerm("a");
		Function* b2 = graph.AddTerm("b");

		Function* f = graph.AddFunction(new std::vector<Function*>{ a, b }, "f");
		Function* g = graph.AddFunction(new std::vector<Function*>{ a2, b2 }, "f");
		Function* h = graph.AddFunction(new std::vector<Function*>{ b, a }, "f");

		assert(*f == *g);
		assert(*f != *h);
	}

	void AddPredicateTest()
	{
		EGraph graph = EGraph();
		Function* a = graph.AddTerm("a");
		Function* b = graph.AddTerm("b");

		Function* f = graph.AddFunction(new std::vector<Function*>{ a, b }, "f");
		Function* h = graph.AddFunction(new std::vector<Function*>{ b, a }, "f");

		graph.AddEquality(*f, *h);
	}

	void Tests()
	{
		TermInequalityTest();
		FunctionEqualityTest();
		AddPredicateTest();
	}
}

int main(int argc, char* argv[])
{
	Tests::Tests();
	return 0;
}