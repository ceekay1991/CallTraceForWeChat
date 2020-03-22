//
//  BBASMTimeProfilerVC.m
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMTimeProfilerVC.h"
#import "BBASMCallTraceInterface.h"
#import "BBASMRecordCell.h"
#import "BBASMRecordModel.h"
#import "BBASMRecordHierarchyModel.h"
#import <objc/runtime.h>
#import "BBASMTimeProfileDataManager.h"

typedef NS_ENUM(NSInteger, BBASMTableType) {
    tableTypeSequential,
    tableTypecostTime,
    tableTypeCallCount,
};

static CGFloat BBASMScrollWidth = 600;
static CGFloat BBASMHeaderHight = 100;

#define BBASMPerformanceLog(str) [BBASMTimeProfilerVC log:str];
bool BBASMTimeProfilerVCHasShow = NO;
@interface BBASMTimeProfilerVC () <UITableViewDataSource, BBASMRecordCellDelegate> {
    BBASMThreadCallRecord *_threadCallRecord;
}

@property (nonatomic, strong)UILabel *titleLable;
@property (nonatomic, strong)UIButton *recordBtn;
@property (nonatomic, strong)UIButton *costTimeSortBtn;
@property (nonatomic, strong)UIButton *callCountSortBtn;
@property (nonatomic, strong)UIButton *popVCBtn;
@property (nonatomic, strong)UIButton *clearBtn;
@property (nonatomic, strong)UITableView *tpTableView;
@property (nonatomic, strong)UILabel *tableHeaderViewLabel;
@property (nonatomic, strong)UIScrollView *tpScrollView;
@property (nonatomic, copy)NSArray *sequentialMethodRecord;
@property (nonatomic, copy)NSArray *costTimeSortMethodRecord;
@property (nonatomic, copy)NSArray *callCountSortMethodRecord;
@property (nonatomic, assign)BBASMTableType tpTableType;

@end

@implementation BBASMTimeProfilerVC
- (instancetype)initWithCallRecord:(BBASMThreadCallRecord *)callRecord {
    self = [super init];
    if (self) {
        BBASMTimeProfilerVCHasShow = YES;
        _threadCallRecord = callRecord;
    }
    return self;
}
- (void)dealloc {
    
}
- (void)viewDidLoad {
    [super viewDidLoad];
    _sequentialMethodRecord = [NSArray array];
    _tpTableType = tableTypeSequential;
    self.view.backgroundColor = [UIColor whiteColor];
    [self.view addSubview:self.titleLable];
    [self.view addSubview:self.recordBtn];
    [self.view addSubview:self.costTimeSortBtn];
    [self.view addSubview:self.callCountSortBtn];
    [self.view addSubview:self.popVCBtn];
    [self.view addSubview:self.clearBtn];
    [self.view addSubview:self.tpScrollView];
    [self.tpScrollView addSubview:self.tableHeaderViewLabel];
    [self.tpScrollView addSubview:self.tpTableView];
    // Do any additional setup after loading the view.
    [self stopAndGetCallRecord];
    NSString *totalTime = [NSString stringWithFormat:@"总耗时：%llu ms \r", [BBASMCallTrace totalCostTime]/1000];
    self.titleLable.text = totalTime;
    BBASMPerformanceLog(totalTime);
    //[BBASMTimeProfileDataManager addData:totalCostTime()/1000];
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [_clearBtn setEnabled:YES];
}
- (NSUInteger)findStartDepthIndex:(NSUInteger)start arr:(NSArray *)arr {
    NSUInteger index = start;
    if (arr.count > index) {
        BBASMRecordModel *model = arr[index];
        int minDepth = model.depth;
        int minTotal = model.total;
        for (NSUInteger i = index+1; i < arr.count; i++) {
            BBASMRecordModel *tmp = arr[i];
            if (tmp.depth < minDepth || (tmp.depth == minDepth && tmp.total < minTotal)) {
                minDepth = tmp.depth;
                minTotal = tmp.total;
                index = i;
            }
        }
    }
    return index;
}

- (NSArray *)recursive_getRecord:(NSMutableArray *)arr {
    if ([arr isKindOfClass:NSArray.class] && arr.count > 0) {
        BOOL isValid = YES;
        NSMutableArray *recordArr = [NSMutableArray array];
        NSMutableArray *splitArr = [NSMutableArray array];
        NSUInteger index = [self findStartDepthIndex:0 arr:arr];
        if (index > 0) {
            [splitArr addObject:[NSMutableArray array]];
            for (int i = 0; i < index; i++) {
                [[splitArr lastObject] addObject:arr[i]];
            }
        }
        BBASMRecordModel *model = arr[index];
        [recordArr addObject:model];
        [arr removeObjectAtIndex:index];
        int startDepth = model.depth;
        int startTotal = model.total;
        for (NSUInteger i = index; i < arr.count; ) {
            model = arr[i];
            if (model.total == startTotal && model.depth-1==startDepth) {
                [recordArr addObject:model];
                [arr removeObjectAtIndex:i];
                startDepth++;
                isValid = YES;
            } else {
                if (isValid) {
                    isValid = NO;
                    [splitArr addObject:[NSMutableArray array]];
                }
                [[splitArr lastObject] addObject:model];
                i++;
            }
            
        }
        
        for (NSUInteger i = splitArr.count; i > 0; i--) {
            NSMutableArray *sArr = splitArr[i-1];
            [recordArr addObjectsFromArray:[self recursive_getRecord:sArr]];
        }
        return recordArr;
    }
    return @[];
}

- (void)setRecordDic:(NSMutableArray *)arr record:(BBASMCallRecord *)record {
    if ([arr isKindOfClass:NSMutableArray.class] && record) {
        int total=1;
        for (NSUInteger i = 0; i < arr.count; i++) {
            BBASMRecordModel *model = arr[i];
            if (model.depth == record->depth) {
                total = model.total+1;
                break;
            }
        }
        if (record->cls && record->sel) {
            BBASMRecordModel *model = [[BBASMRecordModel alloc] initWithCls:record->cls
                                                                        sel:record->sel
                                                                       time:record->costTime
                                                                      depth:record->depth
                                                                      total:total];
            model.isMainThread = record->isMainThread;
            [arr insertObject:model atIndex:0];
        }
    }
}

- (NSMutableArray *)allMethodRecordWithThreadCallRecord:(BBASMThreadCallRecord *)threadCallRecord {
    NSMutableArray *allMethodRecord = [NSMutableArray array];
           int i = 0, j;
           while (i <= threadCallRecord->index) {
               NSMutableArray *methodRecord = [NSMutableArray array];
               for (j = i; j <= threadCallRecord->index;j++) {
                   BBASMCallRecord *callRecord = &threadCallRecord->record[j];
                   [self setRecordDic:methodRecord record:callRecord];
                   if (callRecord->depth==0 || j==threadCallRecord->index) {
                       NSArray *recordModelArr = [self recursive_getRecord:methodRecord];
                       BBASMRecordHierarchyModel *model = [[BBASMRecordHierarchyModel alloc] initWithRecordModelArr:recordModelArr];
                       [allMethodRecord addObject:model];
                       //退出循环
                       break;
                   }
               }
               
               i = j+1;
           }
    return allMethodRecord;
}
- (void)stopAndGetCallRecord {
    [BBASMCallTrace stopTrace];
    if (_threadCallRecord==NULL) {
        return;
    }
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSMutableArray *allMethodRecord = [self allMethodRecordWithThreadCallRecord:self->_threadCallRecord];
        self.sequentialMethodRecord = [[NSArray alloc] initWithArray:allMethodRecord copyItems:YES];
        BBASMPerformanceLog(@"调用顺序排序");
        [self debug_printMethodRecord:self.sequentialMethodRecord];
        dispatch_async(dispatch_get_main_queue(), ^{
            self.tpTableType = tableTypeSequential;
            self.recordBtn.hidden = NO;
            [self clickRecordBtn];
           
        });
        [self sortCostTimeRecord:[[NSArray alloc] initWithArray:allMethodRecord copyItems:YES]];
        [self sortCallCountRecord:[[NSArray alloc] initWithArray:allMethodRecord copyItems:YES]];
    });
}

- (void)debug_printMethodRecord:(NSArray *)array {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self printMethodRecord:array];
    });
}

- (void)printMethodRecord:(NSArray *)array {
    
    @autoreleasepool {
        //记录的顺序是方法完成时间
        BBASMPerformanceLog(@"=========printMethodRecord==Start================");
        [self linefeed];
        NSString *title = @"深度 | 耗时 | 次数 | 线程 |方法名|\r";
        [self log:title];
        [self linefeed];
        for (BBASMRecordHierarchyModel *model in array) {
            if ([model isKindOfClass:[BBASMRecordHierarchyModel class]]) {
                [self printHierarchyModel:model];
            } else {
                [self printRecordModel:(BBASMRecordModel *)model];
            }
            
            [self linefeed];
        }
        //printf("%s", [str UTF8String]);
        BBASMPerformanceLog(@"=========printMethodRecord==End================");
        [self linefeed];
    }
}

- (void)printHierarchyModel:(BBASMRecordHierarchyModel *)model {
    [self linefeed];
    [self printRecordModel:model.rootMethod];
    for (BBASMRecordModel *sub in model.subMethods) {
        [self linefeed];
        [self printRecordModel:sub];
    }
}

- (void)printRecordModel:(BBASMRecordModel *)model {
   
    [self log:[NSString stringWithFormat:@" %2d| ",(int)model.depth]];
    [self log:[NSString stringWithFormat:@" %6.2f |",model.costTime/1000.0]];
    [self log:[NSString stringWithFormat:@" %2d| ",model.callCount]];
    [self log:[NSString stringWithFormat:@" %@| ",model.isMainThread ? @"主" : @"JS"]];
    for (NSUInteger i = 0; i < model.depth; i++) {
       [self log:[NSString stringWithFormat:@"  "]];
    }

   [self log:[NSString stringWithFormat:@"%s[%@ %@]", (class_isMetaClass(model.cls) ? "+" : "-"), model.cls, NSStringFromSelector(model.sel)]];
}

- (void)log:(NSString *)str {
    printf("%s", [str UTF8String]);
}

- (void)linefeed {
    printf("%s", [@"\r" UTF8String]);
}

+ (void)log:(NSString *)str {
    printf("%s", [str UTF8String]);
}


- (NSString *)debug_getMethodCallStr:(BBASMCallRecord *)callRecord {
    NSMutableString *str = [[NSMutableString alloc] init];
    double ms = callRecord->costTime/1000.0;
    [str appendString:[NSString stringWithFormat:@"　%d　|　%lgms　|　", callRecord->depth, ms]];
    if (callRecord->depth>0) {
        [str appendString:[[NSString string] stringByPaddingToLength:callRecord->depth withString:@"　" startingAtIndex:0]];
    }
    if (class_isMetaClass(callRecord->cls)) {
        [str appendString:@"+"];
    } else {
        [str appendString:@"-"];
    }
    [str appendString:[NSString stringWithFormat:@"[%@　　%@]", NSStringFromClass(callRecord->cls), NSStringFromSelector(callRecord->sel)]];
    return str.copy;
}

- (void)sortCostTimeRecord:(NSArray *)arr {
    NSArray *sortArr = [arr sortedArrayUsingComparator:^NSComparisonResult(id  _Nonnull obj1, id  _Nonnull obj2) {
        BBASMRecordHierarchyModel *model1 = (BBASMRecordHierarchyModel *)obj1;
        BBASMRecordHierarchyModel *model2 = (BBASMRecordHierarchyModel *)obj2;
        if (model1.rootMethod.costTime > model2.rootMethod.costTime) {
            return NSOrderedAscending;
        }
        return NSOrderedDescending;
    }];
    for (BBASMRecordHierarchyModel *model in sortArr) {
        model.isExpand = NO;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self.costTimeSortMethodRecord = sortArr;
        self.costTimeSortBtn.hidden = NO;
        
    });
}

- (void)arrAddRecord:(BBASMRecordModel *)model arr:(NSMutableArray *)arr {
    for (int i = 0; i < arr.count; i++) {
        BBASMRecordModel *temp = arr[i];
        if ([temp isEqualRecordModel:model]) {
            temp.callCount++;
            return;
        }
    }
    model.callCount = 1;
    [arr addObject:model];
}

- (void)sortCallCountRecord:(NSArray *)arr {
    NSMutableArray *arrM = [NSMutableArray array];
    for (BBASMRecordHierarchyModel *model in arr) {
        [self arrAddRecord:model.rootMethod arr:arrM];
        if ([model.subMethods isKindOfClass:NSArray.class]) {
            for (BBASMRecordModel *recoreModel in model.subMethods) {
                [self arrAddRecord:recoreModel arr:arrM];
            }
        }
    }
    
    NSArray *sortArr = [arrM sortedArrayUsingComparator:^NSComparisonResult(id  _Nonnull obj1, id  _Nonnull obj2) {
        BBASMRecordModel *model1 = (BBASMRecordModel *)obj1;
        BBASMRecordModel *model2 = (BBASMRecordModel *)obj2;
        if (model1.callCount > model2.callCount) {
            return NSOrderedAscending;
        }
        return NSOrderedDescending;
    }];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.callCountSortMethodRecord = sortArr;
        self.callCountSortBtn.hidden = NO;
    });
}

- (void)clickPopVCBtn:(UIButton *)btn {
    [self dismissViewControllerAnimated:YES completion:^{
        BBASMTimeProfilerVCHasShow = NO;
    }];
}

- (void)clickClearBtn:(UIButton *)btn {
    [BBASMCallTrace clearTrace];
    [btn setEnabled:NO];
    _sequentialMethodRecord = nil;
    _callCountSortMethodRecord = nil;
    _costTimeSortMethodRecord = nil;
    [self.tpTableView reloadData];
}
#pragma mark - BBASMRecordCellDelegate

- (void)recordCell:(BBASMRecordCell *)cell clickExpandWithSection:(NSInteger)section {
    NSIndexSet *indexSet;
    BBASMRecordHierarchyModel *model;
    switch (self.tpTableType) {
        case tableTypeSequential:
            model = self.sequentialMethodRecord[section];
            break;
        case tableTypecostTime:
            model = self.costTimeSortMethodRecord[section];
            break;
            
        default:
            break;
    }
    model.isExpand = !model.isExpand;
    indexSet=[[NSIndexSet alloc] initWithIndex:section];
    [self.tpTableView reloadSections:indexSet withRowAnimation:UITableViewRowAnimationAutomatic];
}

#pragma mark - UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    if (self.tpTableType == tableTypeSequential) {
        return self.sequentialMethodRecord.count;
    } else if (self.tpTableType == tableTypecostTime) {
        return self.costTimeSortMethodRecord.count;
    }
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    if (self.tpTableType == tableTypeSequential) {
        BBASMRecordHierarchyModel *model = self.sequentialMethodRecord[section];
        if (model.isExpand && [model.subMethods isKindOfClass:NSArray.class]) {
            return model.subMethods.count+1;
        }
    }
    else if (self.tpTableType == tableTypecostTime) {
        BBASMRecordHierarchyModel *model = self.costTimeSortMethodRecord[section];
        if (model.isExpand && [model.subMethods isKindOfClass:NSArray.class]) {
            return model.subMethods.count+1;
        }
    }
    else {
        return self.callCountSortMethodRecord.count;
    }
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *BBASMRecordCell_reuseIdentifier = @"BBASMRecordCell_reuseIdentifier";
    BBASMRecordCell *cell = [tableView dequeueReusableCellWithIdentifier:BBASMRecordCell_reuseIdentifier];
    if (!cell) {
        cell = [[BBASMRecordCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:BBASMRecordCell_reuseIdentifier];
    }
    BBASMRecordHierarchyModel *model;
    BBASMRecordModel *recordModel;
    BOOL isShowExpandBtn;
    switch (self.tpTableType) {
        case tableTypeSequential:
            model = self.sequentialMethodRecord[indexPath.section];
            recordModel = [model getRecordModel:indexPath.row];
            isShowExpandBtn = indexPath.row == 0 && [model.subMethods isKindOfClass:NSArray.class] && model.subMethods.count > 0;
            cell.delegate = self;
            [cell bindRecordModel:recordModel isHiddenExpandBtn:!isShowExpandBtn isExpand:model.isExpand section:indexPath.section isCallCountType:NO];
            break;
        case tableTypecostTime:
            model = self.costTimeSortMethodRecord[indexPath.section];
            recordModel = [model getRecordModel:indexPath.row];
            isShowExpandBtn = indexPath.row == 0 && [model.subMethods isKindOfClass:NSArray.class] && model.subMethods.count > 0;
            cell.delegate = self;
            [cell bindRecordModel:recordModel isHiddenExpandBtn:!isShowExpandBtn isExpand:model.isExpand section:indexPath.section isCallCountType:NO];
            break;
        case tableTypeCallCount:
            recordModel = self.callCountSortMethodRecord[indexPath.row];
            [cell bindRecordModel:recordModel isHiddenExpandBtn:YES isExpand:YES section:indexPath.section isCallCountType:YES];
            break;
            
        default:
            break;
    }
    return cell;
}

#pragma mark - Btn click method

- (void)clickRecordBtn {
    self.costTimeSortBtn.selected = NO;
    self.callCountSortBtn.selected = NO;
    if (!self.recordBtn.selected) {
        self.recordBtn.selected = YES;
        self.tpTableType = tableTypeSequential;
        [self.tpTableView reloadData];
    }
}

- (void)clickCostTimeSortBtn {
    self.recordBtn.selected = NO;
    self.callCountSortBtn.selected = NO;
    if (!self.costTimeSortBtn.selected) {
        self.costTimeSortBtn.selected = YES;
        self.tpTableType = tableTypecostTime;
        [self.tpTableView reloadData];
    }
    BBASMPerformanceLog(@"-----最耗时排序--------");
    [self debug_printMethodRecord:_costTimeSortMethodRecord];
}

- (void)clickCallCountSortBtn {
    self.costTimeSortBtn.selected = NO;
    self.recordBtn.selected = NO;
    if (!self.callCountSortBtn.selected) {
        self.callCountSortBtn.selected = YES;
        self.tpTableType = tableTypeCallCount;
        [self.tpTableView reloadData];
    }
    BBASMPerformanceLog(@"-----调用次数排序--------");
    [self debug_printMethodRecord:_callCountSortMethodRecord];
}


#pragma mark - get&set method

- (UIScrollView *)tpScrollView {
    if (!_tpScrollView) {
        _tpScrollView = [[UIScrollView alloc] initWithFrame:CGRectMake(0, BBASMHeaderHight, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height-BBASMHeaderHight)];
        _tpScrollView.showsHorizontalScrollIndicator = YES;
        _tpScrollView.alwaysBounceHorizontal = YES;
        _tpScrollView.contentSize = CGSizeMake(BBASMScrollWidth, 0);
    }
    return _tpScrollView;
}

- (UITableView *)tpTableView {
    if (!_tpTableView) {
        _tpTableView = [[UITableView alloc] initWithFrame:CGRectMake(0, 30, BBASMScrollWidth, [UIScreen mainScreen].bounds.size.height-BBASMHeaderHight-30) style:UITableViewStylePlain];
        _tpTableView.bounces = NO;
        _tpTableView.dataSource = self;
        _tpTableView.rowHeight = 18;
        _tpTableView.separatorStyle = UITableViewCellSeparatorStyleNone;
    }
    return _tpTableView;
}

- (UIButton *)getTPBtnWithFrame:(CGRect)rect title:(NSString *)title sel:(SEL)sel {
    UIButton *btn = [[UIButton alloc] initWithFrame:rect];
    btn.layer.cornerRadius = 2;
    btn.layer.borderWidth = 1;
    btn.layer.borderColor = [UIColor blackColor].CGColor;
    [btn setTitle:title forState:UIControlStateNormal];
    [btn setBackgroundImage:[self imageWithColor:[UIColor colorWithRed:127/255.0 green:179/255.0 blue:219/255.0 alpha:1]] forState:UIControlStateSelected];
    btn.titleLabel.font = [UIFont systemFontOfSize:10];
    [btn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [btn addTarget:self action:sel forControlEvents:UIControlEventTouchUpInside];
    return btn;
}

- (UIImage *)imageWithColor:(UIColor *)color{
    CGRect rect = CGRectMake(0.0f, 0.0f, 1.0f, 1.0f);
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);
    UIImage *theImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return theImage;
}

- (UIButton *)recordBtn {
    if (!_recordBtn) {
        _recordBtn = [self getTPBtnWithFrame:CGRectMake(5, 65, 60, 30) title:@"调用时间" sel:@selector(clickRecordBtn)];
        _recordBtn.hidden = YES;
    }
    return _recordBtn;
}

- (UILabel *)titleLable {
    if (!_titleLable) {
        _titleLable = [[UILabel alloc]initWithFrame:CGRectMake(0, 20, self.view.bounds.size.width, 44)];
        _titleLable.textAlignment = NSTextAlignmentCenter;
    }
    return _titleLable;
}

- (UIButton *)costTimeSortBtn {
    if (!_costTimeSortBtn) {
        _costTimeSortBtn = [self getTPBtnWithFrame:CGRectMake(70, 65, 60, 30) title:@"最耗时" sel:@selector(clickCostTimeSortBtn)];
        _costTimeSortBtn.hidden = YES;
    }
    return _costTimeSortBtn;
}

- (UIButton *)callCountSortBtn {
    if (!_callCountSortBtn) {
        _callCountSortBtn = [self getTPBtnWithFrame:CGRectMake(135, 65, 60, 30) title:@"调用次数" sel:@selector(clickCallCountSortBtn)];
        _callCountSortBtn.hidden = YES;
    }
    return _callCountSortBtn;
}

- (UIButton *)popVCBtn {
    if (!_popVCBtn) {
        _popVCBtn = [self getTPBtnWithFrame:CGRectMake([UIScreen mainScreen].bounds.size.width-50, 65, 40, 30) title:@"关闭" sel:@selector(clickPopVCBtn:)];
    }
    return _popVCBtn;
}

- (UIButton *)clearBtn {
    if (!_clearBtn) {
        _clearBtn = [self getTPBtnWithFrame:CGRectMake([UIScreen mainScreen].bounds.size.width-100, 65, 40, 30) title:@"清空" sel:@selector(clickClearBtn:)];
    }
    return _clearBtn;
}

- (UILabel *)tableHeaderViewLabel {
    if (!_tableHeaderViewLabel) {
        _tableHeaderViewLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, BBASMScrollWidth, 30)];
        _tableHeaderViewLabel.font = [UIFont systemFontOfSize:15];
        _tableHeaderViewLabel.backgroundColor = [UIColor colorWithRed:219.0/255 green:219.0/255 blue:219.0/255 alpha:1];
    }
    return _tableHeaderViewLabel;
}

- (void)setTpTableType:(BBASMTableType)tpTableType {
    if (_tpTableType!=tpTableType) {
        if (tpTableType==tableTypeCallCount) {
            self.tableHeaderViewLabel.text = @"深度       耗时      次数            方法名";
        }
        else
        {
            self.tableHeaderViewLabel.text = @"深度       耗时                  方法名";
        }
        _tpTableType = tpTableType;
    }
}

@end
