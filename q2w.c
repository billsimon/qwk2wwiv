#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

#include "qwk2wwiv.h"

char *strrepl(char *str, size_t bufsiz, char *oldstr, char *newstr)
{
  int oldlen, newlen;
  char *p = str, *q;

  oldlen = strlen(oldstr);
  newlen = strlen(newstr);
  if (NULL == (p = strstr(p, oldstr))) return str;

  if ((strlen(str) + newlen - oldlen + 1) > bufsiz) return NULL;

  memmove(q = p+newlen, p+oldlen, strlen(p+oldlen)+1);
  memcpy(p, newstr, newlen);

  return q;
}

int Qwk2Cr(char* buf)
{
    do {
		if(*buf == '\xe3')
		    *buf = '\r';
		} while(*buf++);
    
    return(0);
}


void translate(char *in, unsigned int size) {
	while(strstr(in,"b"))	strrepl(in,size,"b","");
	while(strstr(in,"c"))	strrepl(in,size,"c","");
	while(strstr(in,"g"))	strrepl(in,size,"g","");
	while(strstr(in,"h"))	strrepl(in,size,"h","");
	while(strstr(in,"k"))	strrepl(in,size,"k","");
	while(strstr(in,"n"))	strrepl(in,size,"n","");
	while(strstr(in,"p"))	strrepl(in,size,"p","");
	while(strstr(in,"r"))	strrepl(in,size,"r","");
	while(strstr(in,"w"))	strrepl(in,size,"w","");
	while(strstr(in,"y"))	strrepl(in,size,"y","");

	while(strstr(in,"B"))	strrepl(in,size,"B","");
	while(strstr(in,"C"))	strrepl(in,size,"C","");
	while(strstr(in,"G"))	strrepl(in,size,"G","");
	while(strstr(in,"H"))	strrepl(in,size,"H","");
	while(strstr(in,"K"))	strrepl(in,size,"K","");
	while(strstr(in,"N"))	strrepl(in,size,"N","");
	while(strstr(in,"P"))	strrepl(in,size,"P","");
	while(strstr(in,"R"))	strrepl(in,size,"R","");
	while(strstr(in,"W"))	strrepl(in,size,"W","");
	while(strstr(in,"Y"))	strrepl(in,size,"Y","");
}

char* StripSpaces(char* str, int size) {
    static char TempStr[128];
    int x;
    
    memcpy(TempStr, str, size);
    TempStr[size] = '\0';
    
    if((x = (size - 1)) > 0)
    {
	while(x > 0 && TempStr[x] == ' ')
	    TempStr[x--] = '\0';
    }
	strcpy(str,TempStr);
	return(TempStr);
}

char *daynames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char *monnames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
int dayofweek(int y, int m, int d) {
        return (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;       

}

int q2w(struct config *conf) {
	int i, pos;
	char cmdline[1024];
	FILE *fptr;
	FILE *optr;
	
	struct QwkHdr qwkhdr;
	net_header_rec wwivhdr;
	
	char tbuf[128];
	char *msgbuf;
	char *wwivbuf;
	int wwivbuf_size;
	char temp1[50];
	
	int records;
	unsigned int confnum;
	char *subtype;
	int cur_pos;
	int qday,qmonth,qyear,qhour,qmin; 
	DIR *dirptr;
	struct dirent *next_file;

	char dt[14];
	struct tm tm;
	time_t timestamp;
	
	pos = 0;
	memset(cmdline, 0, 1024);
	
	for (i=0;i<strlen(conf->unarchivecmd);i++) {
		if (conf->unarchivecmd[i] == '%') {
			i++;
			if (conf->unarchivecmd[i] == '1') {
				strcat(cmdline, conf->qwkpath);
				pos += strlen(conf->qwkpath);
			} else if (conf->unarchivecmd[i] == '2') {
				strcat(cmdline, conf->temppath);
				pos += strlen(conf->temppath);
			} else {
				cmdline[pos] = '%';
				cmdline[pos+1] = conf->unarchivecmd[i];
				pos += 2;
			}
		} else {
			cmdline[pos] = conf->unarchivecmd[i];
			pos ++;
		}
	}
	
	system(cmdline);
	
	memset(cmdline, 0, 1024);
	sprintf(cmdline, "%s/MESSAGES.DAT", conf->temppath);
	
	fptr = fopen(cmdline, "rb");
	if (!fptr) {
		printf("Unable to open MESSAGES.DAT\n");
		return 0;
	}
	optr = fopen(conf->wwivin, "wb");
	if (!optr) {
		printf("Unable to open %s\n", conf->wwivin);
		return 0;
	}
	fread(&qwkhdr, sizeof(struct QwkHdr), 1, fptr); // skip the first rec
	while (fread(&qwkhdr, sizeof(struct QwkHdr), 1, fptr)) {
		memset(tbuf, 0, 128);
		records = atoi(qwkhdr.Msgrecs);
		msgbuf = malloc(records * 128 + 1);
		memset(msgbuf, 0, records * 128 + 1);
		
		confnum=0L;
		confnum=(qwkhdr.Msgareahi & 0x00FF)<<8;
		confnum+=qwkhdr.Msgarealo;
		
		StripSpaces(qwkhdr.MsgFrom,25);
		StripSpaces(qwkhdr.MsgSubj,25);
		StripSpaces(qwkhdr.MsgTo,25);
		StripSpaces(qwkhdr.Msgpass,12);
		StripSpaces(qwkhdr.Msgnum,7);
		StripSpaces(qwkhdr.Msgrply,8);
		StripSpaces(qwkhdr.Msgrecs,6);

		
		for(i=1; i < records; i++) {
			fread(tbuf, 128, 1, fptr);
			StripSpaces(tbuf, 128);
			Qwk2Cr(tbuf);
			strcat(msgbuf, tbuf); 
		}
		
		translate(msgbuf, strlen(msgbuf));
		
		subtype = NULL;
		
		for (i=0;i<conf->subcount;i++) {
			if (confnum == conf->subs[i]->num) {
				subtype = conf->subs[i]->type;
				break;
			}
		}
		
		memset(&wwivhdr, 0, sizeof(net_header_rec));
		
		if (subtype != NULL) {
			wwivbuf = (char *)malloc(records * 128 + 4096);
			wwivhdr.main_type = 0x001a;
			wwivhdr.tosys = conf->yourwwivnode;
			
			strcpy(wwivbuf, subtype);
			wwivbuf_size = strlen(subtype)+1;
			
			/* Append the Subject to the text */
			strncpy(wwivbuf+wwivbuf_size, qwkhdr.MsgSubj, 25);
			wwivbuf_size += strlen(qwkhdr.MsgSubj)+1; /* +1 so we can save NULL terminator */

			/* Append the From to the text */
			strncpy(wwivbuf+wwivbuf_size, qwkhdr.MsgFrom, 25);

			strcat(wwivbuf+wwivbuf_size, "\r\n");
			wwivbuf_size += strlen(qwkhdr.MsgFrom) + 2;

			/* Append the Date to the text */

			/* qyear = 2000 + ((qwkhdr.Msgdate[6] - '0') * 10) + (qwkhdr.Msgdate[7] - '0');
			qmonth = (qwkhdr.Msgdate[0] - '0') * 10 + (qwkhdr.Msgdate[1] - '0');
			qday = (qwkhdr.Msgdate[3] - '0') * 10 + (qwkhdr.Msgdate[4] - '0');

			qhour = (qwkhdr.Msgtime[0] - '0') * 10 + (qwkhdr.Msgtime[1] - '0');
			qmin = (qwkhdr.Msgtime[3] - '0') * 10 + (qwkhdr.Msgtime[4] - '0'); */

			strcpy(dt, qwkhdr.Msgdate);
			strcat(dt, " ");
			strcat(dt, qwkhdr.Msgtime);

			memset(&tm, 0, sizeof(struct tm));
			strptime(dt, "%m-%d-%y %H:%M", &tm);

			timestamp = mktime(&tm);

			/* sprintf(temp1, "%s %s %d %02d:%02d:00 %04d\r\n", daynames[dayofweek(qyear, qmonth,qday)], monnames[qmonth-1], qday, qhour, qmin, qyear); */
			sprintf(temp1, "%d\r\n", timestamp);
			strcpy(wwivbuf+wwivbuf_size, temp1);
			wwivbuf_size += strlen(temp1);
			
			cur_pos = 0;
			
			/* Finally append the acutall message text */
			while(cur_pos < strlen(msgbuf)) {
				if(msgbuf[cur_pos] == '\r') {
					wwivbuf[wwivbuf_size] = '\r';
					wwivbuf[wwivbuf_size+1] = '\n';

					wwivbuf_size+=2;
					++cur_pos;
				} else {
					wwivbuf[wwivbuf_size]=msgbuf[cur_pos];

					++wwivbuf_size;
					++cur_pos;
				}
			}
			

			wwivbuf[wwivbuf_size]=0xE0;     /* Control Z terminate text */

			wwivhdr.list_len = 0;
			wwivhdr.length = wwivbuf_size;
			wwivhdr.daten = timestamp;

			wwivhdr.fromsys = conf->theirwwivnode;

			wwivhdr.fromuser = 0;
			
			fwrite(&wwivhdr, sizeof(net_header_rec), 1, optr);
			fwrite(wwivbuf, 1, wwivbuf_size, optr);
			
		} else {
			printf("WARNING: Unknown QWK Conference %d\n", confnum);
		}
		
		
		
		free(msgbuf);
	}
	
	fclose(optr);
	fclose(fptr);
	
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
