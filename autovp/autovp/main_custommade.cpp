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

//using namespace android;

static char* get_local_ip()
{                                                                                           
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;
	sock = socket(AF_INET, SOCK_DGRAM, 0); 
	if (sock == -1) {
		ALOGD("socket");
		return NULL;
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		ALOGD("ioctl");
		return NULL;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	return inet_ntoa(sin.sin_addr);
}

static void config_vm_net_work(void)
{
	errno = 0;
	int ret = system("ifconfig eth0 down");
	ALOGD("ifconfig eth0 down errno = %s", strerror(errno));

	ret = system("ip link set eth0 name wlan0");
	ALOGD("ip link set eth0 name wlan0 errno = %s", strerror(errno));

	char value[PROPERTY_VALUE_MAX];
	property_get("persist.ro.custommade.deviceinfo.mac", value, "ac:c1:ee:57:64:9b");
	char cmd[256] = {0};
	snprintf(cmd, sizeof(cmd), "ifconfig wlan0 hw ether %s", value);
	ret = system(cmd);
	ALOGD("%s errno = %s", cmd, strerror(errno));

	ret = system("ifconfig wlan0 up");
	ALOGD("ifconfig wlan0 up errno = %s", strerror(errno));
};

int main(int /*argc*/, char** /*argv*/)
{
    ALOGI("GuiExt service start...");

    ALOGD("CustomMade service exit...");
    return 0;
}
