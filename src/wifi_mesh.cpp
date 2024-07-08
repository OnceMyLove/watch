#include "wifi_mesh.h"



#define   MESH_PREFIX     "wyc"
#define   MESH_PASSWORD   "86629715w"
#define   MESH_PORT       5555

Scheduler     userScheduler; // 控制你的个人任务
painlessMesh  mesh;
// 原型
void receivedCallback( uint32_t from, String &msg );

//自定义的函数和变量
int msg_value=0;
char* msg_topic=NULL;
char* length_of_string(char* p,int start,int end);
void get_device_and_value( char* p_msg,char** msg_topic,int* msg_value);
void create_or_updata_device(char* name,int value);
typedef struct device{
  char* name;
  int value;
  struct device* next;
}device;  //存放设备的名字和值

// device device1={"wyc",1,NULL};
//初始化一个头指针和头节点
// device head={"wyc",0,NULL};
device* p_head=NULL;



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
  // printf("logServer: Received from %u msg=%s\n", from, msg.c_str());
  get_device_and_value(p_msg,&msg_topic,&msg_value);
  // p_msg=length_of_string(p_msg,27,1);
  // msg_value=atoi(p_msg);
  // printf("logServer: Received from %u msg=%s  value=%d\n", from,p_msg,msg_value);
  // lv_label_set_text_fmt(ui_Label1,"device1:%d",msg_value);
  // printf("logServer: Received from %s  value=%d\n", msg_topic,msg_value);
  create_or_updata_device(msg_topic,msg_value);
  show();
  
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

void get_device_and_value( char* p_msg,char** msg_topic,int* msg_value)
{
  cJSON* json=cJSON_Parse(p_msg);
  cJSON* topic;
  cJSON* value;
  if(json==NULL)
  {
    const char* error_ptr=cJSON_GetErrorPtr();
    if(error_ptr!=NULL)
    {
      fprintf(stderr,"Error before:%s\n",error_ptr);
      
    }  
  }
  // printf("%s\n",p_msg);
  // 获取'topic'字段
  topic=cJSON_GetObjectItemCaseSensitive(json,"topic");
  if(cJSON_IsString(topic)&&(topic->valuestring!=NULL))
  {
    // printf("Topic:%s\n",topic->valuestring);
    *msg_topic=topic->valuestring;
  }
  // 获取'value'字段
  value =cJSON_GetObjectItemCaseSensitive(json, "value");
  if (cJSON_IsNumber(value)) {
    // printf("Value: %d\n", value->valueint);
    *msg_value=value->valueint;
  }
}

void create_or_updata_device(char* name,int value)
{
  int is_seted=0;//判断是否已经接收过此设备，0为没有，1为有
  device* p=p_head;
  while(p!=NULL)
  {
    if(strcmp(p->name,name)==0)
    {
      is_seted=1;
      // printf("break\n");
      break;
    }
    // printf("p->next\n");
    p=p->next;
  }
  if(is_seted)
  {
    p->value=value;
    printf("%d,%s\n",p->value,p->name);
  }
  else
  {
    device* new_device=(device*)malloc(sizeof(device));
    new_device->name=name;
    new_device->value=value;
    // new_device->next=NULL;
    
    new_device->next=p_head;
    p_head=new_device;
    printf("加入新设备%d,%s\n",new_device->value,new_device->name);
  }
  
}


void show(void)
{
  device* visit=p_head;
  //最大值排序链表  大->小
  device* p_max=NULL;
  device* max_now_location=p_max;//用于最大值排序链表的定位指针
  while(visit!=NULL)//visit指针用于对p_head链表的搜寻
  {

    printf("logServer: Received from %s  value=%d\n",visit->name,visit->value);
    device* p_this=(device*)malloc(sizeof(device));
    p_this->name=visit->name;
    p_this->value=visit->value;
    p_this->next=NULL;
    if(max_now_location==NULL)
    {
      max_now_location=p_this;
      p_max=max_now_location;//一定要有这一步，不然p_max仍为空，因为上一步给max_now_location重新赋值了
    }
    else 
    {
      while(max_now_location->next!=NULL)
      {
        if((visit->value<max_now_location->value)&(visit->value>max_now_location->next->value))//当前值比max_now_location小，且大于max_now_location下一个值
        {
          
          break;
        }
        max_now_location=max_now_location->next;
        printf("while(max_now_location->next!=NULL)");//程序运行时会进入此死循环max_now_location->next可能永远不空注意其赋值。
      }
      if(max_now_location->next==NULL)
      {
        max_now_location->next=p_this;
        
      }
      else
      {
        p_this->next=max_now_location->next;
        max_now_location->next=p_this;
      }
    }
    if(visit->next==NULL)printf("NULL");
    
    visit=visit->next;
  }
  printf("exit");
  visit=p_max;
  
  if(visit!=NULL)
  {
    lv_label_set_text_fmt(ui_Label1,"%s,value:{%d}\n",visit->name,visit->value);
  }
  if(visit->next!=NULL)
  {
    visit=visit->next;
    lv_label_set_text_fmt(ui_Label2,"%s,value:{%d}\n",visit->name,visit->value);
  }
  if(visit->next!=NULL)
  {
    visit=visit->next;
    lv_label_set_text_fmt(ui_Label3,"%s,value:{%d}\n",visit->name,visit->value);
  }
}