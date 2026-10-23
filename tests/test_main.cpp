#include "vivarium/vivarium.hpp"
#include "vivarium/scene.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
void check(bool v,const char*m){if(!v){std::cerr<<"FAIL: "<<m<<"\n";std::exit(1);}}
bool same(const vivarium::Observation&a,const vivarium::Observation&b){if(a.size()!=b.size())return false;for(std::size_t i=0;i<a.size();++i)if(a[i]!=b[i])return false;return true;}
void test_scene_serialization(){using namespace vivarium;auto scene=make_default_scene();check(scene.validate(),"default scene validates");const char* path="vivarium_scene_test.json";std::string error;check(scene.save_json(path,&error),"scene JSON saves");SceneDocument loaded;check(loaded.load_json(path,&error),"scene JSON loads");check(loaded.objects.size()==scene.objects.size(),"scene object count roundtrips");check(loaded.objects[1].type==SceneObjectType::Road,"scene object type roundtrips");std::remove(path);}
int main(){using namespace vivarium;test_scene_serialization();BallPush a,b;check(same(a.reset(123),b.reset(123)),"seeded reset");for(int i=0;i<25;++i){auto x=a.step({.2,-.1}),y=b.step({.2,-.1});check(same(x.observation,y.observation),"repeatable actions");check(x.reward==y.reward,"repeatable reward");}Navigate n;check(n.reset(2).size()==6,"Navigate shape");check(n.step({1,1}).observation.size()==6,"Navigate step");PoleBalance p;check(p.reset(2).size()==4,"PoleBalance shape");StackBlocks s;check(s.reset(2).size()==6,"StackBlocks shape");bool caught=false;try{n.step({1});}catch(const std::invalid_argument&){caught=true;}check(caught,"invalid action rejected");bool bad=false;try{WorldConfig2D c;c.fixed_dt=0;PhysicsWorld2D w(c);}catch(const std::invalid_argument&){bad=true;}check(bad,"invalid timestep rejected");PhysicsWorld2D w;w.add_body({{2,2},{0,0},.2,1,0,false,0});w.step();check(w.body(0).position.y<2,"gravity integration");std::cout<<"All Vivarium C++ tests passed.\n";}
