#include <Arduino.h>
#include "painlessMesh.h"
#include "wifi_mesh.h"


#define   MESH_PREFIX     "wyc"
#define   MESH_PASSWORD   "86629715w"
#define   MESH_PORT       5555

Scheduler     userScheduler; // 控制你的个人任务
painlessMesh  mesh;
// 原型
void receivedCallback( uint32_t from, String &msg );

//自定义的函数和变量
int msg_value;
char* length_of_string(char* p,int start,int end);

// 每10秒发送我的ID以通知其他人。
Task logServerTask(10000, TASK_FOREVER, []() {
#if ARDUINOJSON_VERSION_MAJOR==6
        DynamicJsonDocument jsonBuffer(1024);
        JsonObject msg = jsonBuffer.to<JsonObject>();
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& msg = jsonBuffer.createObject();
#endif
    msg["topic"] = "logServer";
    msg["nodeId"] = mesh.getNodeId();

    String str;
#if ARDUINOJSON_VERSION_MAJOR==6
    serializeJson(msg, str);
#else
    msg.printTo(str);
#endif
    mesh.sendBroadcast(str);

    // 记录到串行
#if ARDUINOJSON_VERSION_MAJOR==6
    serializeJson(msg, Serial);
#else
    msg.printTo(Serial);
#endif
    printf("\n");
});

void mesh_setup(void) {
  
    
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG ); // all types on
  //mesh.setDebugMsgTypes( ERROR | CONNECTION | SYNC | S_TIME );  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  // 在init()之前设置，这样你可以看到启动消息。

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);

  mesh.onNewConnection([](size_t nodeId) {
    printf("New Connection %u\n", nodeId);
  });

  mesh.onDroppedConnection([](size_t nodeId) {
    printf("Dropped Connection %u\n", nodeId);
  });

  // 将任务添加到你的调度程序中。
  userScheduler.addTask(logServerTask);
  logServerTask.enable();
}

void mesh_loop(void) {
  // 它将运行用户调度程序。
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {
  msg.c_str(); //c_str()获取字符串首字母的地址
  char *p_msg=new char[strlen(msg.c_str()+1)];
  strcpy(p_msg,msg.c_str());
  printf("logServer: Received from %u msg=%s\n", from, msg.c_str()+26);
  // printf("logServer: Received from %u msg=%s\n", from, msg.c_str()+23);
  p_msg=length_of_string(p_msg,27,1);
  msg_value=atoi(p_msg);
  printf("logServer: Received from %u msg=%s  value=%d\n", from,p_msg,msg_value);
}

char* length_of_string(char* p,int start,int end)  //截取字符串部分，start为正数第几位，end为倒数第几位
{
  char* q=p;
  while(*q!='\0')
  {
    q++;
  }
  p=p+start-1;
  q=q-end;
  *q='\0';
  return p;
}