#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* generate a fatal error, debug trap or a message. */
void myError(int num, const char *msg, ...);
//void myDebug(int num, const char *msg, ...);
void myMessage(int num, const char *msg, ...);

#ifdef __cplusplus
}
#endif

#endif // __ERROR_H__
