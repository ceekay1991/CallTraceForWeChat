//
//  BBASMRecordCell.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@class BBASMRecordModel;
@class BBASMRecordCell;

@protocol BBASMRecordCellDelegate <NSObject>

- (void)recordCell:(BBASMRecordCell *)cell clickExpandWithSection:(NSInteger)section;

@end

@interface BBASMRecordCell : UITableViewCell

@property (nonatomic, weak)id<BBASMRecordCellDelegate> delegate;

- (void)bindRecordModel:(BBASMRecordModel *)model
      isHiddenExpandBtn:(BOOL)isHidden
               isExpand:(BOOL)isExpand
                section:(NSInteger)section
        isCallCountType:(BOOL)isCallCountType;

@end

NS_ASSUME_NONNULL_END
