#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <boost/foreach.hpp>
#include <list>

#include "amalgamate/Database.hpp"
#include "amalgamate/Config.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	struct StatisticInfo
	{
		StatisticInfo() { init(); }
		void init() { used = 0; excluded = false; }
		int used;
		bool excluded;
	};

	class Statistics : public map<Descriptor*,StatisticInfo>
	{
	public:
		Statistics(float _weight = 1.0, bool _exclude = false): weight_(_weight), exclude_(_exclude) {}

		float weight() { return weight_; }
		void weight(float _weight) { weight_ = _weight; }

		bool exclude() { return exclude_; } 

		
	private:
		bool exclude_;
		float weight_;
	};

	struct Match
	{
		Match() : 	result(INVALID_MATCH) {}
		Match(float _result, Descriptor* _desc = NULL, Rect _rect = Rect()) 
		{ 
			result = _result; 
			desc = _desc;
			rect = _rect;
		}
		Descriptor* desc;
		Rect rect;
		float result;
	};

	class MatchList : public list<Match>
	{
	public:
		bool addMatch(Match m, size_t maxSize);

		Descriptors descriptors();
		void sortMatches();
	};

	struct DescriptorFilter
	{
		DescriptorFilter(Config* _config = NULL, 
						 Statistics* _statistics = NULL) 
		{  
			config = _config;
			statistics = _statistics; 
		}

		MatchList getMatches(Descriptor& desc, DescriptorType dType, 
							 size_t maxCount, Descriptors& descs,
							 bool sort = false);
		MatchList getMatches(Descriptor& desc, DescriptorType dType, 
							 Descriptors& descs, bool sort = false);
		MatchList getMatches(Descriptor& desc, Descriptors& descs, 
							 bool sort = false);
		Match     getBestMatch(Descriptor& desc, Descriptors& descs);

		Statistics* 		statistics;
		Config* config;

	private:
		size_t descTypeCount(const DescriptorType dType);
	};
}
