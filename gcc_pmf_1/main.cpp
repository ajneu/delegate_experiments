// https://llvm.org/bugs/show_bug.cgi?id=22121#c5

#include <iostream>
#include <cassert>

template<typename Cl, typename Ret, typename... Args>
using signat_t = Ret(*)(Cl *, Args...);
/*
  Examples:
  signat_t<T, void>               = void(*)(T *);
  signat_t<T, int, double, float) =  int(*)(T *, double, float);
*/


struct A {
   char x = 'x';
   int           f_i(int    i) { std::cout << "A::f_i \t   i == " << i << " \tx == " << x << " \tthis == "<< this << std::endl; return i; }
   virtual float f_d(double d) { std::cout << "A::f_d \t   d == " << d << " \tx == " << x << " \tthis == "<< this << std::endl; return d; }
};

struct X { virtual ~X() {} char y; };

struct B : X, A {
   char y = 'y';
   int   f_i(int    i)               { std::cout << "B::f_i \t   i == " << i << " \ty == " << y << " \tthis == "<< this << std::endl; return i; }
   float f_d(double d) /*override*/  { std::cout << "B::f_d \t   d == " << d << " \ty == " << y << " \tthis == "<< this << std::endl; return d; }
};


template<typename Cl>
using fptr_i = signat_t<Cl, int, int>;
//                      ^   ^    ^
//                  Class  Ret  Args

template<typename Cl>
using fptr_d = signat_t<Cl, float, double>;
//                      ^   ^      ^
//                  Class  Ret    Args

#define LOGCMD(cmd) std::cout << "\n" #cmd "\n\t=> "; cmd

template <typename Signature>
class FFF;

B b;

template <typename Ret, typename... Args>
class FFF<Ret(Args...)>
{
private:
   void *inst;
   signat_t<void, Ret, Args...> mfp;

public:
   template<typename Cl>
   static FFF make(Cl *ins, Ret(Cl::*mmm)(Args...)) {
      return FFF{ins, reinterpret_cast<decltype(mfp)>(ins->*mmm)};
   }

   Ret operator()(Args... args) {
      return mfp(inst, std::forward<Args>(args)...);
   }

private:
   FFF(void *ins, decltype(mfp) mmm) : inst{ins}, mfp{mmm}
   {   }

};


int main() {


   {
      auto mydeleg = FFF<int(int)>::make<A>(&b, &A::f_i);
      mydeleg(1);
   }
   {
      auto mydeleg2 = FFF<float(double)>::make<A>(&b, &A::f_d);
      mydeleg2(1.5);
      (reinterpret_cast<signat_t<void, float, double>>(b.*(&A::f_d)))(static_cast<A*>(&b), 1.5);
      //                             cannot do ....   (   (&A::f_d)) ...
   }
   
   using T = void;
   fptr_i<T> mfpA_f_i = reinterpret_cast<decltype(mfpA_f_i)>(&A::f_i);
   fptr_d<T> mfpA_f_d = reinterpret_cast<decltype(mfpA_f_d)>(&A::f_d);

   fptr_i<T> mfpB_f_i = reinterpret_cast<decltype(mfpB_f_i)>(&B::f_i);
   fptr_d<T> mfpB_f_d = reinterpret_cast<decltype(mfpB_f_d)>(&B::f_d);

   std::cout << "&b                  == " << &b << std::endl;
   std::cout << "static_cast<A*>(&b) == " << static_cast<A*>(&b) << std::endl;

   LOGCMD(b.A::f_i(    1));
   LOGCMD(mfpA_f_i(&b, 1)); // ERR!! USE mfpA_f_i(static_cast<A*>(&b), 1)   OR   T = A
   std::cout << std::endl;
   
   LOGCMD(b.f_i(    1));
   LOGCMD(mfpB_f_i(&b, 1));
   std::cout << std::endl;

   LOGCMD(b.A::f_d( 1.5));
   LOGCMD(mfpA_f_d(&b, 1.5)); // ERR!! USE mfpA_f_d(static_cast<A*>(&b), 1.5)   OR   T = A
   std::cout << std::endl;
   
   LOGCMD(b.f_d( 1.5));
   LOGCMD(mfpB_f_d(&b, 1.5));

   std::cout << "......................................" << std::endl;

   A *ap = &b;
   LOGCMD(ap->f_i(  1));
   LOGCMD(mfpA_f_i(ap, 1));
   LOGCMD(ap->f_d(  1.5));
   LOGCMD(mfpA_f_d(ap, 1.5));

   std::cout << "......................................" << std::endl;

   fptr_d<T> mfpB_A_f_d = reinterpret_cast<decltype(mfpB_A_f_d)>(b.*(&A::f_d));
   
   LOGCMD(ap->f_i(  1));
   LOGCMD(mfpA_f_i(ap, 1));
   LOGCMD(ap->f_d(  1.5));
   LOGCMD(mfpB_A_f_d(ap, 1.5));

   return 0;
}
