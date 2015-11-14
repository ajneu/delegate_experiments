#include <iostream>
#include <functional>


template<typename T>
class Delegate;

// from http://blog.coldflake.com/posts/C++-delegates-on-steroids/#truly-generic-delegate
// - expanded to allow delegate to hold class member functions AND normal static functions

template<typename return_type, typename... params>
class Delegate<return_type(params...)>
{
   typedef return_type (*Type)(void* callee, params...);
public:
   Delegate(void* callee, Type function)
      : fpCallee(callee)
      , fpCallbackFunction(function) {}
   template <class T, return_type (T::*TMethod)(params...)>
   static Delegate from_function(T* callee)
   {
      Delegate d(callee, &methodCaller<T, TMethod>);
      return d;
   }

   template <return_type (*TMethod)(params...)>
   static Delegate from_function()
   {
      Delegate d(nullptr, &staticMethodCaller<TMethod>);
      return d;
   }

   return_type operator()(params... xs) const
   {
      return (*fpCallbackFunction)(fpCallee, xs...);
   }
private:
   void* fpCallee;
   Type fpCallbackFunction;
   template <class T, return_type (T::*TMethod)(params...)>
   static return_type methodCaller(void* callee, params... xs)
   {
      T* p = static_cast<T*>(callee);
      return (p->*TMethod)(xs...);
   }
   template <return_type (*TMethod)(params...)>
   static return_type staticMethodCaller(void* callee, params... xs)
   {
      return (*TMethod)(xs...);
   }

};

int mydoub(int i)
{
   return i + i;
}

class listener {
public:
   void msgReceived(int len) { std::cout << "len: " << len << std::endl; }
   int doub(int i) { return i+i; }
};

int main(int argc, char const* argv[]) {
    listener l;
    {
       Delegate<void(int)> x = Delegate<void(int)>::from_function<listener, &listener::msgReceived>(&l);
       x(33);
    }
    {
       Delegate<int(int)> x = Delegate<int(int)>::from_function<mydoub>();
       std::cout << x(3) << std::endl;
    }
    return 0;
}
