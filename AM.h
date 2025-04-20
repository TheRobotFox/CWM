#pragma once

#include "BasicWindow.h"

typedef BasicWindow App;
typedef BasicWindow AM;

typedef void (*AM_callback)(App app, char key, void *data);

AM AM_create();
void AM_free(AM _am);
App AM_app_create(const char *name, AM_callback callback);
void AM_app_free(App app);
void AM_register(AM _am, App app);
void AM_app_data_set(App _app, void *data);
void* AM_app_data_get(App _app);
void AM_unregister(AM _am, App _app);
void AM_key_event(AM _am, char key);
void AM_prepare(AM _am);
