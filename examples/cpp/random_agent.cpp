#include "vivarium/vivarium.hpp"
#include <iostream>
int main(){vivarium::BallPush env;auto obs=env.reset(42);double total=0;int steps=0;for(;steps<500;++steps){auto r=env.step({steps%40<20?.5:-.5,steps%30<15?.2:-.2});obs=r.observation;total+=r.reward;if(r.done())break;}std::cout<<env.spec().id<<" steps="<<steps+1<<" obs="<<obs.size()<<" return="<<total<<"\n";}
