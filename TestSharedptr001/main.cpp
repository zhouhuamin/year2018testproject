/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017年8月22日, 下午5:03
 */

#include <cstdlib>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

class A  
{  
    public:  
    A(){}  
    virtual  void mem_func() = 0;  
   protected:  
   virtual ~A()  
   {  
      cout<<"A's dector\n";  
   }  
  
    private:  
  
}; 

class B:public A  
{  
    public:  
    B(){}  
    virtual void mem_func()  
    {  
       cout<<"I'm mem_fun\n";  
    }  
  
}; 

/*
 * 
 */
int main(int argc, char** argv)
{
    boost::shared_ptr<B> sp((B*)0);
    
    if (sp.get())
        sp->mem_func();
    
    return 0;
}

