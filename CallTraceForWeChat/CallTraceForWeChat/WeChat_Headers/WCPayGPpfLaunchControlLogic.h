//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import "WCPayControlLogic.h"

#import "WCPayGPLaunchControlLogicDelegate-Protocol.h"
#import "WCPayGppfQueryInfoCgiDelegate-Protocol.h"

@class CContact, NSString, WCPayGppfQueryInfoCgi;
@protocol WCPayGppfLaunchControlLogicDelegate;

@interface WCPayGPpfLaunchControlLogic : WCPayControlLogic <WCPayGppfQueryInfoCgiDelegate, WCPayGPLaunchControlLogicDelegate>
{
    _Bool _bIsLaunchSucc;
    id <WCPayGppfLaunchControlLogicDelegate> _launchDelegate;
    NSString *_appId;
    NSString *_pfOrderNo;
    unsigned long long _pfFromScene;
    CContact *_launchContact;
    WCPayGppfQueryInfoCgi *_queryPfInfoCgi;
}

@property(retain, nonatomic) WCPayGppfQueryInfoCgi *queryPfInfoCgi; // @synthesize queryPfInfoCgi=_queryPfInfoCgi;
@property(retain, nonatomic) CContact *launchContact; // @synthesize launchContact=_launchContact;
@property(nonatomic) _Bool bIsLaunchSucc; // @synthesize bIsLaunchSucc=_bIsLaunchSucc;
@property(nonatomic) unsigned long long pfFromScene; // @synthesize pfFromScene=_pfFromScene;
@property(retain, nonatomic) NSString *pfOrderNo; // @synthesize pfOrderNo=_pfOrderNo;
@property(retain, nonatomic) NSString *appId; // @synthesize appId=_appId;
@property(nonatomic) __weak id <WCPayGppfLaunchControlLogicDelegate> launchDelegate; // @synthesize launchDelegate=_launchDelegate;
- (void).cxx_destruct;
- (void)onLaunchControlLogicStop;
- (void)onLaunchGroupPaySuccWithContact:(id)arg1;
- (void)saveAndRefreshSvrConfigData;
- (void)onGetpfQueryInfoCgiResp:(id)arg1;
- (void)sendQueryPfInfoRequest;
- (void)startLogic;
- (void)stopLogic;
- (void)dealloc;
- (void)setPfLaunchLogicDelegate:(id)arg1;
- (id)initWith3rdAppId:(id)arg1 pfOrderNo:(id)arg2 pfFromScene:(unsigned long long)arg3;

// Remaining properties
@property(readonly, copy) NSString *debugDescription;
@property(readonly, copy) NSString *description;
@property(readonly) unsigned long long hash;
@property(readonly) Class superclass;

@end
