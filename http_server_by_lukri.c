//
// Created by root on 11/17/19.
//

#include "http_server_by_lukri.h"
#include <stdlib.h>

//event_dispatcher的具体实现，这里是使用epoll来实现IO复用的
const struct event_dispatcher epoll_dispatcher = {
        "epoll",
        epoll_init,
        epoll_add,
        epoll_del,
        epoll_update,
        epoll_dispatch,
        epoll_clear,
};

//实现对event_loop结构体的初始化
struct event_loop *event_loop_init(){
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(char * thread_name){
    //分配event_loop内存空间
    struct event_loop *eventLoop = malloc(sizeof(struct event_loop));
    //初始化线程互斥锁和条件变量
    pthread_mutex_init(&eventLoop->mutex, NULL);
    pthread_cond_init(&eventLoop->cond, NULL);

    //获取线程的名字
    if (thread_name == NULL){
        eventLoop->thread_name = thread_name;
    }else{
        eventLoop->thread_name = "main thread";
    }

    eventLoop->quit = 0;
    //分配channelMap的内存空间
    eventLoop->channelMap = malloc(sizeof(struct channel_map));
    //初始化channelMap的值
    map_init(eventLoop->channelMap);

    printf("set epoll as dispatcher, %s", eventLoop->thread_name);
    //使用epoll类型的IO事件复用
    eventLoop->eventDispatcher = &epoll_dispatcher;

    //对应event_dispatcher的数据
    eventLoop->event_dispatcher_data = eventLoop->eventDispatcher->init(eventLoop);
}

//在event_loop_init中被调用，用来初始化event_dispatcher的数据
void *epoll_init(struct event_loop *eventLoop){
    //给epoll_dispatcher_data结构体分配内存空间
    epoll_dispatcher_data *epollDispatcherData = malloc(sizeof(epoll_dispatcher_data));

    //初始化epoll_dispatcher_data里的数据
    epollDispatcherData->event_count = 0;
    epollDispatcherData->nfds = 0;
    epollDispatcherData->realloc_copy = 0;
    epollDispatcherData->efd = 0;

    //创建一个epoll实例
    epollDispatcherData->efd = epoll_create1(0);
    if (epollDispatcherData->efd == -1){
        printf("epoll create failed!");
    }

    //给event事件数组分配内存空间
    epollDispatcherData->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    return epollDispatcherData;
}

int epoll_add(struct event_loop *eventLoop, struct channel *channel1){
    //获取当前event_loop里的epoll_dispatcher_data。
    epoll_dispatcher_data *pollDispatcherData = (epoll_dispatcher_data *)eventLoop->event_dispatcher_data;

    //获取channel里的fd和event事件
    int fd = channel1->fd;
    int events = 0;
    if(channel1->events & EVENT_READ){
        events = events | EPOLLIN;
    }
    if(channel1->events & EVENT_WRITE){
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    //将fd和event添加到epoll_event中
    event.data.fd = fd;
    event.events = events | EPOLLET;

    //将相应的fd和事件添加到当前的epoll实例中
    if (epoll_ctl(pollDispatcherData->efd, EPOLL_CTL_ADD,fd, &event))
    {
        printf("epoll_ctl add fd failed!");
    }

    return 0;
}

int epoll_del(struct event_loop *eventLoop, struct channel *channel){
    epoll_dispatcher_data *pollDispatcherData = (epoll_dispatcher_data *) eventLoop->event_dispatcher_data;

    int fd = channel1->fd;
    int events = 0;
    if (channel1->events & EVENT_READ){
        events = events | EPOLLIN;
    }

    if (channel1->events & EVENT_WRITE){
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if(epoll_ctl(pollDispatcherData->efd, EPOLL_CTL_DEL, fd, &event) == -1){
        printf("epoll ctl delete fd failed");
    }
    return 0;
}

void map_init(struct channle_map *map){
    map->nentries = 0;
    map->entries = NULL;
}

int main(int c, char **v){
    //main thread
    /**
     * 1.初始化event_loop结构里数据
     * 2.初始化eventDispatcher为epoll，并选用了epoll为IO复用。
     * 3.初始化epoll_dispatcher_data的值，会创建epoll对象，同时给evnet事件进行初始化
     * **/
    struct event_loop *eventLoop = event_loop_init();

}