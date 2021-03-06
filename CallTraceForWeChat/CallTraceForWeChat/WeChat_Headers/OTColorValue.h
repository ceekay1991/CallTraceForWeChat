//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import <objc/NSObject.h>

@interface OTColorValue : NSObject
{
    double _red;
    double _green;
    double _blue;
    double _weight;
}

+ (id)colorValueWith:(id)arg1;
@property(nonatomic) double weight; // @synthesize weight=_weight;
@property(nonatomic) double blue; // @synthesize blue=_blue;
@property(nonatomic) double green; // @synthesize green=_green;
@property(nonatomic) double red; // @synthesize red=_red;
- (id)color;
- (void)meanWithColorValue:(id)arg1;
- (double)distanceWith:(id)arg1;
- (id)initWithRed:(double)arg1 green:(double)arg2 blue:(double)arg3;

@end

