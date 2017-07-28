//added by gxh
//#import "../interface/file_ios.h"
//#import <AVFoundation/AVFoundation.h>
#import "fileios.h"
//__attribute__ ((__visibility__("default"))) char* makePreferencesFilename(char *name);

@interface OCClass (private) //: NSObject


//- (char*)makePreferencesFilename:(char *)name;

@end

@implementation OCClass{
    
}
/*
- (id)initWithCoder:(NSCoder*)coder {
    // init super class
    self = [super initWithCoder:coder];
    if (self) {
        
    }
    return self;
}
*/
//这个函数得到存取的路径。
- (char*)makePreferencesFilename:(char *)name
{
#if 1
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *SName = [[NSString alloc] initWithCString:(char*)name encoding:NSASCIIStringEncoding];
    NSString *writablePath = [documentsDirectory stringByAppendingPathComponent:SName];
    int len = [writablePath length];
    char *filename = (char*)malloc(sizeof(char) * (len + 1));
    [writablePath getCString:filename maxLength:len + 1 encoding:[NSString defaultCStringEncoding]];
#else
    char *filename = "null";
#endif
    return filename;
}

@end

char *MAKE_FILE_NAME(char *name)
{
#if 1
    OCClass *someObj=[OCClass new];
    return [someObj makePreferencesFilename:name];
#else
    return NULL;
#endif
}

#if 0
- (void)saveOptions
{
    char *path = [self makePreferencesFilename];
    FILE *fp = fopen(path, "wt");
    char sTemp [100];
    //snprintf(sTemp, [loginName length], "%s", [loginName UTF8String]);
    fputs([loginName UTF8String],  fp);
    fputs("\n", fp);
    if(bSavePassword) {
        //snprintf(sTemp, [loginPassword length], "%s", [loginPassword UTF8String]);
        fputs([loginPassword UTF8String],  fp);
    }
    else {
        fputs("",  fp);
    }
    
    fputs("\n", fp);
    
    //存其他的信息
    fclose(fp);
    free(path);
}
- (void)loadOptions
{
    char *path = [self makePreferencesFilename];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *spath = [[NSString alloc] initWithFormat:@"%s", path];
    BOOL fileExists = [fileManager fileExistsAtPath:spath];
    if (!fileExists)
    {
        loginName = nil;
        loginPassword = nil;
        //初始化其他的信息。
        free(path);
        [spath release];
        return;
    }
    char sTemp[100];
    FILE *fp = fopen(path, "rt");
    //fscanf(fp, "%s", sTemp);
    fgets(sTemp, 100,fp);
    int ilen = strlen(sTemp);
    sTemp[ilen - 1] = 0;
    loginName = [[NSString alloc] initWithFormat:@"%s",sTemp];
    if([loginName isEqualToString:@"(null)"]) {
        [loginName release];
        loginName = nil;
    }
    if(sTemp[0] == 0){
        [loginName release];
        loginName = nil;
    }
    fgets(sTemp, 100,fp);
    ilen = strlen(sTemp);
    sTemp[ilen - 1] = 0;
    loginPassword = [[NSString alloc] initWithFormat:@"%s",sTemp];
    if([loginPassword isEqualToString:@"(null)"]) {
        [loginPassword release];
        loginPassword = nil;
    }
    if(sTemp[0] == 0){
        [loginPassword release];
        loginPassword = nil;
    }
    //读其他的信息
    fclose(fp);
    [spath release];
    free(path);
}
#endif