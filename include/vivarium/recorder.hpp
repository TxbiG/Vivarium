#pragma once
#include "vivarium/vivarium.hpp"
#include <string>
namespace vivarium {
struct EpisodeFrame {std::uint64_t step{0};Observation observation;std::vector<double> action;double reward{0};bool terminated{false},truncated{false};};
struct Episode {std::uint32_t schema_version{1};std::string environment_id;std::uint64_t seed{0};std::vector<EpisodeFrame> frames;};
class EpisodeRecorder {public:EpisodeRecorder(std::string environment_id,std::uint64_t seed);void record(const Observation&,const std::vector<double>&,const StepResult&);const Episode& episode()const noexcept{return episode_;}bool save_json(const std::string&,std::string* error=nullptr)const;private:Episode episode_;};
}
