#define LOG_TAG "CustomMadeService"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <cutils/log.h>
#include <cutils/properties.h>

int main(int /*argc*/, char** /*argv*/)
{
    ALOGI("GuiExt service start...");

    ALOGD("CustomMade service exit...");
    return 0;
}
