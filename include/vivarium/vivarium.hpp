#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace vivarium {
struct Vec2 { double x{0}, y{0}; Vec2 operator+(Vec2 b) const{return{x+b.x,y+b.y};} Vec2 operator-(Vec2 b) const{return{x-b.x,y-b.y};} Vec2 operator*(double s)const{return{x*s,y*s};} Vec2& operator+=(Vec2 b){x+=b.x;y+=b.y;return *this;} };
using Observation=std::vector<double>;
using InfoValue=std::variant<double,std::int64_t,bool,std::string>;
using Info=std::map<std::string,InfoValue>;
struct StepResult { Observation observation; double reward{0}; bool terminated{false},truncated{false}; Info info; bool done()const noexcept{return terminated||truncated;} };
struct EnvironmentSpec {std::string id,description;std::size_t observation_size{0},action_size{0};};
class Rng { public: explicit Rng(std::uint64_t seed=1){reseed(seed);} void reseed(std::uint64_t s)noexcept{state_=s?s:0x9E3779B97F4A7C15ULL;} std::uint64_t next()noexcept{state_^=state_>>12;state_^=state_<<25;state_^=state_>>27;return state_*2685821657736338717ULL;} double uniform()noexcept{return double(next()>>11)*(1.0/9007199254740992.0);} double uniform(double a,double b)noexcept{return a+(b-a)*uniform();} private: std::uint64_t state_{1};};
class Environment {public: virtual ~Environment()=default; virtual Observation reset(std::uint64_t seed)=0; virtual StepResult step(const std::vector<double>& action)=0; virtual Observation observe()const=0; virtual EnvironmentSpec spec()const=0; virtual void close()noexcept{closed_=true;} protected: bool closed_{false};};
struct Body2D {Vec2 position{},velocity{};double radius{.25},mass{1},restitution{.1};bool fixed{false};std::uint64_t id{0};};
struct WorldConfig2D {Vec2 gravity{0,-9.81};double width{10},height{10},fixed_dt{1.0/60.0},max_speed{100};};
class PhysicsWorld2D {public: explicit PhysicsWorld2D(WorldConfig2D config={});void set_config(const WorldConfig2D&);const WorldConfig2D& config()const noexcept{return config_;}void clear();std::size_t add_body(Body2D);Body2D& body(std::size_t);const Body2D& body(std::size_t)const;std::size_t body_count()const noexcept{return bodies_.size();}void step();private:WorldConfig2D config_;std::vector<Body2D>bodies_;std::uint64_t next_id_{1};void bounds(Body2D&);void collide(Body2D&,Body2D&);};
class BallPush final:public Environment {public:explicit BallPush(std::size_t max_steps=500);Observation reset(std::uint64_t)override;StepResult step(const std::vector<double>&)override;Observation observe()const override;EnvironmentSpec spec()const override;private:PhysicsWorld2D world_;Rng rng_;std::size_t limit_,steps_{0};Vec2 target_{9,3};};
class Navigate final:public Environment {public:explicit Navigate(std::size_t max_steps=300);Observation reset(std::uint64_t)override;StepResult step(const std::vector<double>&)override;Observation observe()const override;EnvironmentSpec spec()const override;private:Rng rng_;Vec2 position_{.5,.5},target_{9,9};std::size_t limit_,steps_{0};};
class PoleBalance final:public Environment {public:explicit PoleBalance(std::size_t max_steps=1000);Observation reset(std::uint64_t)override;StepResult step(const std::vector<double>&)override;Observation observe()const override;EnvironmentSpec spec()const override;private:Rng rng_;double angle_{0},angular_velocity_{0},cart_position_{0},cart_velocity_{0};std::size_t limit_,steps_{0};};
class StackBlocks final:public Environment {public:explicit StackBlocks(std::size_t max_steps=30);Observation reset(std::uint64_t)override;StepResult step(const std::vector<double>&)override;Observation observe()const override;EnvironmentSpec spec()const override;private:Rng rng_;std::size_t limit_,steps_{0},height_{0};double top_x_{5},placement_x_{5};};
std::shared_ptr<Environment> make_environment(const std::string& id);
}
