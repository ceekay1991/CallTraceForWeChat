//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import <objc/NSObject.h>

@class NSString;

@interface GameCenterMsgReportInfo : NSObject
{
    unsigned int _msgSubTye;
    NSString *_noticeId;
    NSString *_extData;
}

@property(retain, nonatomic) NSString *extData; // @synthesize extData=_extData;
@property(retain, nonatomic) NSString *noticeId; // @synthesize noticeId=_noticeId;
@property(nonatomic) unsigned int msgSubTye; // @synthesize msgSubTye=_msgSubTye;
- (void).cxx_destruct;

@end
