//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import <objc/NSObject.h>

@interface WCFinderAbstractTask : NSObject
{
    CDUnknownBlockType _action;
}

@property(copy, nonatomic) CDUnknownBlockType action; // @synthesize action=_action;
- (void).cxx_destruct;
- (void)asyncPreformActionOn:(id)arg1 success:(CDUnknownBlockType)arg2 fail:(CDUnknownBlockType)arg3;
- (void)asyncPreformAction:(CDUnknownBlockType)arg1 fail:(CDUnknownBlockType)arg2;
- (void)preformAction:(CDUnknownBlockType)arg1 fail:(CDUnknownBlockType)arg2;
- (id)initWithAction:(CDUnknownBlockType)arg1;

@end
