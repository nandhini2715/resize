/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
 // Copyright (C) 2009-2010, Willow Garage Inc., all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

#include "precomp.hpp"
namespace cv
{

DynamicAdaptedFeatureDetector::DynamicAdaptedFeatureDetector(const Ptr<AdjusterAdapter>& a,
                                         int min_features, int max_features, int max_iters,
                                         bool save_adjusted_parameters) :
        escape_iters_(max_iters), min_features_(min_features), max_features_(max_features),
        adjuster_(a == 0 ? 0 : a->clone()), save_adjusted_parameters_(save_adjusted_parameters), call_const_impl_(true)
{}

bool DynamicAdaptedFeatureDetector::empty() const
{
    return adjuster_.empty() || adjuster_->empty();
}

Ptr<DynamicAdaptedFeatureDetector> DynamicAdaptedFeatureDetector::clone() const
{
    Ptr<DynamicAdaptedFeatureDetector> cloned_obj = new DynamicAdaptedFeatureDetector(adjuster_, min_features_, max_features_, escape_iters_, save_adjusted_parameters_);
    return cloned_obj;
}

void DynamicAdaptedFeatureDetector::detect( const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask )
{
    //TODO: is this construction thread-safe in case of save_adjusted_parameters == false?
    //otherwise use if condition and do assigment iff saved_adjusted_parameters == true
    call_const_impl_ = !save_adjusted_parameters_;
    DynamicAdaptedFeatureDetector const *this_const = this;
    this_const->detect(image, keypoints, mask);
    //the same as FeatureDetector::detect(image, keypoints, mask); (if FeatureDetector doesn't have non-const detect)
    //TODO: is this construction thread-safe?
    call_const_impl_ = true;
}

void DynamicAdaptedFeatureDetector::detectImpl( const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask )
{
    //for oscillation testing
    bool down = false;
    bool up = false;

    //flag for whether the correct threshhold has been reached
    bool thresh_good = false;

    //break if the desired number hasn't been reached.
    int iter_count = escape_iters_;

    while( iter_count > 0 && !(down && up) && !thresh_good && adjuster_->good() )
    {
        keypoints.clear();

        //the adjuster takes care of calling the detector with updated parameters
        adjuster_->detect(image, keypoints,mask);

        if( int(keypoints.size()) < min_features_ )
        {
            down = true;
            adjuster_->tooFew(min_features_, (int)keypoints.size());
        }
        else if( int(keypoints.size()) > max_features_ )
        {
            up = true;
            adjuster_->tooMany(max_features_, (int)keypoints.size());
        }
        else
            thresh_good = true;

        iter_count--;
    }
}

void DynamicAdaptedFeatureDetector::detectImpl(const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask) const
{
    if (call_const_impl_ && save_adjusted_parameters_)
    {
        string errorMessage = "You are trying to save adjusted parameters into a constant detector."
                "It is not possible to modify a constant object. Please use save_adjusted_parameters=false or use a non-constant detector";
        CV_Error(CV_StsBadArg, errorMessage);
    }

    if (call_const_impl_)
    {
        CV_Assert(!save_adjusted_parameters_);

        Ptr<DynamicAdaptedFeatureDetector> cloned_obj = clone();
        cloned_obj->detectImpl(image, keypoints, mask);
    }
    else
    {
        //Of course, const_cast is bad.
        //However, it will be applied if only non-const detect() is called
        //so it doesn't violate const correctness of public API.
        DynamicAdaptedFeatureDetector *this_nonconst = const_cast<DynamicAdaptedFeatureDetector*>(this);
        this_nonconst->detectImpl(image, keypoints, mask);
    }
}

FastAdjuster::FastAdjuster( int init_thresh, bool nonmax, int min_thresh, int max_thresh ) :
    thresh_(init_thresh), nonmax_(nonmax), init_thresh_(init_thresh),
    min_thresh_(min_thresh), max_thresh_(max_thresh)
{}

void FastAdjuster::detectImpl(const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask) const
{
    FastFeatureDetector(thresh_, nonmax_).detect(image, keypoints, mask);
}

void FastAdjuster::tooFew(int, int)
{
    //fast is easy to adjust
    thresh_--;
}

void FastAdjuster::tooMany(int, int)
{
    //fast is easy to adjust
    thresh_++;
}

//return whether or not the threshhold is beyond
//a useful point
bool FastAdjuster::good() const
{
    return (thresh_ > min_thresh_) && (thresh_ < max_thresh_);
}

Ptr<AdjusterAdapter> FastAdjuster::clone() const
{
    Ptr<AdjusterAdapter> cloned_obj = new FastAdjuster( init_thresh_, nonmax_, min_thresh_, max_thresh_ );
    return cloned_obj;
}

StarAdjuster::StarAdjuster(double initial_thresh, double min_thresh, double max_thresh) :
    thresh_(initial_thresh), init_thresh_(initial_thresh),
    min_thresh_(min_thresh), max_thresh_(max_thresh)
{}

void StarAdjuster::detectImpl(const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask) const
{
    StarFeatureDetector detector_tmp(16, cvRound(thresh_), 10, 8, 3);
    detector_tmp.detect(image, keypoints, mask);
}

void StarAdjuster::tooFew(int, int)
{
    thresh_ *= 0.9;
    if (thresh_ < 1.1)
            thresh_ = 1.1;
}

void StarAdjuster::tooMany(int, int)
{
    thresh_ *= 1.1;
}

bool StarAdjuster::good() const
{
    return (thresh_ > min_thresh_) && (thresh_ < max_thresh_);
}

Ptr<AdjusterAdapter> StarAdjuster::clone() const
{
    Ptr<AdjusterAdapter> cloned_obj = new StarAdjuster( init_thresh_, min_thresh_, max_thresh_ );
    return cloned_obj;
}

SurfAdjuster::SurfAdjuster( double initial_thresh, double min_thresh, double max_thresh ) :
    thresh_(initial_thresh), init_thresh_(initial_thresh),
    min_thresh_(min_thresh), max_thresh_(max_thresh)
{}

void SurfAdjuster::detectImpl(const Mat& image, vector<KeyPoint>& keypoints, const cv::Mat& mask) const
{
    Ptr<FeatureDetector> surf = FeatureDetector::create("SURF");
    surf->set("hessianThreshold", thresh_);
    surf->detect(image, keypoints, mask);
}

void SurfAdjuster::tooFew(int, int)
{
    thresh_ *= 0.9;
    if (thresh_ < 1.1)
            thresh_ = 1.1;
}

void SurfAdjuster::tooMany(int, int)
{
    thresh_ *= 1.1;
}

//return whether or not the threshhold is beyond
//a useful point
bool SurfAdjuster::good() const
{
    return (thresh_ > min_thresh_) && (thresh_ < max_thresh_);
}

Ptr<AdjusterAdapter> SurfAdjuster::clone() const
{
    Ptr<AdjusterAdapter> cloned_obj = new SurfAdjuster( init_thresh_, min_thresh_, max_thresh_ );
    return cloned_obj;
}

Ptr<AdjusterAdapter> AdjusterAdapter::create( const string& detectorType )
{
    Ptr<AdjusterAdapter> adapter;

    if( !detectorType.compare( "FAST" ) )
    {
        adapter = new FastAdjuster();
    }
    else if( !detectorType.compare( "STAR" ) )
    {
        adapter = new StarAdjuster();
    }
    else if( !detectorType.compare( "SURF" ) )
    {
        adapter = new SurfAdjuster();
    }

    return adapter;
}

}
