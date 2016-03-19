#ifndef __QWK2WWIV_H__
#define __QWK2WWIV_H__

#include <stdint.h>

struct sub {
	char *type;
	int num;
};

struct config {
	char *archivecmd;
	char *unarchivecmd;
	char *qwkpath;
	char *reppath;
	char *temppath;
	char *wwivin;
	char *wwivout;
	char *remoteid;
	char *yourtag;
	unsigned short yourwwivnode;
	unsigned short theirwwivnode;
	int subcount;
	struct sub **subs;
};

typedef struct {
    uint16_t tosys,
                   touser,
                   fromsys,
                   fromuser;
    uint16_t main_type,
                   minor_type;
    uint16_t list_len;
    uint32_t  daten;
    uint32_t  length;
    uint16_t method;
}__attribute__((packed)) net_header_rec;

struct QwkHdr {
        unsigned char   Msgstat;                /* Message status       */
        unsigned char   Msgnum[7];              /* Message number       */
        unsigned char   Msgdate[8];             /* Message date MM-DD-YY*/
        unsigned char   Msgtime[5];             /* Message time HH:MM   */
        unsigned char   MsgTo[25];              /* Message To:          */
        unsigned char   MsgFrom[25];            /* Message From:        */
        unsigned char   MsgSubj[25];            /* Message Subject:     */
        unsigned char   Msgpass[12];            /* Message password     */
        unsigned char   Msgrply[8];             /* Message reply to     */
        unsigned char   Msgrecs[6];             /* Length in records    */
        unsigned char   Msglive;                /* Message active status*/
        unsigned char   Msgarealo;              /* Lo-byte message area */
        unsigned char   Msgareahi;              /* Hi-byte message area */
	   	unsigned char   Msgfiller[3];           /* Filler bytes         */
} __attribute__((packed));

extern void translate(char *in, unsigned int size);

extern int q2w(struct config *conf);
extern int w2r(struct config *conf);

#endif
