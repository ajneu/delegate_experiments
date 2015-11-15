#include <utility>

// very experimental....

/*

Todo: Compare with Don Clugston's original
       https://github.com/dreamcat4/FastDelegate

etc etc
*/


// https://gcc.gnu.org/onlinedocs/gcc-4.9.0/gcc/Bound-member-functions.html


template<typename Cl, typename Ret, typename... Args>
using signat_t = Ret(*)(Cl *, Args...);
/*
  Examples:
  signat_t<T, void>               = void(*)(T *);
  signat_t<T, int, double, float) =  int(*)(T *, double, float);
*/



template <typename Signature>
class FFF;

template <typename Ret, typename... Args>
class FFF<Ret(Args...)>
{
private:
   void *inst{};
   signat_t<void, Ret, Args...> mfp{};
   
   template<typename T>
   static Ret static_invoker(decltype(mfp) mfffp, void *inss, Args... args) {
      return mfffp(static_cast<T*>(inss), std::forward<Args>(args)...);
   }

   decltype(static_invoker<void>) *fp_invoker;
   
public:
   template<typename Cl1, typename Cl2>
   static FFF make(Cl1 *ins, Ret(Cl2::*mmm)(Args...)) {
      return FFF{ins, reinterpret_cast<decltype(mfp)>((ins)->*mmm), static_invoker<Cl2>};
   }

   template<typename Cl1, typename Cl2>
   static FFF make(const Cl1 *ins, Ret(Cl2::*mmm)(Args...) const) {
      return FFF{const_cast<void *>(ins), reinterpret_cast<decltype(mfp)>((ins)->*mmm), static_invoker<Cl2>};
   }

   template<typename Cl1, typename Cl2>
   void bind(Cl1 *ins, Ret(Cl2::*mmm)(Args...)) {
      inst = ins;
      mfp = reinterpret_cast<decltype(mfp)>((ins)->*mmm);
      fp_invoker = static_invoker<Cl2>;
   }

   template<typename Cl1, typename Cl2>
   void bind(const Cl1 *ins, Ret(Cl2::*mmm)(Args...) const) {
      inst = const_cast<void *>(static_cast<const void *>(ins));
      mfp = reinterpret_cast<decltype(mfp)>((ins)->*mmm);
      fp_invoker = static_invoker<Cl2>;
   }

   
   Ret operator()(Args... args) {
      return fp_invoker(mfp, inst, std::forward<Args>(args)...);
   }

   FFF() = default;

   bool operator==(const FFF &rhs)
   {
      return ((fp_invoker == rhs.fp_invoker) && (mfp == rhs.mfp) && (inst == rhs.inst));
   }

   bool operator!() {
      return (inst == nullptr);
   }

private:

   FFF(void *ins, decltype(mfp) mmm, decltype(fp_invoker) inv) : inst{ins}, mfp{mmm}, fp_invoker{inv}
   {   }

};


#include <stdio.h>

// Demonstrate the syntax for FastDelegates.
//				-Don Clugston, May 2004.
// It's a really boring example, but it shows the most important cases.



// Declare some functions of varying complexity...
void SimpleStaticFunction(int num, const char *str) {
	printf("In SimpleStaticFunction. Num=%d, str = %s\n", num, str);
}

void SimpleVoidFunction() {
	printf("In SimpleVoidFunction with no parameters.\n");
}

class CBaseClass {
protected:
	const char *m_name;
public:
	CBaseClass(const char *name) : m_name(name) {};
	void SimpleMemberFunction(int num, const char *str) {
		printf("In SimpleMemberFunction in %s. Num=%d, str = %s\n", m_name, num, str);	}
	int SimpleMemberFunctionReturnsInt(int num, const char *str) {
		printf("In SimpleMemberFunction in %s. Num=%d, str = %s\n", m_name, num, str); return -1;	}
	void ConstMemberFunction(int num, const char *str) const {
		printf("In ConstMemberFunction in %s. Num=%d, str = %s\n", m_name, num, str);	}
	virtual void SimpleVirtualFunction(int num, const char *str) {
		printf("In SimpleVirtualFunction in %s. Num=%d, str = %s\n", m_name, num, str);	}
	static void StaticMemberFunction(int num, const char *str) {
		printf("In StaticMemberFunction. Num=%d, str =%s\n", num, str);	}
};

class COtherClass {
	double rubbish; // to ensure this class has non-zero size.
public:
	virtual void UnusedVirtualFunction(void) { }
	virtual void TrickyVirtualFunction(int num, const char *str)=0;
};

class VeryBigClass {
	int letsMakeThingsComplicated[400];
};

// This declaration ensures that we get a convoluted class heirarchy.
class CDerivedClass : public VeryBigClass, virtual public COtherClass, virtual public CBaseClass
{
	double m_somemember[8];
public:
	CDerivedClass() : CBaseClass("Base of Derived") { m_somemember[0]=1.2345; }
	void SimpleDerivedFunction(int num, const char *str) { printf("In SimpleDerived. num=%d\n", num); }
	virtual void AnotherUnusedVirtualFunction(int num, const char *str) {}
	virtual void TrickyVirtualFunction(int num, const char *str) {
		printf("In Derived TrickyMemberFunction. Num=%d, str = %s\n", num, str);
	}
};





int main(void)
{
	// Delegates with up to 8 parameters are supported.
	// Here's the case for a void function.
	// We declare a delegate and attach it to SimpleVoidFunction()
	printf("-- FastDelegate demo --\nA no-parameter delegate is declared using FastDelegate0\n\n");

        void (*noparameterdelegate) () = &SimpleVoidFunction;

	noparameterdelegate(); // invoke the delegate - this calls SimpleVoidFunction()

	printf("\n-- Examples using two-parameter delegates (int, const char *) --\n\n");

        using MyDelegate    = FFF<void (int, const char *)>;
        using IntMyDelegate = FFF<int  (int, const char *)>;


	MyDelegate funclist[12]; // delegates are initialized to empty
	CBaseClass a("Base A");
	CBaseClass b("Base B");
	CDerivedClass d;
	CDerivedClass c;

	IntMyDelegate newdeleg;
        //newdeleg = MakeDelegate(&a, &CBaseClass::SimpleMemberFunctionReturnsInt);
        //fptr_i<T> mfpA_f_i = reinterpret_cast<decltype(mfpA_f_i)>(&A::f_i);

	// Binding a simple member function
        funclist[0].bind(&a, &CBaseClass::SimpleMemberFunction);
#if 0		
	// You can also bind static (free) functions
        funclist[1].bind(&SimpleStaticFunction);

	// and static member functions
        funclist[2].bind(&CBaseClass::StaticMemberFunction);
#endif
	// and const member functions (these only need a const class pointer).		 
        funclist[11].bind( (const CBaseClass *)&a, &CBaseClass::ConstMemberFunction);

        funclist[3].bind( &a, &CBaseClass::ConstMemberFunction);

	// and virtual member functions
        funclist[4].bind(&b, &CBaseClass::SimpleVirtualFunction);

#if 0
	// You can also use the = operator. For static functions, a fastdelegate
	// looks identical to a simple function pointer.
        funclist[5] = &CBaseClass::StaticMemberFunction;
#endif
        
	// The weird rule about the class of derived member function pointers is avoided.
	// For MSVC, you can use &CDerivedClass::SimpleVirtualFunction here, but DMC will complain.
	// Note that as well as .bind(), you can also use the MakeDelegate()
	// global function.
        //funclist[6] = MyDelegate::make<CBaseClass>(&d, &CDerivedClass::SimpleVirtualFunction);
        funclist[6] = MyDelegate::make<CBaseClass>(&d, &CBaseClass::SimpleVirtualFunction);

        //(reinterpret_cast<signat_t<void, void, int, const char *>>((&d)->*(&CBaseClass::SimpleVirtualFunction)))(static_cast<CBaseClass*>(&d), 6, "here we go!");



	// The worst case is an abstract virtual function of a virtually-derived class
	// with at least one non-virtual base class. This is a VERY obscure situation,
	// which you're unlikely to encounter in the real world.
	// FastDelegate versions prior to 1.3 had problems with this case on VC6.
	// Now, it works without problems on all compilers.
       funclist[7].bind(&c, &CDerivedClass::TrickyVirtualFunction);

       // BUT... in such cases you should be using the base class as an 
	// interface, anyway.
       funclist[8].bind(&c, &COtherClass::TrickyVirtualFunction);

       // Calling a function that was first declared in the derived class is straightforward
        funclist[9] = MyDelegate::make(&c, &CDerivedClass::SimpleDerivedFunction);


        // You can also bind directly using the constructor
        MyDelegate dg = MyDelegate::make(&b, &CBaseClass::SimpleVirtualFunction);

	const char *msg;
	for (int i=0; i<12; i++) {
                msg = "Looking for equal delegate";
		printf("%d :", i);
		// The == and != operators are provided
		// Note that they work even for inline functions.
		if (funclist[i]==dg) { msg = "Found equal delegate"; };
		// operator ! can be used to test for an empty delegate
		// You can also use the .empty() member function.
		if (!funclist[i]) {
			printf("Delegate is empty\n");
		} else {
			// Invocation generates optimal assembly code.
			funclist[i](i, msg);
		};
	}
	return 0;
}
