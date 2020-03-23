[TOC]
# CallTraceForWeChat
## 功能介绍
* 1、基于[TimeProfiler](https://github.com/maniackk/TimeProfiler)扩展的支持统计主线程和某一子线程的方法耗时分析工具
    * 可指定线程名对任意子线程进行监听
    * 可视化界面，高亮大于10ms的方法，区分主线程和子线程
    * 可视化界面触发入口去除对UIWindow摇一摇依赖（影响传感器api）
    * log输出优化
    * 增加oc接口层方便切换其它工具
    
* 2、将该工具通过MonkeyDev工具注入到微信7.0.11中，示例中calltrace起点为微信小程序点击`[WAAppContactPreLoader openApp:taskExtInfo:onSuccess:onFailed:]`，终点为小程序打开完成`[WAWebViewController taskDidOpen]`,demo视频如下
  ![](media/CallTraceForWeChat.gif)

 * 微信示例小程序启动过程中主线程耗时超过0.5s的方法callTrace记录
 
 <details>
<summary>展开查看</summary>

 ```
 总耗时：859 ms调用顺序排序=========printMethodRecord==Start================
深度 | 耗时 | 次数 | 线程 |方法名|
  0|   93.94 |  1|  主| -[WAAppContactPreLoader _openApp:taskExtInfo:onSuccess:onFailed:]
  1|   93.92 |  1|  主|   -[WAAppContactPreLoader openApp:taskExtInfo:handlerWrapper:]
  2|   93.72 |  1|  主|     -[WAAppContactPreLoader firstLoadWeAppContact]
  3|    1.17 |  1|  主|       -[WAAppContactPreLoader isNeedSyncUpdateContact:getReason:]
  3|   91.55 |  1|  主|       -[WALaunchResourceFetcher startFetchLaunchResourceWithOpenInfo:version:taskExtInfo:contactUpdateHandler:]
  4|   91.11 |  1|  主|         +[WAContactGetter getWeAppContact:version:handler:timeout:]
  5|   91.10 |  1|  主|           -[WAContactGetter getWeAppContact:version:handler:timeout:]
  6|   91.10 |  1|  主|             -[WAContactGetter getWeAppContactWithKey:type:version:handler:timeout:]
  7|   91.10 |  1|  主|               -[WAContactGetter mainThread_getWeAppContactWithKey:type:version:handler:timeout:]
  8|   91.08 |  1|  主|                 -[WAContactMainThreadGetter getWeAppContact:getType:version:handler:timeout:]
  9|   90.31 |  1|  主|                   -[WAAppContactPreLoader enterAppWithContact:]
 10|    1.19 |  2|  主|                     -[WABlockCgiMgr isCgi:blockedForAppid:username:inScene:]
 10|    1.63 |  1|  主|                     -[WAContactMgr updateWeAppContact:]
 11|    1.63 |  1|  主|                       -[WAContactMgr updateWeAppContact:isForce:]
 12|    1.62 |  1|  主|                         -[WAContactMgr updateWeAppContactWithKey:updateType:isForce:]
 13|    1.60 |  1|  主|                           -[WAContactMgr mainThread_updateWeAppContact:]
 14|    1.21 |  1|  主|                             -[WAContactMgr checkWxaAttrSyncWaitQueue]
 10|   86.78 |  1|  主|                     -[WAAppContactPreLoader finalyOpenAppWithContact:]
 11|   84.99 |  1|  主|                       -[WAAppTaskMgr openAppTaskWithContact:openInfo:extInfo:handlerWrapper:]
 12|    1.19 |  0|  主|                         -[WABlockCgiMgr isCgi:blockedForAppid:username:inScene:]
 13|    1.12 |  2|  主|                           +[WCSDKAdapter logWithLevel:module:errorCode:file:line:func:format:]
 11|    1.25 |  1|  主|                       +[WAAppItemManager getAppUserStateByUsername:appType:]
 13|    1.66 |  1|  主|                           -[UIDevice setBatteryMonitoringEnabled:]
 12|    4.72 |  1|  主|                         -[WASystemInfo updateRegularSystemInfo]
 12|    2.53 |  1|  主|                         -[WAConfigMgr asyncUpdateWeAppConfigInfo:]
 13|    2.30 |  1|  主|                           -[WADatabaseMgr getConfigInfoByAppID:andType:]
 14|    2.21 |  2|  主|                             -[WCTTable getObjectWhere:]
 15|    1.83 |  1|  主|                               -[WCTSelect nextObject]
 16|    1.54 |  2|  主|                                 -[WCTHandle step]
 12|    1.00 |  1|  主|                         -[WAAppTaskMgr processJumpWeAppBeforeOpenTaskWithAppId:openInfo:extInfo:getOperation:]
 12|   74.51 |  1|  主|                         -[WAAppTask openWithContact:openInfo:taskExtInfo:handlerWrapper:]
 13|   74.50 |  1|  主|                           -[WAAppTask openWithContact:openInfo:taskExtInfo:errorHandler:successHandler:cancelHandler:]
 14|   74.26 |  1|  主|                             -[WAAppTask processSelfPresentedStateOnOpen:Completion:]
 15|   74.14 |  1|  主|                               -[WAAppTask openWithContactAfterProcessPresentState:openInfo:taskExtInfo:errorHandler:successHandler:cancelHandler:PresentingNavigationController:]
 16|    1.29 |  1|  主|                                 -[WAAppTask loadAppConfig:]
 16|    2.24 |  1|  主|                                 -[WAPermissionHandler initWithAppID:userName:debugModeType:appVersion:scene:sessionId:pagePath:externalInfo:dicLaunchParameter:]
 17|    2.14 |  1|  主|                                   -[WAPermissionHandler loadPermissionInfo]
 18|    1.30 |  1|  主|                                     -[WAPermissionMgr getPermissionInfoWithAppID:]
 19|    1.29 |  1|  主|                                       -[WADatabaseMgr getPermissionInfoWithAppID:]
 20|    1.20 |  0|  主|                                         -[WCTTable getObjectWhere:]
 16|   67.10 |  1|  主|                                 -[WAAppTaskLoader loadFirstPageWithDelegate:context:completeHandler:]
 17|   66.99 |  1|  主|                                   -[WAAppTaskLoader runFirstPageWorkFlow]
 18|   66.63 |  1|  主|                                     -[WAAppTaskLoader finalyRunFirstPageWorkFlow]
 19|   66.52 |  4|  主|                                       -[WASerialWorkFlow runWithContext:completeHandler:]
 20|   66.41 |  4|  主|                                         -[WASerialWorkFlow runSteps]
 21|   66.40 |  7|  主|                                           -[WASerialWorkFlow runOneStep:]
 22|   66.40 |  3|  主|                                             -[WASerialWorkFlow runStepSubWorkFlow:]
 23|   66.30 |  2|  主|                                               -[WAAsyncAndGateWorkFlow runWithContext:completeHandler:]
 24|   66.23 |  2|  主|                                                 -[WAAsyncAndGateWorkFlow runSteps]
 25|   13.07 |  4|  主|                                                   -[WAAsyncAndGateWorkFlow runOneStep:]
 26|   13.07 |  3|  主|                                                     -[WAAsyncAndGateWorkFlow runStepSubWorkFlow:]
 27|   12.99 |  0|  主|                                                       -[WASerialWorkFlow runWithContext:completeHandler:]
 28|   12.93 |  0|  主|                                                         -[WASerialWorkFlow runSteps]
 29|   12.92 |  0|  主|                                                           -[WASerialWorkFlow runOneStep:]
 30|   12.92 |  4|  主|                                                             -[WASerialWorkFlow runStepBlock:]
 31|    1.46 |  1|  主|                                                               -[WAAppTaskLoader fetchPreloadAppServiceIfPossibleWithContact:]
 32|    1.46 |  1|  主|                                                                 -[WAAppTask fetchPreloadAppServiceIfPossibleWithContact:]
 31|   11.33 |  3|  主|                                                               -[WASerialWorkFlow onStepFinish:isSuccess:error:]
 32|   11.21 |  0|  主|                                                                 -[WASerialWorkFlow runOneStep:]
 33|   11.21 |  0|  主|                                                                   -[WASerialWorkFlow runStepBlock:]
 34|    6.30 |  1|  主|                                                                     +[WAAppTaskLoader genLoadingVCWithContext:]
 35|    6.28 |  1|  主|                                                                       -[WANavLoadingViewController initWithContact:pagePath:]
 36|    4.47 |  1|  主|                                                                         -[NSKVONotifying_WANavLoadingViewController setupNavMode]
 37|    4.47 |  1|  主|                                                                           -[NSKVONotifying_WANavLoadingViewController initView]
 38|    2.93 |  2|  主|                                                                             -[NSKVONotifying_WANavLoadingViewController initRightMenu:]
 34|    2.45 |  1|  主|                                                                     -[WAUINavigationController initWithRootViewController:]
 34|    1.20 |  1|  主|                                                                     -[NSKVONotifying_WAMainListViewController PresentModalViewController:animated:forceFullScreen:completion:]
 25|   53.16 |  0|  主|                                                   -[WAAsyncAndGateWorkFlow runOneStep:]
 26|   53.16 |  0|  主|                                                     -[WAAsyncAndGateWorkFlow runStepSubWorkFlow:]
 27|   53.11 |  0|  主|                                                       -[WASerialWorkFlow runWithContext:completeHandler:]
 28|   53.07 |  0|  主|                                                         -[WASerialWorkFlow runSteps]
 29|   53.07 |  0|  主|                                                           -[WASerialWorkFlow runOneStep:]
 30|   53.07 |  0|  主|                                                             -[WASerialWorkFlow runStepSubWorkFlow:]
 31|   53.02 |  0|  主|                                                               -[WAAsyncAndGateWorkFlow runWithContext:completeHandler:]
 32|   52.99 |  0|  主|                                                                 -[WAAsyncAndGateWorkFlow runSteps]
 33|   50.56 |  0|  主|                                                                   -[WAAsyncAndGateWorkFlow runOneStep:]
 33|    2.42 |  0|  主|                                                                   -[WAAsyncAndGateWorkFlow runOneStep:]
 34|    2.42 |  0|  主|                                                                     -[WAAsyncAndGateWorkFlow runStepSubWorkFlow:]
 35|    2.38 |  0|  主|                                                                       -[WASerialWorkFlow runWithContext:completeHandler:]
 36|    2.35 |  0|  主|                                                                         -[WASerialWorkFlow runSteps]
 37|    2.35 |  0|  主|                                                                           -[WASerialWorkFlow runOneStep:]
 38|    2.35 |  0|  主|                                                                             -[WASerialWorkFlow runStepBlock:]
 39|    1.37 |  1|  主|                                                                               -[WAPermissionHandler asyncUpdateLaunchPermissionInfo]
 40|    1.32 |  1|  主|                                                                                 -[WAPermissionMgr requestLaunchPermissonInfoWithAppID:userName:debugModeType:appVersion:scene:appServiceType:sessionId:pagePath:isFromBackground:requestType:dicLaunchParameter:isFromParallelLaunch:]
 34|   50.56 |  1|  主|                                                                     -[WAAsyncAndGateWorkFlow runStepBlock:]
 35|   48.98 |  1|  主|                                                                       -[WACacheVersionChecker checkCacheReadyWithContext:completeHandler:progressHandler:]
 36|    1.44 |  1|  主|                                                                         -[WACacheVersionChecker generateInfoDataWithPackageConfig:relativeUrl:]
 37|    1.42 |  1|  主|                                                                           -[WACacheVersionChecker getInfoDataWithPackageConfig:moduleName:]
 36|   46.52 |  1|  主|                                                                         -[WACacheVersionChecker checkLocalCacheWithInfoDataList:getDownloadInfoDataList:]
 37|   46.51 |  1|  主|                                                                           -[WACacheVersionChecker checkLocalCacheOK:]
 38|   18.97 |  1|  主|                                                                             -[WALocalCacheMgr loadPkgInfoLocalCache:]
 38|   27.49 |  1|  主|                                                                             -[WALocalCacheMgr verifyLocalCacheChecksum:]
 39|   27.26 |  1|  主|                                                                               -[OS_dispatch_data MD5]
 39|   18.75 |  1|  主|                                                                               -[WAPackageInfoCacheLogic unpackPkgFromPath:appid:version:isDebugMode:packageType:]
 40|    5.08 |  1|  主|                                                                                 -[__NSDictionaryM description]
 40|    1.05 |  1|  主|                                                                                 -[WAPackageInfoCacheLogic unpackPkgWithFilePath:unpackLib:]

  0|   50.15 |  1|  主| -[WAAsyncAndGateWorkFlow onStepFinish:isSuccess:error:]
  1|   50.01 |  1|  主|   -[WAAsyncAndGateWorkFlow callSuccess]
  2|   49.97 |  1|  主|     -[WAAsyncAndGateWorkFlow callCompleteHandler:error:]
  3|   49.92 |  0|  主|       -[WASerialWorkFlow onStepFinish:isSuccess:error:]
  4|   49.88 |  0|  主|         -[WASerialWorkFlow runOneStep:]
  5|   49.88 |  0|  主|           -[WASerialWorkFlow runStepBlock:]
  6|    5.00 |  1|  主|             -[WAConfigMgr loadAppConfigWithAppID:data:]
  7|    1.42 |  0|  主|               +[WCSDKAdapter logWithLevel:module:errorCode:file:line:func:format:]
  8|    1.36 |  1|  主|                 +[WCSDKAdapter logWithLevel:module:errorCode:file:line:func:message:]
  9|    1.36 |  1|  主|                   +[iConsole logWithLevel:module:errorCode:file:line:func:message:]
 10|    1.35 |  1|  主|                     +[iConsole logToFile:module:file:line:func:message:]
  7|    3.12 |  1|  主|               -[__NSCFString JSONDictionary]
  8|    3.11 |  1|  主|                 -[__NSCFString JSONValue]
  6|   43.81 |  1|  主|             -[WAAppTaskLoader setupAppServiceAndWebViewWithAppID:contact:packageConfig:baseURL:pageRelativeURL:getWebView:]
  7|   43.81 |  1|  主|               -[WAAppTask setupAppServiceAndWebViewWithAppID:contact:packageConfig:baseURL:pageRelativeURL:getWebView:]
  8|    1.06 |  1|  主|                 -[WAAppTask setupAppServiceWithContact:extraInfo:preloadedAppService:]
  8|   42.38 |  1|  主|                 -[WAAppTask getNewWebViewControllerWithContact:appID:baseURL:pageRelativeURL:webviewId:preloadedWebview:]
  9|   40.91 |  1|  主|                   -[NSKVONotifying_WAWebViewController supplyAppInfoWithBaseURL:pageRelativeURL:extraInfo:delegate:]
 10|   21.95 |  1|  主|                     -[NSKVONotifying_WAWebViewController internalInitWithBaseURL:pageRelativeURL:extraInfo:delegate:]
 11|    4.88 |  1|  主|                       -[NSKVONotifying_WAWebViewController updateNavView]
 12|    3.99 |  2|  主|                         -[NSKVONotifying_WAWebViewController setNavBarElementAndStatusBarStyle:]
 13|    3.94 |  3|  主|                           -[WAWebViewMutiFuncMenuView updateMenuViewWithWebContentMode:navMode:]
 14|    3.91 |  3|  主|                             -[WAWebViewMutiFuncMenuView fsnormal_updateContent]
 11|    3.32 |  1|  主|                       -[NSKVONotifying_WAWebViewController setContentNavMode:]
 12|    3.31 |  0|  主|                         -[WAWebViewMutiFuncMenuView updateMenuViewWithWebContentMode:navMode:]
 13|    3.19 |  0|  主|                           -[WAWebViewMutiFuncMenuView fsnormal_updateContent]
 14|    1.39 |  3|  主|                             -[WAWebViewMutiFuncMenuView genMenuView]
 15|    2.29 |  0|  主|                               -[WAWebViewMutiFuncMenuView genMenuView]
 11|   11.19 |  1|  主|                       -[NSKVONotifying_WAWebViewController loadFrameDataWithBaseURL:pageRelativeURL:]
 12|   11.01 |  1|  主|                         -[NSKVONotifying_WAWebViewController setupAttributeWhenKnowsPagePath]
 13|   10.74 |  1|  主|                           -[WAJSContextPlugin_CustomTabBar layoutTabbarByWAWebviewVC:]
 14|   10.71 |  1|  主|                             -[WAJSContextPlugin_CustomTabBar constructByFirstWAWebviewVC:]
 15|    9.17 |  1|  主|                               -[WAJSContextPlugin_CustomTabBar createChildVCTabBarWithItems:mainTabIndex:position:backgroundColor:textColor:selectedTextColor:borderColor:]
 16|    2.72 |  1|  主|                                 -[WeAppCustomTabbar setItems:]
 16|    4.16 |  1|  主|                                 -[WAJSContextPlugin_CustomTabBar genTabbarItemsWithtextColor:selectedTextColor:]
 16|    1.03 |  1|  主|                                 -[YYWAWebView setHeight:]
 17|    1.02 |  1|  主|                                   -[YYWAWebView setFrame:]
 10|   18.69 |  1|  主|                     -[NSKVONotifying_WAWebViewController tryLoadPreloadAppTaskIndexData]
 11|    5.38 |  1|  主|                       -[NSKVONotifying_WAWebViewController injectWebViewConfigForPreload]
 12|    2.39 |  1|  主|                         -[WAWebViewJSLogicImpl getWebViewLaunchConfig]
 13|    2.32 |  1|  主|                           -[WAJSCoreService getAppLaunchConfig]
 14|    1.65 |  1|  主|                             -[WAConfigMgr appConfigJsonTextForAppService:]
 15|    1.27 |  1|  主|                               -[__NSDictionaryM JSONRepresentation]
 12|    2.96 |  1|  主|                         -[YYWAWebView evaluateJavaScriptFromString:completionBlock:]
 13|    2.96 |  1|  主|                           -[YYWAWebView evaluateJavaScript:completionHandler:]
 11|   11.74 |  1|  主|                       -[NSKVONotifying_WAWebViewController extractAndInjectPageFrameJSForFullPkg]
 12|   10.83 |  1|  主|                         -[NSKVONotifying_WAWebViewController getPageFrameJSStr]
 13|    5.12 |  1|  主|                           -[NSPlaceholderString initWithData:encoding:]
 13|    5.18 |  1|  主|                           -[NSKVONotifying_WAWebViewController allElementStringFromHtml:elementName:containElementName:]
 14|    4.62 |  1|  主|                             -[__NSCFString rangeOfString:options:range:]
 11|    1.32 |  1|  主|                       -[NSKVONotifying_WAWebViewController loadIndexData]
  9|    1.28 |  1|  主|                   -[NSKVONotifying_WAWebViewController setTitle:]

  0|    7.93 |  1|  主| -[UILayoutContainerView mmLayoutSublayersOfLayer:]

  0|    1.00 |  1|  主| -[UIView mmLayoutSublayersOfLayer:]

  0|   27.21 |  1|  主| -[CMainControll performSelector:withObject:]
  1|   27.02 |  1|  主|   -[CAppObserverCenter NotifyFromMainCtrl:Event:]
  2|   26.96 |  1|  主|     -[WACgiProxy MessageReturn:Event:]
  3|   26.89 |  1|  主|       -[WAPermissionMgr onCgiResponse:forEvent:]
  4|   26.82 |  1|  主|         -[WAPermissionMgr onResponseLaunchPermissionInfo:]
  5|   26.77 |  1|  主|           -[WAPermissionMgr processResponseLaunchPermissionInfoWithRequest:andResponse:cgiUserInfo:]
  6|   23.43 |  1|  主|             -[WADatabaseMgr setPermissionInfo:]
  7|   23.26 |  1|  主|               -[WCTTable insertOrReplaceObject:]
  8|   23.24 |  1|  主|                 -[WCTInsert execute]
  9|   23.24 |  1|  主|                   -[WCTInsert realExecute]
 10|   23.02 |  0|  主|                     -[WCTHandle step]
  6|    2.77 |  1|  主|             -[WAPermissionMgr updateAppItem:]
  7|    2.72 |  1|  主|               -[WAAppItemManager onOpenWeAppItem:scene:sessionId:isAlreadyRunning:isUpdateMainFrameTask:]

  0|    9.40 |  1|  主| -[NSKVONotifying_MMTableView mmLayoutSublayersOfLayer:]

  0|   17.84 |  2|  主| -[YYWAWebView userContentController:didReceiveScriptMessage:]
  1|   17.79 |  1|  主|   -[NSKVONotifying_WAWebViewController webviewDidReceiveScriptMessage:handler:rawMessage:]
  2|   17.79 |  1|  主|     -[WAWebViewJSLogicImpl handleJSApiPostMessage:forWebView:]
  3|   17.62 |  1|  主|       -[WAJSEventHandler_setGlobalStorage handleJSEvent:]
  4|   17.42 |  1|  主|         +[WAWebLocalStorage setData:dataType:withKey:storageId:appID:getError:]
  5|   17.40 |  1|  主|           -[WAAppTaskDB runTransaction:]
  6|   17.39 |  1|  主|             -[WCTDatabase runTransaction:]

  0|    1.03 |  0|  主| -[YYWAWebView userContentController:didReceiveScriptMessage:]

  0|    8.83 |  1|  主| -[_UIViewControllerOneToOneTransitionContext completeTransition:]
  1|    3.73 |  1|  主|   -[NSKVONotifying_WAMainListViewController sfViewDidDisappear:]
  2|    3.57 |  2|  主|     +[WCSDKAdapter clickStreamViewDidDisappear:]
  3|    3.51 |  2|  主|       -[WCNewClickStatMgr pageDisappear:]
  4|    3.48 |  2|  主|         -[WCNewClickStatMgr saveWidgetRecordForRecordPage:]
  5|    3.45 |  2|  主|           -[WCWidgetReocordMgr saveWidgetRecordForPage:andTimeStamp:]
  6|    3.22 |  2|  主|             -[NSConcreteMutableData writeToFile:atomically:]

  0|   19.32 |  1|  主| -[WAMainFrameTaskBarView reloadData]
  1|   19.03 |  1|  主|   -[WAMainFrameTaskBarView checkViewNodes]
  2|   18.54 |  1|  主|     -[WAMainFrameTaskBarView calTaskBarHeight]
  3|   18.54 |  1|  主|       -[WAMainFrameTaskBarView getFullScreenHeight]
  4|   18.48 |  1|  主|         -[WAMainFrameTaskBarLogic getFullScreenHeight]
  5|   18.47 |  1|  主|           -[NSKVONotifying_NewMainFrameViewController getFullScreenHeight]
  6|   18.42 |  1|  主|             -[NSKVONotifying_NewMainFrameViewController view]
  7|    4.69 |  1|  主|               -[NSKVONotifying_NewMainFrameViewController initView]
  8|    1.44 |  1|  主|                 -[NSKVONotifying_NewMainFrameViewController initTitle]
  8|    3.16 |  1|  主|                 -[NSKVONotifying_NewMainFrameViewController initTableView]
  9|    1.73 |  1|  主|                   -[MMMainTableView setContentInset:]
 10|    1.37 |  1|  主|                     -[NSKVONotifying_NewMainFrameViewController notifyTaskBarOnMainTableViewDidScroll:]
 11|    1.34 |  1|  主|                       -[NSKVONotifying_NewMainFrameViewController mainTableViewOffsetDidChangeTo:]
  7|   12.51 |  1|  主|               -[NSKVONotifying_NewMainFrameViewController tryDoLazyInitThing]
  8|   12.46 |  1|  主|                 -[NSKVONotifying_NewMainFrameViewController doMainLazyThing]
  9|    3.59 |  2|  主|                   -[NSKVONotifying_NewMainFrameViewController updateUnReadCount]
 10|    1.33 |  2|  主|                     -[MainTabBarViewController setTabBarBadgeValue:forIndex:]
 11|    1.16 |  1|  主|                       -[MMTabbarItem setTitle:]
 11|    1.26 |  1|  主|                       -[NSKVONotifying_NewMainFrameViewController setTitleOnly:]
  9|    6.25 |  1|  主|                   -[NSKVONotifying_NewMainFrameViewController initNavigationTitileView]
 10|    4.90 |  0|  主|                     -[NSKVONotifying_NewMainFrameViewController updateUnReadCount]
 11|    1.86 |  0|  主|                       -[MainTabBarViewController setTabBarBadgeValue:forIndex:]
  9|    1.04 |  1|  主|                   -[NSKVONotifying_NewMainFrameViewController fixMenuBug]
 10|    1.03 |  1|  主|                     -[UIMenuController setMenuVisible:animated:]

  0|    1.82 |  1|  主| -[NSKVONotifying_WANavLoadingViewController callOnLoadingAnimationDoneAfterViewDidAppear]
  1|    1.79 |  1|  主|   -[WAAppTaskLoader onLoadingAnimationDone]
  2|    1.72 |  1|  主|     -[WAAsyncOrGateWorkFlow onStepFinish:isSuccess:error:]
  3|    1.68 |  1|  主|       -[WAAsyncOrGateWorkFlow callSuccess]
  4|    1.65 |  1|  主|         -[WAAsyncOrGateWorkFlow callCompleteHandler:error:]
  5|    1.62 |  0|  主|           -[WASerialWorkFlow onStepFinish:isSuccess:error:]
  6|    1.59 |  0|  主|             -[WASerialWorkFlow runOneStep:]
  7|    1.59 |  0|  主|               -[WASerialWorkFlow runStepSubWorkFlow:]
  8|    1.56 |  1|  主|                 -[WAAsyncOrGateWorkFlow runWithContext:completeHandler:]
  9|    1.53 |  1|  主|                   -[WAAsyncOrGateWorkFlow runSteps]
 10|    1.48 |  1|  主|                     -[WAAsyncOrGateWorkFlow runOneStep:]
 11|    1.48 |  1|  主|                       -[WAAsyncOrGateWorkFlow runStepBlock:]

  1|    4.40 |  1|  主|   -[NSKVONotifying_WAWebViewController setPreloadPageAttribute]
  2|    4.32 |  1|  主|     +[WAWebViewController setNavigationAttribute:webView:]
  3|    2.50 |  0|  主|       -[NSKVONotifying_WAWebViewController setNavBarElementAndStatusBarStyle:]
  4|    2.12 |  0|  主|         -[WAWebViewMutiFuncMenuView updateMenuViewWithWebContentMode:navMode:]
  5|    2.03 |  0|  主|           -[WAWebViewMutiFuncMenuView fsnormal_updateContent]
  6|    1.13 |  0|  主|             -[WAWebViewMutiFuncMenuView genMenuView]
  1|    9.48 |  1|  主|   -[NSKVONotifying_WANavLoadingViewController sfViewDidDisappear:]
  2|    9.31 |  0|  主|     +[WCSDKAdapter clickStreamViewDidDisappear:]
  3|    9.24 |  0|  主|       -[WCNewClickStatMgr pageDisappear:]
  4|    9.20 |  0|  主|         -[WCNewClickStatMgr saveWidgetRecordForRecordPage:]
  5|    9.17 |  0|  主|           -[WCWidgetReocordMgr saveWidgetRecordForPage:andTimeStamp:]
  6|    8.97 |  0|  主|             -[NSConcreteMutableData writeToFile:atomically:]
  1|    1.18 |  0|  主|   -[NSKVONotifying_WANavLoadingViewController initRightMenu:]
=========printMethodRecord==End================

 ```
  </details>
  
## 实现原理
### TimeProfiler
> TimeProfiler：请移步[原作者maniackk](https://github.com/maniackk)的博客[监控所有的OC方法耗时](https://juejin.im/post/5d146490f265da1bc37f2065)。建议关注maniackk的[掘金主页](https://juejin.im/user/5aaf755cf265da23870ea3cf),干货很多。核心要点记录如下，方便源码阅读

#### 1、ARM64经常用到的汇编指令
* MOV    X1，X0         ;将寄存器X0的值传送到寄存器X1
* ADD    X0，X1，X2     ;寄存器X1和X2的值相加后传送到X0
* SUB    X0，X1，X2     ;寄存器X1和X2的值相减后传送到X0

* AND    X0，X0，#0xF    ; X0的值与0xF相位与后的值传送到X0
* ORR    X0，X0，#9      ; X0的值与9相位或后的值传送到X0
* EOR    X0，X0，#0xF    ; X0的值与0xF相异或后的值传送到X0

* LDR    X5，[X6，#0x08]        ；ld：load; X6寄存器加0x08的和的地址值内的数据传送到X5
* LDP  x29, x30, [sp, #0x10]    ; ldp :load pair ; 一对寄存器, 从内存读取数据到寄存器

* STR X0, [SP, #0x8]         ；st:store,str:往内存中写数据（偏移值为正）; X0寄存器的数据传送到SP+0x8地址值指向的存储空间
* STUR   w0, [x29, #-0x8]   ;往内存中写数据（偏移值为负）
* STP  x29, x30, [sp, #0x10]    ;store pair，存放一对数据, 入栈指令

* CBZ  ;比较（Compare），如果结果为零（Zero）就转移（只能跳到后面的指令）
* CBNZ ;比较，如果结果非零（Non Zero）就转移（只能跳到后面的指令）
* CMP  ;比较指令，相当于SUBS，影响程序状态寄存器CPSR

* B   ;跳转指令，可带条件跳转与cmp配合使用
* BL  ;带返回的跳转指令， 返回地址保存到LR（X30）
* BLR  ; 带返回的跳转指令，跳转到指令后边跟随寄存器中保存的地址(例：blr    x8 ;跳转到x8保存的地址中去执行)
* RET   ;子程序返回指令，返回地址默认保存在LR（X30）

####  2、相关介绍的博客
* [arm64程序调用规则](https://juejin.im/post/5d14623ef265da1bb47d7635)

* [arm64 objc_msgSend 源码](https://opensource.apple.com/source/objc4/objc4-723/runtime/Messengers.subproj/objc-msg-arm64.s.auto.html)

* [ARM64汇编学习笔记二(寄存器和函数本质)](https://www.jianshu.com/p/cf29fb303bdc)

####  3、hook objc_msg_send核心思路
* objc_msgSend 方法执行的逻辑是：先获取对象对应类的信息，再获取方法的缓存，根据方法的 selector 查找函数指针，经过异常错误处理后，最后跳到对应函数的实现。
* bbasm_calltrace_fake_objc_msgSend_safe大致逻辑
     *  保存寄存器x0 - x8，q0 - q7到栈上，
     *  记录class，selector，startTime，lr
     *  恢复寄存器x0 - x8，q0 - q7，
     *  调用原始objc_msg_send，
     *  保存寄存器x0 - x8，q0 - q7到栈上，
     *  记录endTime，
     *  恢复寄存器x0 - x8，q0 - q7，以及lr
     
### 逆向
* 1、逆向相关原理可关注[逻辑教育Hank老师的公开课](https://ke.qq.com/course/339445?term_id=100403325&from=41&enter_room=1)

## 环境依赖

* 1、MonkeyDev [安装](https://github.com/AloneMonkey/MonkeyDev/wiki/%E5%AE%89%E8%A3%85) 

* 2、微信7.0.11，可通过[frida-ios-dump](https://github.com/AloneMonkey/frida-ios-dump)砸壳获取
 
* 3、砸壳需要越狱
  * 1、下载[checkra1n](https://assets.checkra.in/downloads/macos/8137a379774904450180054fca898044d3588b8da73edf853e956b1060e5e79b/checkra1n%20beta%200.9.8.2.dmg)
  * 2、终端执行 

    ```
    codesign -f -s - --deep /Applications/checkra1n.app
    ```
  * 3、按提示操作越狱
  
## 其它  

* 1、感谢maniackk对TimeProfiler支持子线程耗时统计提供的技术支持。

* 2、最近打算从代码层面分析下各家小程序（微信、头条、支付宝、百度）的启动性能，探究各家小程序的实现细节和差异，有感兴趣的小伙伴欢迎issure里留言。

## 免责声明
免责声明: 软件仅供技术交流，禁止用于商业及非法用途，如产生法律纠纷与本人无关。

