#include <jni.h>
#include "Hello.h"
#include <stdio.h>

JNIEXPORT void JNICALL
Java_Hello_hello(JNIEnv *env, jobject obj)
{
    printf("c hello \n");
    return;
}
