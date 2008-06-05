/*
  KernelActions.hpp
  (c) 2008 Tod D. Romo


  Grossfield Lab
  Department of Biochemistry and Biophysics
  University of Rochester Medical School

  Classes for storing/compiling commands for the Kernel VM...
  This is an example of the Command design pattern...

*/



#if !defined(KERNELACTIONS_HPP)
#define KERNELACTIONS_HPP

#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <string.h>

#include <boost/regex.hpp>

#include "Atom.hpp"

#include "KernelValue.hpp"
#include "KernelStack.hpp"


using namespace std;

namespace loos {


  //! Base class for all commands...
  /** All subclasses must implement the execute() method, which will
   *  operate on the data stack pointer.
   *
   *  Subclasses may also override the name() method if they want to
   *  augment the command-name string (i.e. to show additional internal
   *  data)
   */

  
  class Action {
  protected:
    //! Pointer to the data stack
    ValueStack *stack;
    //! Pointer to the atom we'll be working on...
    pAtom atom;
    //! Record of command-name (for printing)
    string my_name;

    // Some utility functions...

    //! Compare the top two items on the stack...
    int binComp(void) {
      Value v1 = stack->pop();
      Value v2 = stack->pop();
      return(compare(v2, v1));
    }

    //! Check to make sure an atom has been set...
    void hasAtom(void) {
      if (atom == 0)
	throw(runtime_error("No atom set"));
    }

  public:
    Action(const string s) : stack(0), atom(pAtom()), my_name(s) { }

    void setStack(ValueStack* ptr) { stack=ptr; }
    void setAtom(pAtom pa) { atom = pa; }

    virtual string name(void) const { return(my_name); }

    virtual void execute(void) =0;
    virtual ~Action() { }

  };



  //! Push a string onto the data stack
  class pushString : public Action {
    Value val;
  public:
    pushString(const string str) : Action("pushString"), val(str) { }
    void execute(void) { stack->push(val); }
    string name(void) const {
      stringstream s;
      s << my_name << "(" << val << ")";
      return(s.str());
    }

  };

  //! Push an integer onto the data stack
  class pushInt : public Action {
    Value val;
  public:
    pushInt(const int i) : Action("pushInt"), val(i) { }
    void execute(void) { stack->push(val); }
    string name(void) const {
      stringstream s;
      s << my_name << "(" << val << ")";
      return(s.str());
    }
  };

  //! Push a float onto the data stack
  class pushFloat : public Action {
    Value val;
  public:
    pushFloat(const float f) : Action("pushFloat"), val(f) { }
    void execute(void) { stack->push(val); }
    string name(void) const {
      stringstream s;
      s << my_name << "(" << val << ")";
      return(s.str());
    }
  };


  //! Drop the top item from the stack
  class drop : public Action {
  public:
    drop() : Action("drop") { }
    void execute(void) { stack->drop(); }
  };

  //! Duplicate the top item on the stack
  class dup : public Action {
  public:
    dup() : Action("dup") { }
    void execute(void) { stack->dup(); }
  };


  //! Test for equality: ARG1 ARG ==
  class equals : public Action {
  public:
    equals() : Action("==") { }
    void execute(void) {
      Value v(binComp() == 0);
      stack->push(v);
    }
  };

  //! Relation operators...:  ARG1 ARG2 <
  class lessThan : public Action {
  public:
    lessThan() : Action("<") { }
    void execute(void) {
      Value v(binComp() < 0);
      stack->push(v);
    }
  };

  //! ARG1 ARG2 <=
  class lessThanEquals : public Action {
  public:
    lessThanEquals() : Action("<=") { }
    void execute(void) {
      Value v(binComp() <= 0);
      stack->push(v);
    }
  };

  //! ARG1 ARG2 >
  class greaterThan : public Action {
  public:
    greaterThan() : Action(">") { }
    void execute(void) {
      Value v(binComp() > 0);
      stack->push(v);
    }
  };

  //! ARG1 ARG2 >=
  class greaterThanEquals : public Action {
  public:
    greaterThanEquals() : Action(">=") { }
    void execute(void) {
      Value v(binComp() >= 0);
      stack->push(v);
    }
  };

  //! Regular expression matching: ARG1 regexp(S)
  /** Compiles the passed string into a regex pattern at instantiation,
   * then at execution matches the top stack entry against the
   * pattern...
   */

  class matchRegex : public Action {
    boost::regex regexp;
  public:
    matchRegex(const string s) : Action("matchRegex"), regexp(s, boost::regex::perl|boost::regex::icase), pattern(s) { }
    void execute(void) { 
      Value v = stack->pop();
      Value r(0);
      if (boost::regex_search(v.getString(), regexp))
	r.setInt(1);

      stack->push(r);
    }
    string name(void) const {
      return(my_name + "(" + pattern + ")");
    }
    
  private:
    string pattern;
  };
  

  //! Regular expression matching: ARG1 regexp(ARG2)
  /** Takes the top item on the stack and compiles this into a regular
   * expression, then matches it against the next item on the stack.
   * This is likely to be pretty inefficient, so it's better to use
   * matchRegex instead if you can.
   */
  class matchStringAsRegex : public Action {
  public:
    matchStringAsRegex() : Action("matchStringAsRegex") { }
    void execute(void) {
      Value v = stack->pop();
      boost::regex re(v.getString(), boost::regex::perl|boost::regex::icase);
      Value u = stack->pop();
      Value r(0);
      
      if (boost::regex_search(u.getString(), re))
	r.setInt(1);
      
      stack->push(r);
    }
  };
  
  
  //! Extracts a number for a string on the stack using a regular expression: ARG1 regexp(S)
  /** Compiles the passed string into a regex pattern at
   * instantiation.  At execution, examines each matched capture for
   * the first one that converts to an integer and pushes that value
   * onto the data stack.  If no match is found (or no numeric
   * conversion works), then "-1" is pushed onto the stack.
   */
  class extractNumber : public Action {
  public:
    extractNumber(const string s) : Action("extractNumber"),
				    regexp(s, boost::regex::perl|boost::regex::icase),
				    pattern(s) { }

    void execute(void) {
      Value v = stack->pop();
      Value r(-1);
      boost::smatch what;

      if (boost::regex_search(v.getString(), what, regexp)) {
	unsigned i;
	int val;
	for (i=0; i<what.size(); i++) {
	  if ((stringstream(what[i]) >> val)) {
	    r.setInt(val);
	    break;
	  }
	}
      }

      stack->push(r);
    }

    string name(void) const {
      return(my_name + "(" + pattern + ")");
    }

  private:
    boost::regex regexp;
    string pattern;
  };




  //! Push atom name onto the stack
  class pushAtomName : public Action {
  public:
    pushAtomName() : Action("pushAtomName") { }
    void execute(void) {
      hasAtom();
      Value v(atom->name());
      stack->push(v);
    }
  };

  //! Push atom id onto the stack
  class pushAtomId : public Action {
  public:
    pushAtomId() : Action("pushAtomId") { }
    void execute(void) {
      hasAtom();
      Value v(atom->id());
      stack->push(v);
    }
  };

  //! Push atom'ss residue name onto the stack
  class pushAtomResname : public Action {
  public:
    pushAtomResname() : Action("pushAtomResname") { }
    void execute(void) {
      hasAtom();
      Value v(atom->resname());
      stack->push(v);
    }
  };

  //! Push atom's residue id onto the stack
  class pushAtomResid : public Action {
  public:
    pushAtomResid() : Action("pushAtomResid") { }
    void execute(void) {
      hasAtom();
      Value v(atom->resid());
      stack->push(v);
    }
  };

  //! Push atom's segid onto the stack
  class pushAtomSegid : public Action {
  public:
    pushAtomSegid() : Action("pushAtomSegid") { }
    void execute(void) {
      hasAtom();
      Value v(atom->segid());
      stack->push(v);
    }
  };


  // Logical operations...  Assumes stack args are ints...

  //! ARG1 ARG2 &&
  class logicalAnd : public Action {
  public:
    logicalAnd() : Action("&&") { }
    void execute(void) {
      Value v2 = stack->pop();
      Value v1 = stack->pop();

      if (!(v1.type == Value::INT && v2.type == Value::INT))
	throw(runtime_error("Invalid operands to logicalAnd"));

      Value u(v1.itg && v2.itg);
      stack->push(u);
    }
  };

  //! ARG1 ARG2 ||
  class logicalOr : public Action {
  public:
    logicalOr() : Action("||") { }
    void execute(void) {
      Value v1 = stack->pop();
      Value v2 = stack->pop();

      if (!(v1.type == Value::INT && v2.type == Value::INT))
	throw(runtime_error("Invalid operands to logicalOr"));

      Value u(v1.itg || v2.itg);
      stack->push(u);
    }
  };


  //! ARG1 !
  class logicalNot : public Action {
  public:
    logicalNot() : Action("!") { }
    void execute(void) {
      Value v1 = stack->pop();

      if (v1.type != Value::INT)
	throw(runtime_error("Invalid operand to logicalNot"));

      Value u(!v1.itg);
      stack->push(u);
    }
  };

  // Always returns true...
  class logicalTrue : public Action {
  public:
    logicalTrue() : Action("TRUE") { }
    void execute(void) {
      Value v((int)1);
      stack->push(v);
    }
  };

};



#endif

