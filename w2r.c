#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

#include "qwk2wwiv.h"

void strip_heart_colors(char *text)
{
  int pos=0;
  int len=strlen(text);

  while(pos<len && text[pos] != 26)
  {
    if(text[pos]==3)
    {
      memmove(text+pos, text+pos+2, len-pos);
      --len;
      --len;
    }
    else
      ++pos;
  }
}



int w2r(struct config *conf) {
	FILE *fptr;
	
	net_header_rec wwivrec;
	char *msgbuf;
	char *from;
	char *subject;
	char *date;
	char *msg;
	char *subtype;
	
	int i;
	
	struct QwkHdr qh;
	char *cvtmsg;
	unsigned int msgrecs;
	int j;
	char *finalbuf;
	char buff[26];
	int subnum;
	struct tm *time_now;
	char date2[10];
	char cmdline[1024];
	FILE *optr;
	
	int pos;
	DIR *dirptr;
	struct dirent *next_file;
		
	fptr = fopen(conf->wwivout, "rb");
	
	if (!fptr) {
		return 0;
	}
	
	memset(&qh, ' ', sizeof(struct QwkHdr));
	memcpy(&qh, conf->remoteid, strlen(conf->remoteid));
	
	sprintf(cmdline, "%s/%s.msg", conf->temppath, conf->remoteid);
		
	optr = fopen(cmdline, "wb");
	fwrite(&qh, sizeof(struct QwkHdr), 1, optr);
		
	while (fread(&wwivrec, sizeof(net_header_rec), 1, fptr)) {
		msgbuf = (char *)malloc(wwivrec.length + 1);
		
		memset(msgbuf, 0, wwivrec.length + 1);
		
		fread(msgbuf, wwivrec.length, 1, fptr);
		
		subtype = msgbuf;
		
		subject = subtype + strlen(msgbuf) + 1;
		
		from = subject + strlen(subject) + 1;
		

		
		for (i=0;i<strlen(from);i++) {
			if (from[i] == '\r') {
				from[i] = '\0';
				from[i+1] = '\0';
				date = &from[i+2];
				break;
			}
		}
		
		for (i=0;i<strlen(date);i++) {
			if (date[i] == '\r') {
				date[i] = '\0';
				date[i+1] = '\0';
				msg = &date[i+2];
				break;
			}
		}
		
		while (msg[i] != '\n') {
			i++;
		}
		msg = &msg[i+1];
		
		subnum = -1;
		
		for (i=0;i<conf->subcount;i++) {
			if (strcmp(conf->subs[i]->type, subtype) == 0) {
				subnum = conf->subs[i]->num;
				break;
			}
		}
		
		if (subnum == -1) {
			printf("WARNING: Unknown Sub Type: %s\n", subtype);
			free(msgbuf);
			continue;
		}
		
		memset(qh.MsgFrom,' ',25);
		memset(qh.MsgSubj,' ',25);
		memset(qh.MsgTo,' ',25);
		
		strip_heart_colors(subject);
		
		for (i=0;i<strlen(from);i++) {
			if (from[i] == '#') {
				memcpy(qh.MsgFrom, from, (i-1 > 25 ? 25 : i-1));
				break;
			}
		}

		/* memcpy(qh.MsgFrom, from, (strlen(from) > 25 ? 25 : strlen(from))); */
		memcpy(qh.MsgSubj, subject, (strlen(subject) > 25 ? 25 : strlen(subject)));
		memcpy(qh.MsgTo, "ALL", 3);

		memset(qh.Msgpass,' ',12);

		qh.Msglive = 0xE1;
		qh.Msgstat= ' ';
		memset(qh.Msgfiller,' ',3);qh.Msgfiller[2]='*';
		memset(qh.Msgrply,' ',8);
		
		/* Define the conference area */
		qh.Msgarealo = (unsigned char) (subnum & 0xFF);
		qh.Msgareahi = (unsigned char) ((subnum & 0xFF00) >> 8);
		
		/* Fill up msgnum fields */
		memset(buff,' ',25);
		sprintf(buff,"%d",subnum);
		memset(qh.Msgnum,' ',7);
		memcpy(qh.Msgnum,buff,strlen(buff));		
		
		
		qh.Msgtime[0] = date[11];
		qh.Msgtime[1] = date[12];
		qh.Msgtime[2] = date[13];
		qh.Msgtime[3] = date[14];
		qh.Msgtime[4] = date[15];
		
		if (strncmp(&date[4], "Jan", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '1';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Feb", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '2';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];

		} else
		if (strncmp(&date[4], "Mar", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '3';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];

		} else
		if (strncmp(&date[4], "Apr", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '4';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "May", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '5';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Jun", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '6';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Jul", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '7';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Aug", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '8';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Sep", 3) == 0) {
			qh.Msgdate[0] = '0';
			qh.Msgdate[1] = '9';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Oct", 3) == 0) {
			qh.Msgdate[0] = '1';
			qh.Msgdate[1] = '0';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Nov", 3) == 0) {
			qh.Msgdate[0] = '1';
			qh.Msgdate[1] = '1';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		} else
		if (strncmp(&date[4], "Dec", 3) == 0) {
			qh.Msgdate[0] = '1';
			qh.Msgdate[1] = '2';
			qh.Msgdate[2] = '-';
			qh.Msgdate[3] = date[8];
			qh.Msgdate[4] = date[9];
			qh.Msgdate[5] = '-';
			qh.Msgdate[6] = date[22];
			qh.Msgdate[7] = date[23];
		}
		
		
		cvtmsg = (char *)malloc(strlen(msg) + strlen(conf->yourtag) + 25);	
		
		j = 0;
		for (i=0;i<strlen(msg);i++) {
			if (msg[i] == '\x4' && msg[i+1] == '0') { // skips ^D0 lines
				while (msg[i] != '\n') {
					i++;
				}
				i++; // skip the \n too
			} 
			if (msg[i] == '\r') {
				cvtmsg[j] = '\xe3';
				i++;
			} else if (msg[i] == '\x1') { // strip soft line ending markers
				continue;
			} else {
				cvtmsg[j] = msg[i];
			}
			j++;
		}
		
		cvtmsg[strlen(cvtmsg) -1] = '\0'; // get rid of ^Z
		
		strip_heart_colors(cvtmsg);
		
		sprintf(cvtmsg, "%s\xe3---\xe3 * QWK2WWIV * %s\xe3", cvtmsg, conf->yourtag);
		
		msgrecs = (strlen(cvtmsg) / 128) + 1;
		
		if (strlen(cvtmsg) % 128) {
			msgrecs++;
		}
		
		finalbuf = (char *)malloc((msgrecs - 1) * 128);
		
		memset(finalbuf, 0, (msgrecs - 1) * 128);
		
		memcpy(finalbuf, cvtmsg, strlen(cvtmsg));
		
		free(cvtmsg);
		
		sprintf(buff, "%d", msgrecs);
		for(i=strlen(buff);i<26;i++)
			buff[i]=' ';
		
		memset(qh.Msgrecs,' ',6);
		memcpy(qh.Msgrecs,buff,6);
		
		translate(finalbuf, strlen(finalbuf));
		for (i=strlen(finalbuf);i<(msgrecs - 1) * 128;i++) {
			finalbuf[i] = ' ';
		}
		
		fwrite(&qh, sizeof(struct QwkHdr), 1, optr);
		fwrite(finalbuf, (msgrecs - 1) * 128, 1, optr);
		
		free(finalbuf);
		
		free(msgbuf);
	}
	fclose(optr);
	fclose(fptr);
	
	memset(cmdline, 0, 1024);
	pos = 0;
	
	for (i=0;i<strlen(conf->archivecmd);i++) {
		if (conf->archivecmd[i] == '%') {
			i++;
			if (conf->archivecmd[i] == '1') {
				strcat(cmdline, conf->reppath);
				pos += strlen(conf->reppath);
			} else if (conf->archivecmd[i] == '2') {
				strcat(cmdline, conf->temppath);
				strcat(cmdline, "/");
				strcat(cmdline, conf->remoteid);
				strcat(cmdline, ".msg");
				pos += strlen(conf->temppath) + strlen(conf->remoteid) + 5;
			} else {
				cmdline[pos] = '%';
				cmdline[pos+1] = conf->archivecmd[i];
				pos += 2;
			}
		} else {
			cmdline[pos] = conf->archivecmd[i];
			pos ++;
		}
	}
	
	system(cmdline);	
	
// clean up temp directory
	dirptr = opendir(conf->temppath);
	
	while ((next_file = readdir(dirptr)) != NULL) {
		if (strcmp(next_file->d_name, ".") == 0 || strcmp(next_file->d_name, "..") == 0) {
			continue;
		}
		
		sprintf(cmdline, "%s/%s", conf->temppath, next_file->d_name);
		remove(cmdline);
	}
	closedir(dirptr);	
	
	return 1;
}
