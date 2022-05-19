//
//  ttt.m
//  test007
//
//  Created by yuhao on 2022/4/13.
//

#import "RTCTtt.h"
#import "sdk/msl-core/src/public/test.h"

@implementation RTCTtt

@synthesize connected = _connected;
-(void)t {
     NSLog(@"123!");

     test(4, 5);
}

- (instancetype)init {
    self = [super init];
    if (self) {
        self.connected = YES;
    }
    NSLog(@"RTCTtt init");
    return self;
}

- (void)dealloc {
    NSLog(@"RTCTtt dealloc");
}
@end
