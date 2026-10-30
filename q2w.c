/* strptime() is declared by glibc only with _GNU_SOURCE (or _XOPEN_SOURCE) */
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

#include "qwk2wwiv.h"

/* QWK ends each line of a message with this byte, in place of CR LF */
#define QWK_EOL '\xe3'

/* Synchronet's Ctrl-A attribute codes are this byte and the one after it */
#define CTRL_A '\x01'

/*
 * Remove every Synchronet Ctrl-A code, ^A and the character after it, from
 * the NUL terminated string in.  WWIV reads a 0x01 byte in message text as
 * a soft line ending, so none may remain.  size is unused, kept for w2r.c.
 */
void translate(char *in, unsigned int size) {
	char *src = in, *dst = in;

	(void)size;
	while (*src) {
		if (*src == CTRL_A) {
			src++;
			if (*src)
				src++;
			continue;
		}
		*dst++ = *src++;
	}
	*dst = '\0';
}

/*
 * Copy the len bytes of fixed width QWK header field src to dst, a buffer of
 * dstsize bytes, as a NUL terminated string without trailing spaces.
 * Header fields are padded with spaces and are not NUL terminated.
 */
static char *field(char *dst, size_t dstsize, const unsigned char *src, size_t len) {
	size_t n = len < dstsize - 1 ? len : dstsize - 1;

	memcpy(dst, src, n);
	dst[n] = '\0';
	while (n > 0 && (dst[n - 1] == ' ' || dst[n - 1] == '\0'))
		dst[--n] = '\0';
	return dst;
}

/*
 * Convert the text of a QWK message, text of len bytes, to WWIV text at
 * out, returning its length.  Each line ends with CR LF.  A line ending of
 * QWK (0xE3) is one, and so is any a packet may also contain: CR LF, LF CR,
 * LF, CR, or 0xE3 followed by LF or CR LF.  Ctrl-A codes are removed, and so
 * is the padding of the last record.  out must hold 2 * len bytes.
 */
static size_t qwk_to_wwiv_text(const char *text, size_t len, char *out) {
	size_t i, o = 0;

	/* the last record is padded with spaces (or NULs) */
	while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\0'))
		len--;

	for (i = 0; i < len; i++) {
		char c = text[i];

		if (c == CTRL_A) {
			i++; /* and the character after it */
			continue;
		}
		if (c == QWK_EOL || c == '\r' || c == '\n') {
			if (c == QWK_EOL && i + 1 < len && text[i + 1] == '\r')
				i++;
			if (i + 1 < len && (
			    ((c == QWK_EOL || text[i] == '\r') && text[i + 1] == '\n') ||
			    (c == '\n' && text[i + 1] == '\r')))
				i++;
			out[o++] = '\r';
			out[o++] = '\n';
			continue;
		}
		if (c == '\0')
			continue;
		out[o++] = c;
	}
	return o;
}

int q2w(struct config *conf) {
	int i, pos;
	char cmdline[1024];
	FILE *fptr;
	FILE *optr;

	struct QwkHdr qwkhdr;
	net_header_rec wwivhdr;

	char *msgbuf;
	char *wwivbuf;
	size_t msglen, wwivbuf_size;
	char temp1[50];

	char from[26], subj[26], date[9], timeofday[6], recs[7];

	int records;
	unsigned int confnum;
	char *subtype;
	DIR *dirptr;
	struct dirent *next_file;

	char dt[32];
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
	snprintf(cmdline, sizeof(cmdline), "%s/MESSAGES.DAT", conf->temppath);

	fptr = fopen(cmdline, "rb");
	if (!fptr) {
		printf("Unable to open MESSAGES.DAT\n");
		return 0;
	}
	optr = fopen(conf->wwivin, "wb");
	if (!optr) {
		printf("Unable to open %s\n", conf->wwivin);
		fclose(fptr);
		return 0;
	}
	fread(&qwkhdr, sizeof(struct QwkHdr), 1, fptr); // skip the first rec
	while (fread(&qwkhdr, sizeof(struct QwkHdr), 1, fptr)) {
		field(from, sizeof(from), qwkhdr.MsgFrom, sizeof(qwkhdr.MsgFrom));
		field(subj, sizeof(subj), qwkhdr.MsgSubj, sizeof(qwkhdr.MsgSubj));
		field(date, sizeof(date), qwkhdr.Msgdate, sizeof(qwkhdr.Msgdate));
		field(timeofday, sizeof(timeofday), qwkhdr.Msgtime, sizeof(qwkhdr.Msgtime));
		field(recs, sizeof(recs), qwkhdr.Msgrecs, sizeof(qwkhdr.Msgrecs));

		/* Msgrecs counts the header record, too */
		records = atoi(recs);
		if (records < 1) {
			printf("WARNING: Message of %d records, stopping\n", records);
			break;
		}

		confnum=(qwkhdr.Msgareahi & 0x00FF)<<8;
		confnum+=qwkhdr.Msgarealo;

		/* read the text, of all its records, as it is */
		msglen = (size_t)(records - 1) * 128;
		msgbuf = malloc(msglen + 1);
		if (!msgbuf) {
			printf("Out of memory\n");
			break;
		}
		msglen = fread(msgbuf, 1, msglen, fptr);
		msgbuf[msglen] = '\0';

		subtype = NULL;

		for (i=0;i<conf->subcount;i++) {
			if (confnum == conf->subs[i]->num) {
				subtype = conf->subs[i]->type;
				break;
			}
		}

		memset(&wwivhdr, 0, sizeof(net_header_rec));

		if (subtype != NULL) {
			/* each byte of text may become 2, of CR LF */
			wwivbuf = (char *)malloc(msglen * 2 + 4096);
			if (!wwivbuf) {
				printf("Out of memory\n");
				free(msgbuf);
				break;
			}
			wwivhdr.main_type = 0x001a;
			wwivhdr.tosys = conf->yourwwivnode;

			strcpy(wwivbuf, subtype);
			wwivbuf_size = strlen(subtype)+1;

			/* Append the Subject to the text, with its NUL terminator */
			strcpy(wwivbuf+wwivbuf_size, subj);
			wwivbuf_size += strlen(subj)+1;

			/* Append the From to the text */
			strcpy(wwivbuf+wwivbuf_size, from);
			strcat(wwivbuf+wwivbuf_size, "\r\n");
			wwivbuf_size += strlen(from) + 2;

			/* Append the Date to the text, of "MM-DD-YY" and "HH:MM" */
			snprintf(dt, sizeof(dt), "%s %s", date, timeofday);

			memset(&tm, 0, sizeof(struct tm));
			if (strptime(dt, "%m-%d-%y %H:%M", &tm) == NULL) {
				printf("WARNING: Bad date \"%s\" of a message from %s, using now\n", dt, from);
				timestamp = time(NULL);
			} else {
				tm.tm_isdst = -1; /* of local time, either */
				timestamp = mktime(&tm);
			}

			snprintf(temp1, sizeof(temp1), "%ld\r\n", (long)timestamp);
			strcpy(wwivbuf+wwivbuf_size, temp1);
			wwivbuf_size += strlen(temp1);

			/* Finally append the actual message text */
			wwivbuf_size += qwk_to_wwiv_text(msgbuf, msglen, wwivbuf + wwivbuf_size);

			wwivhdr.list_len = 0;
			wwivhdr.length = wwivbuf_size;
			wwivhdr.daten = timestamp;

			wwivhdr.fromsys = conf->theirwwivnode;

			wwivhdr.fromuser = 0;

			fwrite(&wwivhdr, sizeof(net_header_rec), 1, optr);
			fwrite(wwivbuf, 1, wwivbuf_size, optr);

			free(wwivbuf);
		} else {
			printf("WARNING: Unknown QWK Conference %d\n", confnum);
		}

		free(msgbuf);
	}

	fclose(optr);
	fclose(fptr);

	// clean up temp directory
	dirptr = opendir(conf->temppath);
	if (!dirptr) {
		return 1;
	}

	while ((next_file = readdir(dirptr)) != NULL) {
		if (strcmp(next_file->d_name, ".") == 0 || strcmp(next_file->d_name, "..") == 0) {
			continue;
		}

		snprintf(cmdline, sizeof(cmdline), "%s/%s", conf->temppath, next_file->d_name);
		remove(cmdline);
	}
	closedir(dirptr);

	return 1;
}
