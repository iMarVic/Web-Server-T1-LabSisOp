#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>
#include<sys/utsname.h>


long get_uptime(void) {
        struct sysinfo s_info;
        int error = sysinfo(&s_info);
        if(error != 0) {
                printf("code error = %d\n", error);
        }
        return s_info.uptime;
}

#define BUFLEN 1024 //Max length of buffer
#define PORT 8000 //The port on which to listen for incoming data
char http_ok[] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\nServer: Test\r\n\r\n";
char http_error[] = "HTTP/1.0 400 Bad Request\r\nContent-type: text/html\r\nServer: Test\r\n\r\n";
char page[10000];

void die(char *s)
{
        perror(s);
        exit(1);
}

void pag(){
	
        char header[] = "\n<!DOCTYPE html>\n<html>\n <head>\n <title> T1-LabSisOp </title>\n <meta charset=\"utf-8\">\n </head>\n<body>\n";
	strcat(page,header);
        char p[] = "<p>";
        char p_end[] = "</p>\n";
        FILE *fp;
        char a;
	char cToStr[2];

        /*----TIME----*/
        strcat(page,p);
                char txt[200]="";
                fp = popen("date", "r");
                  while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
			cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                pclose(fp);
        strcat(page,p_end);
        
        /*----UPTIME----*/
        strcat(page,p);
                //txt[200]="";
                fp = popen("uptime", "r");
                  while((a=fgetc(fp))!= EOF){
			//printf("%c",a); //PRINT
			cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
        strcat(page,p_end);
       
        /*----Sys version----*/
        strcat(page,p);
                fp = popen("uname -a", "r");
                while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
                        cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
        strcat(page,p_end);

        /*----CPU----*/
        strcat(page,p);
                fp = popen("lscpu", "r");
                while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
                        cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
        strcat(page,p_end);

	/*----%CPU----*/
        strcat(page,p);
                fp = popen("ps -A -o pcpu | tail -n+2 | paste -sd+ | bc", "r");
                while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
                        cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
	strcat(page,"porcentage in use of the CPU");
        strcat(page,p_end);
	

	/*----Processos----*/
        strcat(page,p);
                fp = popen("ps", "r");
                while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
                        cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
        strcat(page,p_end);

	/*----MEMO----*/
        strcat(page,p);
                fp = popen("free", "r");
                while((a=fgetc(fp))!= EOF){
                        //printf("%c",a); //PRINT
                        cToStr[0] = a;
			cToStr[1] = '\0';
			strcat(page,cToStr);
                  }
                  pclose(fp);
        strcat(page,p_end);


        char end[] = "\n</body>\n</html>\n\n";
        strcat(page,end);

        //printf("%s",page);
}


int main(void)
{	
	
        struct sockaddr_in si_me, si_other;
        int s, i, slen = sizeof(si_other), recv_len, conn, child = 0;
        char buf[BUFLEN];
        pid_t pid;

        /* create a TCP socket */
        if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
                die("socket");

        /* zero out the structure */
        memset((char *) &si_me, 0, sizeof(si_me));

        si_me.sin_family = AF_INET;
        si_me.sin_port = htons(PORT);
        si_me.sin_addr.s_addr = htonl(INADDR_ANY);

        /* bind socket to port */
        if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)
                die("bind");
        /* allow 10 requests to queue up */
        if (listen(s, 10) == -1)
                die("listen");

        /* keep listening for data */
        while (1) {
		
		pag();
                memset(buf, 0, sizeof(buf));
                printf("Waiting a connection...");
                fflush(stdout);
		
		

                conn = accept(s, (struct sockaddr *) &si_other, &slen);
                if (conn < 0)
                        die("accept");

                if ((pid = fork()) < 0)
                        die("fork");

                else if (pid == 0) {
                        close(s);

                        printf("Client connected: %s:%d\n", inet_ntoa(si_other.sin_addr),ntohs(si_other.sin_port));
                       
                        /* try to receive some data, this is a blocking call */
                        recv_len = read(conn, buf, BUFLEN);
                       
                        if (recv_len < 0)
                                die("read");

                        /* print details of the client/peer and the data received */
                        printf("Data: %s\n" , buf);

                        if (strstr(buf, "GET")) {
                                /* now reply the client with the same data */
                                if (write(conn, http_ok, strlen(http_ok)) < 0)
                                        die("write");
                                if (write(conn, page, strlen(page)) < 0)
                                        die("write");
                        } else {
                                if (write(conn, http_error, strlen(http_error)) < 0)
                                die("write");
                        }
		
                exit(0);		
                }
		strcpy(page, "");
		//page[]="";

                /* close the connection */
                close(conn);

                child++;

                while (child) {
                        pid = waitpid((pid_t) -1, NULL, WNOHANG);
                        if (pid < 0)
                                die("?");
                        else if (pid == 0) break;
                        else child--;
                }
        }
        close(s);

        return 0;
}
