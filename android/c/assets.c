#include "common.h"
#include "assets.h"

#include <android/asset_manager.h>

static AAssetManager *mngr = NULL;

asset assetBuffer(const char *filename, const void **buf, iter *len) {
  AAsset *reading = AAssetManager_open(mngr, filename, AASSET_MODE_BUFFER);
  *len = CAST(iter)AAsset_getLength(reading);
  *buf = AAsset_getBuffer(reading);
  return CAST(asset)reading;
}
asset openAsset(const char *filename) {
  return CAST(asset)AAssetManager_open(mngr, filename, AASSET_MODE_STREAMING);
}
int assetRead(asset a, void *buf, iter count) {
  return AAsset_read(CAST(AAsset *)a, buf, count);
}
void assetSeek(asset a, int l) {
  AAsset_seek(CAST(AAsset *)a, l, SEEK_CUR);
}
iter assetLength(asset a) {
  return CAST(iter)AAsset_getRemainingLength64(CAST(AAsset *)a);
}
void assetClose(asset a) {
  AAsset_close(CAST(AAsset *)a);
}
// external call
void androidAsset_init(AAssetManager *m) {
  mngr = m;
}
void androidAsset_term(void) {
  mngr = NULL;
}