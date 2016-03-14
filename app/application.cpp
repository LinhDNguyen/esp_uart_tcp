#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "200C" // Put you SSID and Password here
	#define WIFI_PWD "maitrang200c"
#endif

Timer memoryTimer;
int savedHeap = 0;
TcpClient* activeClient = NULL;

void tcpServerClientConnected (TcpClient* client)
{
	debugf("Application onClientCallback : %s\r\n",client->getRemoteIp().toString().c_str());
	activeClient = client;
}

bool tcpServerClientReceive (TcpClient& client, char *data, int size)
{
	debugf("Application DataCallback : %s, %d bytes \r\n",client.getRemoteIp().toString().c_str(),size );
	debugf("Data : %s", data);
	// client.sendString("sendString data\r\n", false);
	// client.writeString("writeString data\r\n",0 );
	if (strcmp(data,"+++") == 0)
	{
		// Process special data here
		debugf("Closing client");
		client.close();
		return true;
	};
	for (int i = 0; i < size; ++i) {
		Serial.write(data[i]);
	}
	return true;
}

void tcpServerClientComplete(TcpClient& client, bool succesfull)
{
	debugf("Application CompleteCallback : %s \r\n",client.getRemoteIp().toString().c_str() );
	activeClient = NULL;
}

TcpServer tcpServer(tcpServerClientConnected, tcpServerClientReceive, tcpServerClientComplete);

void startServers()
{
	tcpServer.listen(23);
	tcpServer.setTimeOut(USHRT_MAX);

	Serial.println("\r\n=== TCP SERVER Port 23 STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	startServers();
}
void connectFail();
void smartConfigCallback(sc_status status, void *pdata) {

	switch (status) {
		case SC_STATUS_WAIT:
			debugf("SC_STATUS_WAIT\n");
			break;
		case SC_STATUS_FIND_CHANNEL:
			debugf("SC_STATUS_FIND_CHANNEL\n");
			break;
		case SC_STATUS_GETTING_SSID_PSWD:
			debugf("SC_STATUS_GETTING_SSID_PSWD\n");
			break;
		case SC_STATUS_LINK:
			{
				debugf("SC_STATUS_LINK\n");
				station_config *sta_conf = (station_config *)pdata;
				char *ssid = (char*)sta_conf->ssid;
				char *password = (char*)sta_conf->password;
				WifiStation.config(ssid, password);
				WifiStation.waitConnection(connectOk, 30, connectFail);
			}
			break;
		case SC_STATUS_LINK_OVER:
			debugf("SC_STATUS_LINK_OVER\n");
			WifiStation.smartConfigStop();
			break;
	}
}

void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	// WifiStation.waitConnection(connectOk, 10, connectFail);
	WifiStation.smartConfigStart(SCT_EspTouch, smartConfigCallback);
}


void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	// Just simply forward to TCP
	char str[availableCharsCount];
	int ret = 0;
	ret = stream.readBytes(str, availableCharsCount);
	if (!activeClient->send(str, availableCharsCount)) {
		debugf("Write %d bytes to TCP client failed", ret);
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Enable debug output to serial
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD, true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk, 30, connectFail);
	Serial.setCallback(serialCallBack);
}
