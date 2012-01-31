#include "amalgamate/descriptor/GIST.hpp"

#include <boost/foreach.hpp>
#include <cmath>
#include "gist/gist.h"

using namespace boost;
using namespace Magick;


namespace amalgamate
{
	namespace descriptor 
	{

	void GIST::build(YUVImage& yuvImage)
	{
		orgWidth(yuvImage.orgWidth()); orgHeight(yuvImage.orgHeight());
		
		int nblocks=4;
  		int n_scale=3;
  		int orientations_per_scale[3]={8,8,4};
		
		#define R 4
		int w = IMAGE_SIZE/R;
		color_image_t im; int nPix = w*w;
		im.width = im.height = w;
		im.c1 = (float*)malloc(nPix*sizeof(float));
		im.c2 = (float*)malloc(nPix*sizeof(float));
		im.c3 = (float*)malloc(nPix*sizeof(float));

		float invP = 1.0f/255.0f;

		int X,Y; FOR_2D(X,Y,w)
		{
			int pos = Y*w+X;

			int pixY = 0,pixU = 0,pixV = 0;
			int xx,yy; FOR_2D(xx,yy,R)
			{
				pixY += yuvImage.y(X*R+xx,Y*R+yy);
				pixU += yuvImage.u(X*R+xx,Y*R+yy);
				pixV += yuvImage.v(X*R+xx,Y*R+yy);
			}
			
			im.c1[pos] = (pixY/R/R)*invP;
			im.c2[pos] = (pixU/R/R)*invP;
			im.c3[pos] = (pixV/R/R)*invP;
		}

		float *desc = color_gist_scaletab(&im,nblocks,n_scale,orientations_per_scale);

		/* compute descriptor size */
		size_t descsize = 0;
  		for(int i=0;i<n_scale;i++) 
			descsize+=nblocks*nblocks*orientations_per_scale[i];
  		descsize*=3; /* color */

		desc_.resize(descsize);
		for(size_t i = 0; i < descsize; i++)
			desc_[i] = CLIP_u16(int(desc[i]*0.5*65535));
		
		free(desc);
		free(im.c1); free(im.c2); free(im.c3);
	}

	float GIST::compare(const GIST& gist)
	{
		if (desc_.size()!=gist.desc_.size() || !gist.desc_.size() || !desc_.size())
			return INVALID_MATCH;

		int sum = 0;
		for (size_t i = 0; i < desc_.size(); i++)
		{
			u32 diff = desc_[i] - gist.desc_[i];
			sum += (diff*diff) >> 8;
		}
		sum >>= 8;
		return float(sum)/(65536.0*float(desc_.size()));
	}

	}
}
