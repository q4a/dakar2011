/****************************************************************
*                                                               *
*    Name: own_server_udp.cpp                                   *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the server part of the game. It      *
*       will just listen for packet and the send back the       *
*       datas of the other car on that stage.                   *
*                                                               *
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "multiplayer.h"

#define STAGE_NUM 30

struct SCarDataWrapper* carDataWrappers[STAGE_NUM][CAR_NUM] = {0};
int sockfd = 0;


int checkCar(int stage, int car, struct sockaddr_in* their_addr)
{
    time_t sec = getSec();
    if (stage<0 || STAGE_NUM <= stage) return 0;
    if (car<0 || CAR_NUM <= car) return 0;
    
    if (carDataWrappers[stage][car]==0) return 0;

    if (carDataWrappers[stage][car]->their_addr.sin_addr.s_addr != their_addr->sin_addr.s_addr
        || carDataWrappers[stage][car]->their_addr.sin_port != their_addr->sin_port
        ) return 0;
    

    if (sec - carDataWrappers[stage][car]->time > TIMEOUT)
    {
        printf("delete car %d from stage %d TIMEDOUT\n", car, stage);
        delete carDataWrappers[stage][car];
	carDataWrappers[stage][car] = 0;
	return 0;
    }
    
    //carDataWrappers[stage][car]->time = sec;
    
    return 1;
}

int addNewCar(int stage, struct sockaddr_in* their_addr)
{
    int i;

    if (stage<0 || STAGE_NUM <= stage) return -1;
    
    for (i=0;i<CAR_NUM;i++)
    {
	if (carDataWrappers[stage][i]==0 || !checkCar(stage , i, &carDataWrappers[stage][i]->their_addr))
	{
	    carDataWrappers[stage][i] = new SCarDataWrapper;
	    memset(carDataWrappers[stage][i], 0, sizeof(SCarDataWrapper));
	    carDataWrappers[stage][i]->carData.num = i;
	    carDataWrappers[stage][i]->carData.type = -1;
	    carDataWrappers[stage][i]->time = getSec();
	    carDataWrappers[stage][i]->their_addr.sin_addr.s_addr = their_addr->sin_addr.s_addr;
	    carDataWrappers[stage][i]->their_addr.sin_port = their_addr->sin_port;
	    return i;
	}
    }
    
    return -1;
}

void deleteCar(int stage, int car, struct sockaddr_in* their_addr)
{
    if (!checkCar(stage, car, their_addr)) return;
    printf("delete car %d from stage %d\n", car, stage);
    delete carDataWrappers[stage][car];
    carDataWrappers[stage][car] = 0;
}

void sendCarDatas(int stage, int car, struct sockaddr_in* their_addr)
{
    char data[DATA_LENGTH];
    int i;
    
    if (stage < 0 || STAGE_NUM <= stage) return;
    
    for (i=0;i<CAR_NUM;i++)
    {
	if (car!=i && carDataWrappers[stage][i] && checkCar(stage, i, &carDataWrappers[stage][i]->their_addr) && 
	    carDataWrappers[stage][i]->carData.type!=-1)
	{
	    //printf("send updated datas from %d to %d\n", car, i);
	    data[0] = 'u';
	    memcpy(data + 1, &carDataWrappers[stage][i]->carData, sizeof(SCarData));
	    sendto(sockfd, data, DATA_LENGTH, MSG_DONTWAIT, (struct sockaddr*)their_addr, sizeof(struct sockaddr_in));
	}
    }
}

void sendInitialMessage(int car, struct sockaddr_in* their_addr)
{
    char data[INITIAL_DATA_LENGTH];
    int i;
    
    data[0] = 'n';
    memcpy(data + 1, &car, sizeof(int));
    sendto(sockfd, data, INITIAL_DATA_LENGTH, MSG_DONTWAIT, (struct sockaddr*)their_addr, sizeof(struct sockaddr_in));
}

void sendRemoveMessage(int car, struct sockaddr_in* their_addr)
{
    char data[3];
    int i;
    
    data[0] = 'd';
    data[1] = 'd';
    data[2] = 'd';
    sendto(sockfd, data, 3, MSG_DONTWAIT, (struct sockaddr*)their_addr, sizeof(struct sockaddr_in));
}


int main(void)
{
    struct sockaddr_in my_addr; // a saját címinformációm
    struct sockaddr_in their_addr; // a saját címinformációm
    socklen_t sin_size;
    int readamount;
    int stage, car;
    SCarData* carData;
    char data[DATA_LENGTH+sizeof(int)]; // max: char + sizeof(SCarData) + sizeof(int) = command + stage index + data
    // struct sigaction sa;
    int yes=1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
          perror("socket");
          exit(1); 
    }

    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
          perror("setsockopt");
          exit(1); 
    }

    my_addr.sin_family = AF_INET; // host byte order
    my_addr.sin_port = htons(MYPORT); // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatikusan kitölti az IP-mel
    memset(&(my_addr.sin_zero), '\0', 8); // nulla a struktúra többi részében

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
          perror("bind");
          exit(1); 
    }

    while(1) { // main accept() loop
	readamount=0;
        sin_size = sizeof(struct sockaddr_in);
	
	readamount = recvfrom(sockfd, data, DATA_LENGTH+sizeof(int),0, (struct sockaddr *)&their_addr, &sin_size);
        if (readamount <= 0)
	{
            printf("error recvfrom\n");
            continue; 
	}
	
	if (data[0]=='n')
	{
        unsigned int ip = their_addr.sin_addr.s_addr;
	    printf("new car request from %hhu.%hhu.%hhu.%hhu\n", 
         (unsigned char)(ip&0xFF), (unsigned char)((ip>>8)&0xFF), (unsigned char)((ip>>16)&0xFF), (unsigned char)(ip>>24));
	    if (readamount == 1 + sizeof(int))
	    {
		stage = *((int*)(data+1));
		car = addNewCar(stage, &their_addr);
		printf("create new car %d for stage %d\n", car, stage);
        if (car>=0)
		    sendInitialMessage(car, &their_addr);
	    }
	    else
		printf("wrong size n got %d exp %d\n", readamount, 1+sizeof(int));
	}
	else
	if (data[0]=='u')
	{
	    //printf("update car request\n");
	    if (readamount == (1 + sizeof(int) + sizeof(SCarData)))
	    {
		stage = *((int*)(data+1));
		carData = (SCarData*)(data+1+sizeof(int));
		car = carData->num;
		//printf("update car %d for stage %d\n", car, stage);
		if (checkCar(stage, car, &their_addr))
		{
		    memcpy(&carDataWrappers[stage][car]->carData, carData, sizeof(SCarData));
		    carDataWrappers[stage][car]->time = getSec();
		    sendCarDatas(stage, car, &their_addr);
		}
		else
		{
		    printf("wrong update car %d for stage %d\n", car, stage);
		    sendRemoveMessage(car, &their_addr);
		}
	    }
	    else
		printf("wrong size n got %d exp %d\n", readamount, 1+sizeof(int)+sizeof(SCarData));
	}
	else	
	if (data[0]=='d')
	{
	    printf("delete car request\n");
	    if (readamount == 1 + 2*sizeof(int))
	    {
		stage = *((int*)(data+1));
		car = *((int*)(data+1+sizeof(int)));
		printf("delete car %d from stage %d\n", car, stage);
		deleteCar(stage, car, &their_addr);
	    }
	    else
		printf("wrong size n got %d exp %d\n", readamount, 1+2*sizeof(int));
	}	
    }
    return 0; 
}

