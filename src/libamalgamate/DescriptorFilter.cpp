#include "amalgamate/DescriptorFilter.hpp"

#include <list> 
#include <map>
#include <boost/foreach.hpp>
#include "amalgamate/utils.hpp"

using namespace std;
using namespace boost;

namespace amalgamate 
{
	bool MatchList::addMatch(Match m, size_t maxSize)
	{
		MatchList::iterator it = begin();
		bool found = false;

		for (size_t idx = 0; idx < size(); idx++)
		{
			if ((*it).result < m.result) { found = true; break; }
			++it;
		}

		if (!found && size()>maxSize) return false;
		insert(it,m);

		if (size()>maxSize) pop_front();
		return true;
	}

	Descriptors MatchList::descriptors()
	{
		Descriptors result;
		MatchList::iterator it;
		for (it = begin(); it != end(); ++it) 
			result.push_back(it->desc);
		return result;

	}

	bool compareMatch(Match a, Match b)
	{
		return (a.result < b.result);
	}

	void MatchList::sortMatches() 
	{
		sort(compareMatch);
	}

	MatchList DescriptorFilter::getMatches(Descriptor& desc, DescriptorType dType, 
			size_t maxCount, Descriptors& descs, bool sort)
	{
		LOG_MSG_(1) << fmt("Getting matches for %, maxCount: %") % int(dType) % maxCount;
		MatchList result;
		Rect rect;	

		BOOST_FOREACH( Descriptor* pDesc, descs )
		{
			Rect rect;
			float r = INVALID_MATCH; // r = Result from descriptor compare

			switch (dType)
			{
				case DT_HISTSMALL:
					r = desc.histogramSmall().compare(pDesc->histogramSmall()); 
					break;
				case DT_HISTLARGE:
					r = desc.histogramLarge().compare(pDesc->histogramLarge()); 
					break;
				case DT_GIST:
					r = desc.gist().compare(pDesc->gist()); 
					break;
				case DT_THUMBNAIL:
					r = desc.thumbnail().compare(pDesc->thumbnail());
					break;
				default: 
					continue;
			}

			if (statistics) 
			{ 	
				StatisticInfo* info = NULL;
				if (!statistics->count(pDesc))
				{
					info = new StatisticInfo;
					statistics->insert( pair<Descriptor*,StatisticInfo>(pDesc,*info));
				} else
					if (statistics->count(pDesc) == 1)
					{
						info = &statistics->at(pDesc);
					}
					else
						LOG_WRN << fmt("Multiple statistics for one descriptor '%'") % pDesc->filename();

				if (info)
				{
					info->used++;
					r *= (1.0 + info->used * statistics->weight());
					LOG_MSG_(2) << fmt("Statistics: r = %, used = %, %") % r % info->used % statistics->weight();
				} else
				{
					LOG_WRN << "No statistic info for descriptor available.";
				}
			}


			result.addMatch(Match(r,pDesc,rect),maxCount);
		}
		if (sort) result.sortMatches();
		return result;
	}

	size_t DescriptorFilter::descTypeCount(DescriptorType dType)
	{
		size_t count = 0;
		
		switch (dType)
		{
			case DT_HISTSMALL: 
				if (config)  count = config->as<int>("FILTER_HISTSMALL_MATCHES"); 
				else  	count = 1000;
				break;
			case DT_HISTLARGE:
				if (config) count = config->as<int>("FILTER_HISTLARGE_MATCHES"); 
				else 	count = 250;
				break;
			case DT_GIST: 		
				if (config) count = config->as<int>("FILTER_GIST_MATCHES"); 				
				else 	count = 100;
				break;
			case DT_THUMBNAIL:  
				if (config) count = config->as<int>("FILTER_THUMBNAIL_MATCHES"); 				
				else 	count = 100;
				break;
			default: break; 
		}

		return count;
	}
	
	MatchList DescriptorFilter::getMatches(Descriptor& desc, DescriptorType dType, 
			Descriptors& descs, bool sort)
	{
		if (!config) { LOG_ERR << "No config given."; return MatchList(); }

		return getMatches(desc,dType,descTypeCount(dType),descs,sort);
	}

	MatchList DescriptorFilter::getMatches(Descriptor& desc, Descriptors& descs, bool sort)
	{
		if (!config) { LOG_ERR << "No config given."; return MatchList(); }

		MatchList matches;
		Descriptors tmpDescs = descs;
		vector<DescriptorType> descTypes(DT_);

		// Bubble sort descriptor types descending to their count
		size_t n = descTypes.size();
		for (size_t i = 0; i < n; i++) 
			descTypes[i] = DescriptorType(i);

		bool swapped;
		do
		{
			swapped = false;
			for (int i = 0; i < n-1; i++)
				if (descTypeCount(descTypes[i]) < descTypeCount(descTypes[i+1]))
				{
					swap(descTypes[i],descTypes[i+1]);
					swapped = true;
				}
			n--;
		}
		while (swapped && n > 1);

		// Delete every element with a count of 0
		for (size_t i = descTypes.size()-1; i > 0; i--)
			if (!descTypeCount(descTypes[i]))
				descTypes.erase(descTypes.begin()+i);

		BOOST_FOREACH(DescriptorType dType, descTypes)
			LOG_MSG_(2) << fmt("count = %, id = %") % descTypeCount(dType) % dType; 

		if (descTypes.empty())
		{
			LOG_ERR << "Config error: Amount of matches is zero for all descriptor types";
			return MatchList();
		}

		BOOST_FOREACH(DescriptorType dType, descTypes)
		{
			matches = getMatches(desc,dType,tmpDescs,sort);
			tmpDescs = matches.descriptors();
		}
		return matches;
	}

	Match DescriptorFilter::getBestMatch(Descriptor& desc, Descriptors& descs)
	{
		MatchList matches = getMatches(desc,descs,true);
		LOG_MSG << fmt("Found % matches.") % matches.size();

		Match result = matches.front();

		if (statistics)
		{
			if (statistics->exclude())
			{		
				while (statistics->at(result.desc).excluded && !matches.empty())
				{
					matches.pop_front();
					result = matches.front();
				}

				if (!matches.empty())
				{
					statistics->at(result.desc).excluded = true;
				}
			}
		}
		return result;
	}

}

