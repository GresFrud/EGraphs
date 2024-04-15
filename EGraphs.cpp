#include <iostream>
#include <map>
#include <set>
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

		Function()
		{
			this->Name = "";
			this->UsedBy = new std::vector<Function*>();
			this->Parent = this;
			this->Inputs = new std::vector<Function*>();
		}

		void ManualDestroy()
		{
			if (this->UsedBy->empty())
			{
				for (Function* func : *this->Inputs)
				{
					func->UsedBy->erase(std::remove(func->UsedBy->begin(), func->UsedBy->end(), this), func->UsedBy->end());
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
			for (size_t i = 0; i < this->Inputs->size(); i++)
			{
				if (!(*(*(this->Inputs))[i] == *(*(other.Inputs))[i]))
				{
					return false;
				}
			}
			return true;
		}

		bool operator<(const Function& other) const
		{
			if (this->Name != other.Name)
			{
				return this->Name < other.Name;
			}
			if (this->Inputs->size() != other.Inputs->size())
			{
				return this->Inputs->size() < other.Inputs->size();
			}
			for (size_t i = 0; i < this->Inputs->size(); i++)
			{
				if (*(*this->Inputs)[i] != *(*other.Inputs)[i])
				{
					return *(*this->Inputs)[i] < *(*other.Inputs)[i];
				}
			}
			return false;
		}

		bool operator!=(const Function& other) const
		{
			return !(*this == other);
		}

		bool IsEquivalent(Function other)
		{
			if (this->GetRoot() == other.GetRoot())
			{
				return true;
			}
			if (this->getName() != other.getName())
			{
				return false;
			}
			for (size_t i = 0; i < this->Inputs->size(); i++)
			{
				if ((*this->Inputs)[i]->GetRoot() != (*other.Inputs)[i]->GetRoot())
				{
					return false;
				}
			}
			return true;

		}
	};

	bool TryGetRealFunction(Function* function, std::map<std::string, std::vector<Function*>*> functions, Function** outFunction)
	{
		if (functions.find(function->getName()) == functions.end())
		{
			return false;
		}
		for (Function* realFunction : *functions[function->getName()])
		{
			if (*realFunction == *function)
			{
				*outFunction = realFunction;
				return true;
			}
		}
		return false;
	}

	class EGraph
	{

		std::map<std::string, std::vector<Function*>*> _functions;
		std::map<Function, std::vector<Function*>*> _class;
		std::vector<Function*> _in_equalities;
		std::set<Function*> _quantified_variables;

		public: EGraph()
		{
			this->_quantified_variables = std::set<Function*>();
			this->_functions = std::map<std::string, std::vector<Function*>*>{};
			this->_in_equalities = std::vector<Function*>();
			this->_class = std::map<Function, std::vector<Function*>*>();
		}

		~EGraph()
		{
			for (auto it = this->_functions.begin(); it != this->_functions.end(); it++)
			{
				for (Function* f : *it->second)
				{
					delete f->Inputs;
					delete f->UsedBy;
					delete f;
				}
				delete it->second;
			}
			for (auto it = this->_class.begin(); it != this->_class.end(); it++)
			{
				delete it->second;
			}
		}

		std::map<Function, std::vector<Function*>*> GetClasses()
		{
			return _class;
		}

		void MakeEqual(Function* first, Function* second)
		{
			if (first->GetRoot() == second->GetRoot())
			{
				return;
			}
			Function* root = second->GetRoot();
			root->Parent = first->GetRoot();
			// concat
			auto asd = this->_class[*first->GetRoot()]->end();
			this->_class[*first->GetRoot()]->insert(this->_class[*first->GetRoot()]->end(), this->_class[*root]->begin(), this->_class[*root]->end());
			delete this->_class[*root];
			this->_class.erase(*root);
		}

		void AddQuantifiedVariable(std:: string name)
		{
			_quantified_variables.insert(AddTerm(name));
		}

		Function* AddTerm(std::string name)
		{
			if (this->_functions.find(name) == this->_functions.end())
			{
				Function* term = new Function(new std::vector<Function*>{}, name);
				this->_functions[name] = new std::vector<Function*>{ term }; // if term didn't exist, make it
				this->_class[*this->_functions[name]->at(0)] = new std::vector<Function*>{ term };
			}
			return this->_functions[name]->at(0);
		}

		Function* AddFunction(std::vector<Function*>* inputs, std::string name)
		{
			if (this->_functions.find(name) == this->_functions.end())
			{
				Function* func = new Function(inputs, name);
				this->_functions[name] = new std::vector<Function*>{ func }; // if function didn't exist, make it
				this->_class[*this->_functions[name]->at(0)] = new std::vector<Function*>{ func };
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
			this->_class[*this->_functions[name]->back()] = new std::vector<Function*>{temporary};
			CheckEqualities(temporary);
			return temporary;
		}

		void CheckEqualities(Function* func)
		{
			for (size_t i = 0; i < this->_functions[func->getName()]->size() - 1; i++)
			{
				if (func->IsEquivalent(*(*this->_functions[func->getName()])[i]))
				{
					MakeEqual(func, (*this->_functions[func->getName()])[i]);
				}
			}
		}

		void AddEquality(Function* first, Function* second)
		{
			Function* realFirst = first;
			if (TryGetRealFunction(first, this->_functions, &realFirst))
			{
				if (first != realFirst)
				{
					first->ManualDestroy();
				}
			}

			Function* realSecond = second;
			if (TryGetRealFunction(second, this->_functions, &realSecond))
			{
				if (second != realSecond)
				{
					second->ManualDestroy();
				}
			}
			std::vector<Function*>* equality = new std::vector<Function*>{ realFirst, realSecond };
			Function* eq = this->AddFunction(equality, "eq");
			this->_in_equalities.push_back(eq);

			MakeEqual(realFirst, realSecond);
			for (Function* func : *realFirst->UsedBy)
			{
				CheckEqualities(func);
			}
			for (Function* func : *realSecond->UsedBy)
			{
				CheckEqualities(func);
			}
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

		std::map<Function, Function*>* FindDefs()
		{
			// Initialize representative function
			std::map<Function, Function*>* repr = new std::map<Function, Function*>();
			std::vector<Function*> groundTerms = std::vector<Function*>();

			// process every ground term
			for (auto pair = _functions.begin(); pair != _functions.end(); pair++)
			{
				for (Function* function : *pair->second)
				{
					if (IsGround(function) && function->Inputs->size() == 0)
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
					if (function->Inputs->size() == 0)
					{
						leftoverTerms.push_back(function);
					}
				}
			}
			repr = AssignRepresentatives(repr, leftoverTerms);

			return repr;
		}

		std::map<Function, Function*>* AssignRepresentatives(std::map<Function, Function*>* repr, std::vector<Function*> toBeAssigned)
		{
			// iterate through functions and terms while possible
			for (size_t i = 0; i < toBeAssigned.size(); i++)
			{
				Function* function = toBeAssigned[i];
				// if they have a representative, skip
				if (repr->find(*function) != repr->end())
				{
					continue;
				}
				for (Function* func : *_class[*function->GetRoot()])
				{
					// set the representative of everything equivalent to self
					(*repr)[*func] = function;
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

		bool MakesCycle(Function* NewGround, std::map<Function, Function*>* repr)
		{
			std::map<Function*, bool>* ColoredGraph = new std::map<Function*, bool>();

			// initialize 0 as the value for every node of EGraph at the start
			std::map<std::string, std::vector<Function*>*>::iterator it;
			for (it = this->_functions.begin(); it != this->_functions.end(); it++)
			{
				for (Function* func : *it->second)
				{
					(*ColoredGraph)[func] = false;
				}
			}

			for (Function* descendant : *NewGround->Inputs)
			{
				if (MakesCycleAux(NewGround, repr, descendant, ColoredGraph))
				{
					return true;
				}
			}

			return false;
		}

		bool MakesCycleAux(Function* NewGround, std::map<Function, Function*>* repr, Function* current, std::map<Function*, bool>* ColoredGraph)
		{
			if (current->GetRoot() == NewGround->GetRoot())
			{
				return true;
			}
			current = (*repr)[*current];
			for (Function* descendant : *current->Inputs)
			{
				if ((*ColoredGraph)[current])
				{
					continue;
				}

				if (!MakesCycleAux(NewGround, repr, descendant, ColoredGraph))
				{
					return true;
				}
				(*ColoredGraph)[current] = true;
			}
			return false;
		}

		std::map<Function, Function*>* RefineDefs(std::map<Function, Function*>* repr)
		{
			for (Function* function : this->_quantified_variables)
			{
				if ((*repr)[*function] != function)
				{
					continue; // quantified variable already represented by something else
				}
				Function* NewGround = function;

				for (Function* InSameClass : *_class[*function])
				{
					if (InSameClass == function || this->_quantified_variables.count(InSameClass) != 0 || this->MakesCycle(InSameClass, repr))
					{
						continue; // quantified variable shouldn't be represented by self, another quantified variable and it shouldn't create a cycle
					}
					NewGround = InSameClass;
					break;
				}

				for (Function* InSameClass : *_class[*function])
				{
					(*repr)[*InSameClass] = NewGround;
				}
			}
			return repr;
		}

		void AddPredicate(std::vector<Function*>* functions, std::string name)
		{
			for (size_t i = 0; i < functions->size(); i++)
			{
				Function* oldFunction = (*functions)[i];
				if (TryGetRealFunction(oldFunction, this->_functions, &(*functions)[i]))
				{
					// delete
				}
			}
			Function* newFunc = this->AddFunction(functions, name);
			this->_in_equalities.push_back(newFunc);
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

	void ImplicitEqualityTest()
	{
		EGraph graph = EGraph();
		graph.AddQuantifiedVariable("x");
		graph.AddQuantifiedVariable("y");
		graph.AddQuantifiedVariable("z");
		graph.AddEquality(graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("x") }, "read"), graph.AddTerm("z"));
		graph.AddEquality(graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("k"), graph.AddTerm("1") }, "+"),
			graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("y") }, "read"));
		graph.AddEquality(graph.AddTerm("x"), graph.AddTerm("y"));
		graph.AddPredicate(new std::vector<Function*>{ graph.AddTerm("3"), graph.AddTerm("z") }, ">");
		
		assert(graph.GetClasses().size() == 10);
	}

	void FindDefsTest()
	{
		EGraph graph = EGraph();
		graph.AddQuantifiedVariable("x");
		graph.AddQuantifiedVariable("y");
		graph.AddQuantifiedVariable("z");
		graph.AddEquality(graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("x") }, "read"), graph.AddTerm("z"));
		graph.AddEquality(graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("k"), graph.AddTerm("1") }, "+"), 
					      graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("y") }, "read"));
		graph.AddEquality(graph.AddTerm("x"), graph.AddTerm("y"));
		graph.AddPredicate(new std::vector<Function*>{ graph.AddTerm("3"), graph.AddTerm("z") }, ">");
		auto repr = graph.FindDefs();

		assert((*repr)[*graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("y") }, "read")] ==
				graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("k"), graph.AddTerm("1") }, "+"));
		assert((*repr)[*graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("a"), graph.AddTerm("x") }, "read")] ==
			graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("k"), graph.AddTerm("1") }, "+"));
		assert((*repr)[*graph.AddTerm("z")] == graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("k"), graph.AddTerm("1") }, "+"));
		delete repr;
	}

	void RefineDefsTest()
	{
		EGraph graph = EGraph();
		graph.AddQuantifiedVariable("x");
		graph.AddQuantifiedVariable("y");
		graph.AddEquality(graph.AddTerm("x"), graph.AddFunction(new std::vector<Function*>{ graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("x") }, "f") }, "g"));
		graph.AddEquality(graph.AddTerm("y"), graph.AddFunction(new std::vector<Function*>{ graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("y") }, "f") }, "h"));
		graph.AddEquality(graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("x") }, "f"), graph.AddFunction(new std::vector<Function*>{ graph.AddTerm("y") }, "f"));
		auto repr = graph.FindDefs();
		repr = graph.RefineDefs(repr);
	}

	void AddPredicateTest()
	{
		EGraph graph = EGraph();
		Function* a = graph.AddTerm("a");
		Function* b = graph.AddTerm("b");

		Function* f = graph.AddFunction(new std::vector<Function*>{ a, b }, "f");
		Function* h = graph.AddFunction(new std::vector<Function*>{ b, a }, "f");

		graph.AddEquality(f, h);
	}

	void Tests()
	{
		TermInequalityTest();
		FunctionEqualityTest();
		ImplicitEqualityTest();
		FindDefsTest();
		RefineDefsTest();
		AddPredicateTest();
	}
}

int main(int argc, char* argv[])
{
	Tests::Tests();
	return 0;
}
