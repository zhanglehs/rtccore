#ifndef AVENGINE_SOURCE_IOS_VIDEO_RENDER_API_H_
#define AVENGINE_SOURCE_IOS_VIDEO_RENDER_API_H_

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__ ((__visibility__("default"))) void *lfrtcCreateIosVideoRender(int idx, CGRect frame, float scale, UIView *view, void **rendHnd);
__attribute__ ((__visibility__("default"))) void lfrtcDestroyIosVideoRender(void *rendHnd);
__attribute__((__visibility__("default"))) void CREATE_VIDEO_RENDER_PAUSE(void *hnd, bool flag);
	
#ifdef __cplusplus
}
#endif

#endif
