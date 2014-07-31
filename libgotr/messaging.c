#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gcrypt.h>
#include <arpa/inet.h>

#include "user.h"
#include "util.h"
#include "crypto.h"
#include "gotr.h"
#include "b64.h"
#include "gka.h"

struct msg_pair_channel_init {
	struct gotr_ecdhe_public_key   dh_pub;
};

struct msg_pair_channel_est {
	struct gotr_eddsa_signature  sig;
	struct gotr_ecdhe_public_key   dh_pub;
	struct gotr_eddsa_public_key sender_pub;
};

struct msg_flake_z {
	uint32_t                     op;
	struct gotr_eddsa_signature  sig;
	unsigned char                encrypted[];
};

struct four_mpis {
	unsigned char a1[512];
	unsigned char a2[512];
	unsigned char a3[512];
	unsigned char a4[512];
};

unsigned char *gotr_pack_pair_channel_init(const struct gotr_roomdata *room, struct gotr_user *user, size_t *len)
{
	struct msg_pair_channel_init *msg;

	*len = 0;
	if(!room || !user || !(msg = malloc(sizeof(struct msg_pair_channel_init))))
		return NULL;

	memset(msg, 0, sizeof(struct msg_pair_channel_init));

	gotr_ecdhe_key_create(&user->dhe_privkey);
	gotr_ecdhe_key_get_public(&user->dhe_privkey, &msg->dh_pub);

	user->next_msgtype = GOTR_SEND_PAIR_CHAN_ESTABLISH;
	*len = sizeof(struct msg_pair_channel_init);
	return (unsigned char *)msg;
}

unsigned char *gotr_pack_pair_channel_est(const struct gotr_roomdata *room, struct gotr_user *user, size_t *len)
{
	struct msg_pair_channel_est *msg;

	if(!room || !user || !(msg = malloc(sizeof(struct msg_pair_channel_est))))
		return NULL;

	memset(msg, 0, sizeof(struct msg_pair_channel_est));

	user->next_msgtype = GOTR_SEND_FLAKE_z;
	return (unsigned char *)msg;
}

unsigned char *gotr_pack_flake_z(const struct gotr_roomdata *room, struct gotr_user *user, size_t *len)
{
	return NULL;
}

unsigned char *gotr_pack_flake_R(const struct gotr_roomdata *room, struct gotr_user *user, size_t *len)
{
	return NULL;
}

unsigned char *gotr_pack_flake_validation(const struct gotr_roomdata *room, struct gotr_user *user, size_t *len)
{
	return NULL;
}

unsigned char *gotr_pack_msg(const struct gotr_roomdata *room, char *msg, size_t *len)
{
	return NULL;
}

int gotr_parse_pair_channel_init(struct gotr_roomdata *room, struct gotr_user *user, char *packed_msg, size_t len)
{
	struct msg_pair_channel_init *msg = (struct msg_pair_channel_init*)packed_msg;

	if(!room || !user || !packed_msg || len != sizeof(struct msg_pair_channel_init))
		return 0;

	memcpy(&user->dhe_pubkey, &msg->dh_pub, sizeof(struct gotr_ecdhe_public_key));
/// @todo generate keys for enc and hmac:
// gotr_ecc_ecdh(&user->dhe_privkey, &user->dhe_pubkey, &BLA);

	user->expected_msgtype = GOTR_EXPECT_PAIR_CHAN_ESTABLISH;
	return GOTR_OK;
}

int gotr_parse_pair_channel_est(struct gotr_roomdata *room, struct gotr_user *user, char *packed_msg, size_t len)
{
	return GOTR_OK;
}

int gotr_parse_flake_y(struct gotr_roomdata *room, struct gotr_user *user, char *packed_msg, size_t len)
{
	return GOTR_OK;
}

int gotr_parse_flake_V(struct gotr_roomdata *room, struct gotr_user *user, char *packed_msg, size_t len)
{
	return GOTR_OK;
}

int gotr_parse_flake_validation(struct gotr_roomdata *room, struct gotr_user *user, char *packed_msg, size_t len)
{
	return GOTR_OK;
}

int gotr_parse_msg(struct gotr_roomdata *room, char *packed_msg, size_t len)
{
	gotr_eprintf("got \"anonymous\" massage: %s", ++packed_msg);
	return GOTR_OK;
}
