//
//  Mat.h
//
//  Created by Giles Payne on 2019/10/06.
//  Copyright © 2019 Xtravision. All rights reserved.
//

#pragma once

#ifdef __cplusplus
#import <opencv2/opencv.hpp>
#endif

#import <Foundation/Foundation.h>

@class CVSize;
@class Scalar;
@class Range;
@class CVRect;
@class CVPoint;

NS_ASSUME_NONNULL_BEGIN
@interface Mat : NSObject

#ifdef __cplusplus
@property(readonly) cv::Mat* nativePtr;
@property(readonly) cv::Mat& nativeRef;
#endif

- (instancetype)init;
- (void)dealloc;
#ifdef __cplusplus
- (instancetype)initWithNativeMat:(cv::Mat*)nativeMat;
+ (instancetype)fromNative:(cv::Mat*)nativeMat;
#endif
- (instancetype)initWithRows:(int)rows cols:(int)cols type:(int)type;
- (instancetype)initWithRows:(int)rows cols:(int)cols type:(int)type data:(NSData*)data;
- (instancetype)initWithSize:(CVSize*)size type:(int)type;
- (instancetype)initWithSizes:(NSArray<NSNumber*>*)sizes type:(int)type;
- (instancetype)initWithRows:(int)rows cols:(int)cols type:(int)type scalar:(Scalar*)scalar;
- (instancetype)initWithSize:(CVSize*)size type:(int)type scalar:(Scalar*)scalar;
- (instancetype)initWithSizes:(NSArray<NSNumber*>*)sizes type:(int)type scalar:(Scalar*)scalar;
- (instancetype)initWithMat:(Mat*)mat rowRange:(Range*)rowRange colRange:(Range*)colRange;
- (instancetype)initWithMat:(Mat*)mat rowRange:(Range*)rowRange;
- (instancetype)initWithMat:(Mat*)mat ranges:(NSArray<Range*>*)ranges;
- (instancetype)initWithMat:(Mat*)mat rect:(CVRect*)roi;
- (Mat*)adjustRoiTop:(int)dtop bottom:(int)dbottom left:(int)dleft right:(int)dright;
- (void)assignTo:(Mat*)mat type:(int)type;
- (void)assignTo:(Mat*)mat;
- (int)channels;
- (int)checkVector:(int)elemChannels depth:(int)depth requireContinuous:(BOOL) requireContinuous;
- (int)checkVector:(int)elemChannels depth:(int)depth;
- (int)checkVector:(int)elemChannels;
- (Mat*)clone;
- (Mat*)col:(int)x;
- (Mat*)colRange:(int)start end:(int)end;
- (Mat*)colRange:(Range*)range;
- (int)dims;
- (int)cols;
- (void)convertTo:(Mat*)mat rtype:(int)rtype alpha:(double)alpha beta:(double)beta;
- (void)convertTo:(Mat*)mat rtype:(int)rtype alpha:(double)alpha;
- (void)convertTo:(Mat*)mat rtype:(int)rtype;
- (void)copyTo:(Mat*)mat;
- (void)copyTo:(Mat*)mat mask:(Mat*)mask;
- (void)create:(int)rows cols:(int)cols type:(int)type NS_SWIFT_NAME(create(rows:cols:type:));
- (void)create:(CVSize*)size type:(int)type NS_SWIFT_NAME(create(size:type:));
- (void)createEx:(NSArray<NSNumber*>*)sizes type:(int)type  NS_SWIFT_NAME(create(sizes:type:));
- (void)copySize:(Mat*)mat;
- (Mat*)cross:(Mat*)mat;
- (int)depth;
- (Mat*)diag:(int)diagonal;
- (Mat*)diag;
+ (Mat*)diag:(Mat*)diagonal;
- (double)dot:(Mat*)mat;
- (long)elemSize;
- (long)elemSize1;
- (BOOL)empty;
+ (Mat*)eye:(int)rows cols:(int)cols type:(int)type NS_SWIFT_NAME(eye(rows:cols:type:));
+ (Mat*)eye:(CVSize*)size type:(int)type NS_SWIFT_NAME(eye(size:type:));
- (Mat*)inv:(int)method;
- (Mat*)inv;
- (BOOL)isContinuous;
- (BOOL)isSubmatrix;
- (void)locateROI:(CVSize*)wholeSize ofs:(CVPoint*)ofs;
- (Mat*)mul:(Mat*)mat scale:(double)scale;
- (Mat*)mul:(Mat*)mat;
+ (Mat*)ones:(int)rows cols:(int)cols type:(int)type NS_SWIFT_NAME(ones(rows:cols:type:));
+ (Mat*)ones:(CVSize*)size type:(int)type NS_SWIFT_NAME(ones(size:type:));
+ (Mat*)onesEx:(NSArray<NSNumber*>*)sizes type:(int)type NS_SWIFT_NAME(ones(sizes:type:));
- (void)push_back:(Mat*)mat;
- (Mat*)reshape:(int)channels rows:(int)rows;
- (Mat*)reshape:(int)channels;
- (Mat*)reshape:(int)channels newshape:(NSArray<NSNumber*>*)newshape;
- (Mat*)row:(int)y;
- (Mat*)rowRange:(int)start end:(int)end;
- (Mat*)rowRange:(Range*)range;
- (int)rows;
- (Mat*)setToScalar:(Scalar*)scalar NS_SWIFT_NAME(setTo(scalar:));
- (Mat*)setToScalar:(Scalar*)scalar mask:(Mat*)mask NS_SWIFT_NAME(setTo(scalar:mask:));
- (Mat*)setToValue:(Mat*)value mask:(Mat*)mask NS_SWIFT_NAME(setTo(value:mask:));
- (Mat*)setToValue:(Mat*)value NS_SWIFT_NAME(setTo(value:));
- (CVSize*)size;
- (int)size:(int)dim;
- (long)step1:(int)dim;
- (long)step1;
- (Mat*)submat:(int)rowStart rowEnd:(int)rowEnd colStart:(int)colStart colEnd:(int)colEnd NS_SWIFT_NAME(submat(rowStart:rowEnd:colStart:colEnd:));
- (Mat*)submat:(Range*)rowRange colRange:(Range*)colRange NS_SWIFT_NAME(submat(rowRange:colRange:));
- (Mat*)submat:(NSArray<Range*>*)ranges NS_SWIFT_NAME(submat(ranges:));
- (Mat*)submatRoi:(CVRect*)roi NS_SWIFT_NAME(submat(roi:));
- (Mat*)t;
- (long)total;
- (int)type;
+ (Mat*)zeros:(int)rows cols:(int)cols type:(int)type;
+ (Mat*)zeros:(CVSize*)size type:(int)type;
+ (Mat*)zerosEx:(NSArray<NSNumber*>*)sizes type:(int)type;
- (NSString*)description;
- (NSString*)dump;
- (int)put:(int)row col:(int)col data:(NSArray<NSNumber*>*)data;
- (int)put:(NSArray<NSNumber*>*)sizes data:(NSArray<NSNumber*>*)data;
- (int)get:(int)row col:(int)col data:(NSMutableArray<NSNumber*>*)data;
- (int)get:(NSArray<NSNumber*>*)sizes data:(NSMutableArray<NSNumber*>*)data;

@end

NS_ASSUME_NONNULL_END
