//
//     Generated by class-dump 3.5 (64 bit) (Debug version compiled Sep 17 2017 16:24:48).
//
//     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2015 by Steve Nygard.
//

#import "MMTableViewCell.h"

@class NSMutableArray, UIImageView, UILabel, UIView, WAHeaderStackDisplayView;

@interface WAHomePreviewIconTableViewCell : MMTableViewCell
{
    NSMutableArray *_headerArray;
    UIView *_lineView;
    _Bool _isBottom;
    UILabel *_titleLabel;
    UIView *_normalContentView;
    WAHeaderStackDisplayView *_headerStackView;
    UIImageView *_arrowImageView;
}

@property(retain, nonatomic) UIImageView *arrowImageView; // @synthesize arrowImageView=_arrowImageView;
@property(retain, nonatomic) WAHeaderStackDisplayView *headerStackView; // @synthesize headerStackView=_headerStackView;
@property(retain, nonatomic) UIView *normalContentView; // @synthesize normalContentView=_normalContentView;
@property(retain, nonatomic) UILabel *titleLabel; // @synthesize titleLabel=_titleLabel;
- (void).cxx_destruct;
- (void)markUnBottom;
- (void)markBottom;
- (void)fillPreviewIcons:(id)arg1;
- (void)layoutSubviews;
- (void)initSubViews;
- (id)initWithStyle:(long long)arg1 reuseIdentifier:(id)arg2;

@end
