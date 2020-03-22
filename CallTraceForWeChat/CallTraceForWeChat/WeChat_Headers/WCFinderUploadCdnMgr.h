//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import <objc/NSObject.h>

#import "ICdnComMgrExt-Protocol.h"
#import "INetworkStatusMgrExt-Protocol.h"

@class NSMutableArray, NSString, WCFinderUploadTask;
@protocol OS_dispatch_queue, WCFinderUploadCdnMgrDelegate;

@interface WCFinderUploadCdnMgr : NSObject <INetworkStatusMgrExt, ICdnComMgrExt>
{
    NSMutableArray *_taskQueue;
    NSMutableArray *_retryQueue;
    NSMutableArray *_deleteQueue;
    NSString *_taskRetryPath;
    NSString *_taskUploadingPath;
    NSString *_taskUploadingShouldDeletePath;
    WCFinderUploadTask *_currentTask;
    id <WCFinderUploadCdnMgrDelegate> _delegate;
    NSObject<OS_dispatch_queue> *_ioQueue;
}

@property(retain, nonatomic) NSObject<OS_dispatch_queue> *ioQueue; // @synthesize ioQueue=_ioQueue;
@property(nonatomic) __weak id <WCFinderUploadCdnMgrDelegate> delegate; // @synthesize delegate=_delegate;
@property(retain, nonatomic) WCFinderUploadTask *currentTask; // @synthesize currentTask=_currentTask;
@property(retain, nonatomic) NSString *taskUploadingShouldDeletePath; // @synthesize taskUploadingShouldDeletePath=_taskUploadingShouldDeletePath;
@property(retain, nonatomic) NSString *taskUploadingPath; // @synthesize taskUploadingPath=_taskUploadingPath;
@property(retain, nonatomic) NSString *taskRetryPath; // @synthesize taskRetryPath=_taskRetryPath;
@property(retain, nonatomic) NSMutableArray *deleteQueue; // @synthesize deleteQueue=_deleteQueue;
@property(retain, nonatomic) NSMutableArray *retryQueue; // @synthesize retryQueue=_retryQueue;
@property(retain, nonatomic) NSMutableArray *taskQueue; // @synthesize taskQueue=_taskQueue;
- (void).cxx_destruct;
- (void)onNetworkStatusChange:(int)arg1;
- (void)OnCdnDownload:(id)arg1;
- (void)OnCdnUpload:(id)arg1;
- (void)OnCdnUploadProgress:(id)arg1;
- (void)OnSetCdnDnsInfo;
- (void)OnCdnInit;
- (void)moveFileToDataItemPath:(id)arg1;
- (void)finderUploadCDNSuccess:(id)arg1;
- (void)startUploadCGIPost;
- (_Bool)checkFileExistsWithTask:(id)arg1;
- (void)startUploadCdnTask;
- (void)startUploadFinderWithDataItem:(id)arg1;
- (void)checkRetryQueue;
- (void)checkQueue;
- (void)syncQueueToLocal;
- (id)getUploadTaskWithTid:(id)arg1 fromQueue:(id)arg2;
- (id)getUploadTaskWithTid:(id)arg1;
- (void)deleteUploadTaskWithTid:(id)arg1;
- (void)deleteTaskInAllQueueWithTid:(id)arg1;
- (_Bool)uploadTaskNeedDeleteWithTid:(id)arg1;
- (_Bool)deleteTaskTid:(id)arg1 inQueue:(id)arg2;
- (_Bool)checkDataItemHasUpload:(id)arg1;
- (_Bool)checkDataItem:(id)arg1 inQueue:(id)arg2;
- (void)finderUploadRetryFail:(id)arg1;
- (void)addRetryQueueWithTask:(id)arg1;
- (void)dealloc;
- (id)initWithDelegate:(id)arg1;

// Remaining properties
@property(readonly, copy) NSString *debugDescription;
@property(readonly, copy) NSString *description;
@property(readonly) unsigned long long hash;
@property(readonly) Class superclass;

@end
