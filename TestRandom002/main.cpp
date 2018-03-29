/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017年8月7日, 下午3:13
 */

#include <cstdlib>
#include <iostream>

#include <boost/random.hpp>

using namespace std;
using namespace boost;

/*
 * 
 */
int main(int argc, char** argv)
{
    boost::uniform_int<> distribution(1,3);
    boost::mt19937  engine;
    
    boost::variate_generator<boost::mt19937,boost::uniform_int<> > myrandom(engine, distribution);
    
    
    for (int i = 0; i < 10; ++i)
        cout << myrandom() << endl;
    return 0;
}

