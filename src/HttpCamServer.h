#include <WebServer.h>

#define ENABLE_WEBSERVER

#ifdef ENABLE_WEBSERVER
extern WebServer httpCamServer;
#endif

extern bool wifiConnectionEstablished;

void setupHttpServer();
void loopHttpServer();