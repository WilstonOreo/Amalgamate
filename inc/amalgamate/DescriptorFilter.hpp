#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <boost/foreach.hpp>
#include <list>
#include <set>
#include <tbd/config.h>

#include "amalgamate/Database.hpp"

using namespace std;
using namespace tbd;
using namespace Magick;

namespace amalgamate
{
	struct StatisticInfo
	{
		StatisticInfo(int _used = 0, bool _excluded = false) { init(_used,_excluded); }
		void init(int _used = 0, bool _excluded = false) { lastResult = 1.0; used = _used; excluded = _excluded; }
		int used;
		float lastResult;
		bool excluded;
	};

	class Statistics
	{
	public:
		Statistics(float _weight = 1.0, bool _exclude = false): 
			weight_(_weight), exclude_(_exclude) {}

		float weight() { return weight_; }
		void weight(float _weight) { weight_ = _weight; }
		bool exclude() { return exclude_; }
		void exclude(Descriptor* desc, bool _exclude = true)
		{
			if (!desc->statInfo)
				desc->statInfo = new StatisticInfo(0,_exclude);
			else
				desc->statInfo->excluded = _exclude;
		}
		
		bool excluded(Descriptor* desc) 
		{
			if (desc) 
				if (desc->statInfo) 
					return desc->statInfo->excluded; 
			return false;
		} 

	private:
		float weight_;
		bool exclude_;
	};

	struct Match
	{
		Match() : 	result(INVALID_MATCH) {}
		Match(float _result, Descriptor* _desc = NULL) 
		{ 
			result = _result; 
			desc = _desc;
		}
		Descriptor* desc;
		float result;
	};

	struct CompareMatch 
	{
  		bool operator() (const Match& a, const Match& b) const
  			{return a.result<b.result;}
	};

	class Matches : public set<Match,CompareMatch>
	{
	public:
		Matches(size_t _maxSize = 0): maxMatch(INVALID_MATCH), maxSize_(_maxSize) {}

		size_t maxSize() { return maxSize_; }
		void maxSize(size_t _maxSize) { maxSize_=_maxSize; }
		bool addMatch(const Match& m);

		Descriptors descriptors();
	private:
		float maxMatch;
		size_t maxSize_;
	};

	struct DescriptorFilter : public ConfigurableObject
	{
		DescriptorFilter(Config* _config = NULL, 
						 Statistics* _statistics = NULL) :
			ConfigurableObject(_config)
		{  
			statistics = _statistics; 
		}

		TBD_DECLARE_PROPERTY_CFG(size_t,histSmallCount,"FILTER_HISTSMALL_MATCHES",1000);
		TBD_DECLARE_PROPERTY_CFG(size_t,histLargeCount,"FILTER_HISTLARGE_MATCHES",250);
		TBD_DECLARE_PROPERTY_CFG(size_t,gistCount, 	   "FILTER_GIST_MATCHES", 	  100);
		TBD_DECLARE_PROPERTY_CFG(size_t,thumbnailCount,"FILTER_THUMBNAIL_MATCHES",100);

		Matches getMatches(Descriptor& desc, DescriptorType dType, 
							 size_t maxCount, Descriptors& descs);
		Matches getMatches(Descriptor& desc, DescriptorType dType, 
							 Descriptors& descs);
		Matches getMatches(Descriptor& desc, Descriptors& descs);
		Match     getBestMatch(Descriptor& desc, Descriptors& descs);

		Statistics* 		statistics;

	private:
		size_t descTypeCount(const DescriptorType dType);
	};
}
