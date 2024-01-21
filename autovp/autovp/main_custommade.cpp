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

	char value[PROPERTY_VALUE_MAX];
	property_get("ro.boot.simulation", value, "0");
	if((strcmp(value, "1") == 0)){
		config_vm_net_work();
	}

	int ret = 0;

	if(access("/data/adb/config/build.simulation.prop", F_OK) != 0){
		ret = system("mkdir -p /data/adb/config");
		ALOGD("mkdir -p /data/adb/config errno = %s", strerror(errno));

		ret = system("mkdir -p /data/adb/prebuilt/binary");
		ALOGD("mkdir -p /data/adb/prebuilt/binary errno = %s", strerror(errno));

		ret = system("cp /system/bin/custommadebin /data/adb/prebuilt/binary/custommadebin");
		ALOGD("cp /system/bin/custommadebin /data/adb/prebuilt/binary/custommadebin errno = %s", strerror(errno));

		ret = system("cp /system/build.simulation.prop /data/adb/config/build.simulation.prop");
		ALOGD("cp /system/build.simulation.prop /data/adb/config/build.simulation.prop errno = %s", strerror(errno));

		ret = system("cp /system/build.simulation.cell1.prop /data/adb/config/build.simulation.cell1.prop");
		ALOGD("cp /system/build.simulation.cell1.prop /data/adb/config/build.simulation.cell1.prop errno = %s", strerror(errno));

		ret = system("cp /system/build.simulation.cell2.prop /data/adb/config/build.simulation.cell2.prop");
		ALOGD("cp /system/build.simulation.cell2.prop /data/adb/config/build.simulation.cell2.prop errno = %s", strerror(errno));

		ret = system("cp /system/build.simulation.cell3.prop /data/adb/config/build.simulation.cell3.prop");
		ALOGD("cp /system/build.simulation.cell3.prop /data/adb/config/build.simulation.cell3.prop errno = %s", strerror(errno));
	}

	if(access("/data/adb/prebuilt/binary/custommadebin", F_OK) == 0){
		ret = system("rm /data/custommadebin");
		ALOGD("rm /data/custommadebin errno = %s", strerror(errno));

		ret = system("mv /data/adb/prebuilt/binary/custommadebin /data/custommadebin");
		ALOGD("mv /data/adb/prebuilt/binary/custommadebin /data/custommadebin errno = %s", strerror(errno));
	}

	ret = system("chmod 700 /data/custommadebin");
	ALOGD("chmod 700 /data/custommadebin errno = %s", strerror(errno));

	ret = system("/data/custommadebin -mode cell");
	ALOGD("/data/custommadebin -mode cell errno = %s", strerror(errno));

    ALOGD("CustomMade service exit...");
    return 0;
}
