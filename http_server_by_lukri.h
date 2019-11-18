//
// Created by root on 11/17/19.
//
#include <pthread.h>
#include <sys/epoll.h>

#define MAXEVENTS 128
#define EVENT_TIMEOUT   0x01
#define EVENT_READ      0X02
#define  EVENT_WRITE    0x04
#define  EVENT_SIGNAL   0x08

//这里定义函数指针，这里
//event_read_callback pf就是等价于 int (*pf) (void *data)声明
//其中pf是一个函数指针变量
typedef int (*event_read_callback)(void *data);
typedef int (*event_write_callback)(void *data);

//保存描述字，以及描述字上注册的事件，和相应注册事件的回调函数
struct channel{
    int fd;
    int events; //表示event类型

    event_read_callback eventReadCallback;
    event_write_callback  eventWriteCallback;
    void *data; //callback data,可能是event_loop,也可能是tcp_server或者tcp_connection
};

//fd and address mapping list, key is fd, value is channel-address
struct channel_map{
    void **entries;

    int nentries;
};
void map_init(struct channel_map *map);

//to realize select,poll, epoll IO function
struct event_dispatcher{
    const char *name;

    //init function
    void *(*init)(struct event_loop * eventLoop);

    //通知dispatcher新增加一个channel事件
    int (*add)(struct event_loop *eventLoop, struct channel * channel);

    //通知dispatcher删除一个channel事件
    int (*del)(struct event_loop *eventLoop, struct channel * channel);

    //通知dispatcher更新一个channel事件
    int (*update)(struct event_loop *eventLoop, struct channel * channel);

    //实现事件分发，然后调用event_loop的event_activate方法执行回调函数
    int (*dispatch)(struct event_loop *eventLoop, struct channel * channel);

    //清除数据
    void (*clear)(struct event_loop *eventLoop);
};

//声明epoll_dispatcher_data结构体
typedef struct {
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event *events;
} epoll_dispatcher_data;

static void *epoll_init(struct event_loop *);

static int epoll_add(struct event_loop *, struct channel *channel1);

static int epoll_del(struct event_loop *, struct channel *channel1);

static int epoll_update(struct event_loop *, struct channel *channel1);

static void epoll_dispatch(struct event_loop *, struct timeval *);

static void epoll_clear(struct event_loop *);

struct channel_element{
    int type; //1: add  2: delete
    struct channel *channel;
    struct  channel_element * next;
};

//定义2个IO复用结构体,分别表示epoll和poll类型的
const struct event_dispatcher epoll_dispatcher;
const struct event_dispatcher poll_dispatcher;

struct event_loop{
    int quit;
    //实现IO复用的结构体
    const struct event_dispatcher *eventDispatcher;

    //对应event_dispatcher的数据
    void *event_dispatcher_data;
    //fd和channel的映射表，key是fd,value是channel的地址值
    struct channel_map *channelMap;

    int is_handle_pending;
    //用于实现channel链表
    struct channel_element *pending_head;
    struct channel_element *pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int socketPair[2];
    char *thread_name;
};

struct event_loop *event_loop_init();

struct event_loop *event_loop_init_with_name(char * thread_name);

