#ifndef __ASSETS__
#define __ASSETS__

#include "common.h"

typedef void *asset;
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *assetBuffer(const char *, const void **, iter *);
asset openAsset  (const char *);
int   assetRead  (asset, void *, iter);
void  assetSeek  (asset, int);
iter  assetLength(asset);
void  assetClose (asset);
  
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSETS__