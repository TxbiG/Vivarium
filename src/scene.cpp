#include "vivarium/scene.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <unordered_set>

namespace vivarium { namespace {
bool finite(double x) { return std::isfinite(x); }
std::string esc(const std::string& s) {
    std::string o; for(char c:s) { switch(c) { case '"': o += "\\\""; break; case '\\': o += "\\\\"; break; case '\n': o += "\\n"; break; case '\r': o += "\\r"; break; case '\t': o += "\\t"; break; default:o.push_back(c); } } return o;
}
std::string unesc(const std::string& s) {
    std::string o; bool e=false; for(char c:s) { if(e) { switch(c){case 'n':o.push_back('\n');break;case 'r':o.push_back('\r');break;case 't':o.push_back('\t');break;default:o.push_back(c);} e=false; } else if(c=='\\')e=true;else o.push_back(c);} if(e)o.push_back('\\');return o;
}
std::string sf(const std::string& s,const char* key,const std::string& fallback="") {
    std::regex re(std::string("\"")+key+"\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\""); std::smatch m; return std::regex_search(s,m,re)?unesc(m[1].str()):fallback;
}
double nf(const std::string& s,const char* key,double fallback) {
    std::regex re(std::string("\"")+key+"\"\\s*:\\s*(-?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?)");std::smatch m;if(!std::regex_search(s,m,re))return fallback;try{return std::stod(m[1].str());}catch(...){return fallback;}
}
bool bf(const std::string& s,const char* key,bool fallback) {
    std::regex re(std::string("\"")+key+"\"\\s*:\\s*(true|false)");std::smatch m;return std::regex_search(s,m,re)?m[1].str()=="true":fallback;
}
void add_obj(SceneDocument& s,SceneObjectType t,const char* n,Vec3 p,Vec3 size,bool fixed=true) {
    SceneObject o;o.id=s.next_id++;o.type=t;o.name=n;o.position=p;o.size=size;o.fixed=fixed;s.objects.push_back(o);
}
}
const char* to_string(SceneObjectType t) noexcept {
    switch(t){case SceneObjectType::Ground:return "ground";case SceneObjectType::Road:return "road";case SceneObjectType::Terrain:return "terrain";case SceneObjectType::Ramp:return "ramp";case SceneObjectType::Wall:return "wall";case SceneObjectType::Obstacle:return "obstacle";case SceneObjectType::RobotSpawn:return "robot_spawn";case SceneObjectType::Goal:return "goal";case SceneObjectType::Sensor:return "sensor";}return "obstacle";
}
bool scene_object_type_from_string(const std::string& s,SceneObjectType& t) noexcept {
    for (auto candidate : {SceneObjectType::Ground, SceneObjectType::Road, SceneObjectType::Terrain,
                           SceneObjectType::Ramp, SceneObjectType::Wall, SceneObjectType::Obstacle,
                           SceneObjectType::RobotSpawn, SceneObjectType::Goal, SceneObjectType::Sensor}) {
        if (s == to_string(candidate)) { t = candidate; return true; }
    }
    return false;
}
std::size_t SceneDocument::add(SceneObjectType t,const std::string& name) {
    SceneObject o;o.id=next_id++;o.type=t;o.name=name.empty()?to_string(t):name;
    switch(t){
    case SceneObjectType::Ground:o.position={world_width*.5,-.25,world_depth*.5};o.size={world_width,.5,world_depth};o.friction=.9;break;
    case SceneObjectType::Road:o.position={world_width*.5,.03,world_depth*.5};o.size={10,.06,4};o.friction=.8;break;
    case SceneObjectType::Terrain:o.position={world_width*.5,.3,world_depth*.5};o.size={12,.6,12};o.friction=.75;break;
    case SceneObjectType::Ramp:o.position={world_width*.5,.8,world_depth*.5};o.size={8,1.6,5};o.friction=.7;break;
    case SceneObjectType::Wall:o.position={world_width*.5,1,world_depth*.5};o.size={1,2,10};break;
    case SceneObjectType::Obstacle:o.position={world_width*.5,.5,world_depth*.5};o.size={2,1,2};break;
    case SceneObjectType::RobotSpawn:o.position={10,.55,10};o.size={1.1,1.1,1.1};o.fixed=false;break;
    case SceneObjectType::Goal:o.position={world_width-10,.6,world_depth-10};o.size={1.2,1.2,1.2};break;
    case SceneObjectType::Sensor:o.position={12,1.3,14};o.size={.35,.35,.35};o.fixed=false;break;
    } objects.push_back(std::move(o));return objects.size()-1;
}
bool SceneDocument::remove(std::size_t i){if(i>=objects.size())return false;objects.erase(objects.begin()+static_cast<std::ptrdiff_t>(i));return true;}
std::size_t SceneDocument::duplicate(std::size_t i){if(i>=objects.size())return objects.size();SceneObject o=objects[i];o.id=next_id++;o.name+=" Copy";o.position.x+=2;o.position.z+=2;objects.push_back(std::move(o));return objects.size()-1;}
bool SceneDocument::validate(std::string* error)const{
 auto fail=[&](const std::string& e){if(error)*error=e;return false;};
 if(schema_version!=1)return fail("Unsupported schema version");
 if(!finite(world_width)||!finite(world_depth)||!finite(gravity_y)||world_width<=0||world_depth<=0||world_width>100000||world_depth>100000)return fail("World dimensions or gravity are invalid");
 std::unordered_set<std::uint64_t> ids;
 for(const auto&o:objects){if(!o.id||!ids.insert(o.id).second)return fail("Object IDs must be non-zero and unique");if(o.name.empty())return fail("Object names must not be empty");double v[]={o.position.x,o.position.y,o.position.z,o.size.x,o.size.y,o.size.z,o.mass,o.friction,o.restitution};for(double x:v)if(!finite(x))return fail("Object properties must be finite");if(o.size.x<=0||o.size.y<=0||o.size.z<=0)return fail("Object size must be positive");if(o.mass<=0||o.friction<0||o.restitution<0||o.restitution>1)return fail("Object physics properties are invalid");}
 return true;
}
bool SceneDocument::save_json(const std::string& path,std::string* error)const{
 if(!validate(error)) return false;
 std::ofstream out(path);
 if(!out){if(error)*error="Could not open scene file for writing";return false;}
 out<<std::setprecision(17)<<"{\n  \"schema_version\": "<<schema_version<<",\n  \"world\": {\"width\": "<<world_width<<", \"depth\": "<<world_depth<<", \"gravity_y\": "<<gravity_y<<"},\n  \"objects\": [\n";
 for(std::size_t i=0;i<objects.size();++i){const auto&o=objects[i];out<<"    {\"id\": "<<o.id<<", \"type\": \""<<to_string(o.type)<<"\", \"name\": \""<<esc(o.name)<<"\", \"x\": "<<o.position.x<<", \"y\": "<<o.position.y<<", \"z\": "<<o.position.z<<", \"sx\": "<<o.size.x<<", \"sy\": "<<o.size.y<<", \"sz\": "<<o.size.z<<", \"mass\": "<<o.mass<<", \"friction\": "<<o.friction<<", \"restitution\": "<<o.restitution<<", \"fixed\": "<<(o.fixed?"true":"false")<<", \"collision_enabled\": "<<(o.collision_enabled?"true":"false")<<"}"<<(i+1<objects.size()?",":"")<<"\n";}
 out<<"  ]\n}\n";if(!out){if(error)*error="Failed while writing scene JSON";return false;}return true;
}
bool SceneDocument::load_json(const std::string& path,std::string* error){
 std::ifstream in(path);if(!in){if(error)*error="Could not open scene file";return false;}std::ostringstream ss;ss<<in.rdbuf();const std::string text=ss.str();if(text.empty()){if(error)*error="Scene file is empty";return false;}
 SceneDocument temp;temp.schema_version=static_cast<std::uint32_t>(nf(text,"schema_version",0));const auto wp=text.find("\"world\""),op=text.find("\"objects\"");if(wp==std::string::npos||op==std::string::npos){if(error)*error="Invalid scene JSON: expected world and objects";return false;}
 const std::string world=text.substr(wp,op-wp);temp.world_width=nf(world,"width",0);temp.world_depth=nf(world,"depth",0);temp.gravity_y=nf(world,"gravity_y",-9.81);
 const auto b=text.find('[',op),e=text.find(']',b==std::string::npos?op:b);if(b==std::string::npos||e==std::string::npos){if(error)*error="Invalid objects array";return false;}const std::string arr=text.substr(b+1,e-b-1);const std::regex obj_re(R"(\{[^{}]*\})");
 for(std::sregex_iterator i(arr.begin(),arr.end(),obj_re),end;i!=end;++i){const std::string r=i->str();SceneObject o;o.id=static_cast<std::uint64_t>(nf(r,"id",0));if(!scene_object_type_from_string(sf(r,"type"),o.type)){if(error)*error="Unknown scene object type";return false;}o.name=sf(r,"name");o.position={nf(r,"x",0),nf(r,"y",0),nf(r,"z",0)};o.size={nf(r,"sx",1),nf(r,"sy",1),nf(r,"sz",1)};o.mass=nf(r,"mass",1);o.friction=nf(r,"friction",.6);o.restitution=nf(r,"restitution",0);o.fixed=bf(r,"fixed",true);o.collision_enabled=bf(r,"collision_enabled",true);temp.next_id=std::max(temp.next_id,o.id+1);temp.objects.push_back(std::move(o));}
 if(!temp.validate(error)) return false;
 *this=std::move(temp);
 return true;
}
SceneDocument make_default_scene(){SceneDocument s;add_obj(s,SceneObjectType::Ground,"Ground",{50,-.25,50},{100,.5,100});add_obj(s,SceneObjectType::Road,"Main Road",{50,.025,48},{70,.05,6});add_obj(s,SceneObjectType::Terrain,"North Terrain",{23,.35,24},{18,.7,18});add_obj(s,SceneObjectType::Ramp,"Test Ramp",{72,.9,72},{10,1.8,6});add_obj(s,SceneObjectType::Wall,"Boundary Wall",{12,1,42},{1,2,30});add_obj(s,SceneObjectType::Obstacle,"Concrete Block",{43,.75,39},{3,1.5,3});add_obj(s,SceneObjectType::RobotSpawn,"Robot Spawn",{16,.55,16},{1.1,1.1,1.1},false);add_obj(s,SceneObjectType::Goal,"Goal Marker",{83,.6,82},{1.2,1.2,1.2});add_obj(s,SceneObjectType::Sensor,"Forward Range Sensor",{18,1.3,18},{.35,.35,.35},false);return s;}
}
