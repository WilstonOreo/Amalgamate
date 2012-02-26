#include "amalgamate/DescriptorFilter.hpp"

#include <list> 
#include <map>
#include <boost/foreach.hpp>
#include "amalgamate/utils.hpp"

using namespace std;
using namespace boost;

namespace amalgamate 
{
	bool Matches::addMatch(const Match& m)
	{
		if (m.result > maxMatch && (size()>=maxSize() || maxSize()==0)) return false;
		insert(m);

		Matches::iterator it = end(); it--;
		if (size()>maxSize() && !empty()) erase(it);
		return true;
	}

	Descriptors Matches::descriptors()
	{
		Descriptors result(size());
		Matches::iterator it; int i = 0;
		for (it = begin(); it != end(); ++it)
		  		{ result[i] = it->desc; i++; }
		return result;
	}

	Matches DescriptorFilter::getMatches(Descriptor& desc, DescriptorType dType, 
			size_t maxCount, Descriptors& descs)
	{
		LOG_MSG_(1) << fmt("Getting matches for %, maxCount: %") % int(dType) % maxCount;
		Matches result(maxCount);

		BOOST_FOREACH( Descriptor* pDesc, descs )
		{
			if (statistics)
				if (statistics->excluded(pDesc)) continue;
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
/*				if (statistics->weight()>0.0)
				{
					if (pDesc->statInfo)
					{
						pDesc->statInfo->used++;
					} else
					{
						pDesc->statInfo = new StatisticInfo();
					}
					r *= (1.0 + pDesc->statInfo->used * statistics->weight());
				}*/
				if (pDesc->statInfo)
				{
					if (pDesc->statInfo->lastResult < 0)
						LOG_MSG << pDesc->statInfo->lastResult;
					r *= pDesc->statInfo->lastResult;
				}
			}
			result.addMatch(Match(r,pDesc));
		}
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
	
	Matches DescriptorFilter::getMatches(Descriptor& desc, DescriptorType dType, 
			Descriptors& descs)
	{
		if (!config) { LOG_ERR << "No config given."; return Matches(0); }

		return getMatches(desc,dType,descTypeCount(dType),descs);
	}

	Matches DescriptorFilter::getMatches(Descriptor& desc, Descriptors& descs)
	{
		if (!config) { LOG_ERR << "No config given."; return Matches(); }

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
			return Matches();
		}

		Matches matches(0);
		Descriptors tmpDescs = descs;

		BOOST_FOREACH(DescriptorType dType, descTypes)
		{
			matches = getMatches(desc,dType,tmpDescs);	
		if (statistics)
			{
				BOOST_FOREACH( const Match& m, matches )
				{
					if (!m.desc) continue;
					if (!m.desc->statInfo)
						m.desc->statInfo = new StatisticInfo();					
					m.desc->statInfo->lastResult = m.result;
				}
			}
			tmpDescs = matches.descriptors();
		}

		BOOST_FOREACH(Descriptor* pDesc, descs)
			if (pDesc->statInfo) pDesc->statInfo->lastResult = 1.0;

		return matches;
	}

	Match DescriptorFilter::getBestMatch(Descriptor& desc, Descriptors& descs)
	{
		Matches matches = getMatches(desc,descs);
		LOG_MSG << fmt("Found % matches.") % matches.size();

		Match result = *matches.begin();
		return result;
	}

}

