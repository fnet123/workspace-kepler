/*
 * interpacker.h
 *
 *  Created on: 2012-4-26
 *      Author: humingqing
 *  ���ܺͷǼ��ܴ���ְ�����
 */

#ifndef __INTERPACKER_H__
#define __INTERPACKER_H__

#include <stdio.h>
#include <protocol.h>
#include <SocketHandle.h>
#include <interheader.h>
#include <comlog.h>

//static int g_packcount = 0 ;
// �ְ�����
class CInterSpliter : public IPackSpliter
{
public:
	CInterSpliter() {}
	virtual ~CInterSpliter() {}

	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ) {
		unsigned int len = fifo->getLength() ;
		if ( len <= 0 || len > MAX_PACK_LEN ){
			fifo->resetBuf() ;
			return NULL;
		}

		unsigned char *p = NULL, *in_begin = NULL ;
		struct list_head *packet_list_ptr = NULL;

		unsigned int ndel = 0 ;
		unsigned int i = 0;
		in_begin = p = (unsigned char *) fifo->getBuffer() ;
		// OUT_HEX( NULL, 0, "Spliter", p, len ) ;
		while ( i < len && len > 2 ) {
			if ( *p == '\r' && *(p + 1) == '\n' ) {

				unsigned int pack_len = p - in_begin + 2;
				struct packet *item = (struct packet *) malloc(sizeof(struct packet));
				if (item == NULL) break ;

				item->data = (unsigned char *) malloc(pack_len + 3);
				memset(item->data, 0, pack_len + 3);
				//!! begin copy data from the second '[' and end with first ']'
				memcpy(item->data, in_begin, pack_len);
				item->len = pack_len;
				item->type = E_PROTO_IN;

				if (packet_list_ptr == NULL) {
					packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
					if (packet_list_ptr == NULL)
						return NULL;

					INIT_LIST_HEAD(packet_list_ptr);
				}
				list_add_tail(&item->list, packet_list_ptr);
				in_begin = p + 2 ;
				ndel     = i + 2 ;

				p = p + 1 ;
				i = i + 1 ;
				// ++ g_packcount ;
			}

			++ p ;
			++ i ;
		}
		// ��������������ݲ�Ϊ��
		if ( ndel > 0 ) {
			fifo->removePos( ndel );
		}
		//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);
		return (struct packet*) packet_list_ptr;
	}

	// �ͷ����ݰ�
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

class CInterSpliterEx : public IPackSpliter
{
public:
	CInterSpliterEx() {}
	virtual ~CInterSpliterEx() {}

	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ) {
		unsigned int len = fifo->getLength() ;
		if ( len <= 0 || len > MAX_PACK_LEN ){
			fifo->resetBuf() ;
			return NULL;
		}

		unsigned char *p = NULL, *in_begin = NULL ;
		struct list_head *packet_list_ptr = NULL;

		unsigned int ndel = 0 ;
		unsigned int i = 0;
		in_begin = p = (unsigned char *) fifo->getBuffer() ;
		// OUT_HEX( NULL, 0, "Spliter", p, len ) ;
		while ( i < len && len > 2 ) {
			if (*p == 0x5b ) {  // 0x5d

				interheader *header = (interheader *) p ;
				unsigned short nlen   = ntohs( header->len ) ;
				unsigned int pack_len = nlen + sizeof(interheader) + 1 ;

				if ( (i+pack_len) > len || *(p+pack_len-1) != 0x5d || pack_len > len || nlen > len ) {
					++ p ; ++ i ;
					continue ;
				}

				struct packet *item = (struct packet *) malloc(sizeof(struct packet));
				if (item == NULL) break ;

				item->data = (unsigned char *) malloc(pack_len+1);
				memset(item->data, 0, pack_len+1);
				//!! begin copy data from the second '[' and end with first ']'
				memcpy(item->data, p, pack_len);
				item->len = pack_len;
				item->type = E_PROTO_OUT;

				if (packet_list_ptr == NULL) {
					packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
					if (packet_list_ptr == NULL)
						return NULL;

					INIT_LIST_HEAD(packet_list_ptr);
				}
				list_add_tail(&item->list, packet_list_ptr);
				// OUT_INFO( NULL, 0, "Spliter", "split pack pack length: %d" , pack_len ) ;
				//��ת��']'����һ���ַ���
				in_begin = p + pack_len ;
				ndel = i + pack_len ;

				p = p + pack_len - 1 ;  // ֱ��ת��0x5d������ʶ����
				i = i + pack_len - 1 ;
				// ++ g_packcount ;

			} else if ( *p == '\r' && *(p + 1) == '\n' ) {

				unsigned int pack_len = p - in_begin + 2;
				struct packet *item = (struct packet *) malloc(sizeof(struct packet));
				if (item == NULL) break ;

				item->data = (unsigned char *) malloc(pack_len + 3);
				memset(item->data, 0, pack_len + 3);
				//!! begin copy data from the second '[' and end with first ']'
				memcpy(item->data, in_begin, pack_len);
				item->len = pack_len;
				item->type = E_PROTO_IN;

				if (packet_list_ptr == NULL) {
					packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
					if (packet_list_ptr == NULL)
						return NULL;

					INIT_LIST_HEAD(packet_list_ptr);
				}
				list_add_tail(&item->list, packet_list_ptr);
				in_begin = p + 2 ;
				ndel     = i + 2 ;

				p = p + 1 ;
				i = i + 1 ;
				// ++ g_packcount ;
			}

			++ p ;
			++ i ;
		}
		// ��������������ݲ�Ϊ��
		if ( ndel > 0 ) {
			fifo->removePos( ndel );
		}
		//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);
		return (struct packet*) packet_list_ptr;
	}

	// �ͷ����ݰ�
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

// ������ݲ�ִ���
class CBigSpliter : public IPackSpliter
{
public:
	CBigSpliter() {}
	virtual ~CBigSpliter() {}

	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ) {
		unsigned int len = fifo->getLength() ;
		if ( len <= 0 || len > MAX_PACK_LEN ){
			fifo->resetBuf() ;
			return NULL;
		}

		unsigned char *p = NULL, *in_begin = NULL ;
		struct list_head *packet_list_ptr = NULL;

		unsigned int ndel = 0 ;
		unsigned int i = 0;
		in_begin = p = (unsigned char *) fifo->getBuffer() ;
		// OUT_HEX( NULL, 0, "Spliter", p, len ) ;
		while ( i < len && len > 2 ) {
			if ( *p == BIG_VER_0 && *(p+1) == BIG_VER_1 ) {  // 0x01 0x01

				bigheader *header     = (bigheader *) p ;
				unsigned int nlen     = ntohl( header->len ) ;
				unsigned int pack_len = nlen + sizeof(bigheader) ;

				if ( (i+pack_len) > len || pack_len > len || nlen > len ) {
					break ;
				}

				struct packet *item = (struct packet *) malloc(sizeof(struct packet));
				if (item == NULL) break ;

				item->data = (unsigned char *) malloc(pack_len+1);
				memset(item->data, 0, pack_len+1);
				//!! begin copy data from the second '[' and end with first ']'
				memcpy(item->data, p, pack_len);
				item->len = pack_len;
				item->type = E_PROTO_OUT;

				if (packet_list_ptr == NULL) {
					packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
					if (packet_list_ptr == NULL)
						return NULL;

					INIT_LIST_HEAD(packet_list_ptr);
				}
				list_add_tail(&item->list, packet_list_ptr);
				// OUT_INFO( NULL, 0, "Spliter", "split pack pack length: %d" , pack_len ) ;
				//��ת��']'����һ���ַ���
				in_begin = p + pack_len ;

				p = p + pack_len ;  // ֱ��ת��0x5d������ʶ����
				i = i + pack_len ;

				ndel = i ;

			} else {

				++ p ;
				++ i ;
			}
		}
		// ��������������ݲ�Ϊ��
		if ( ndel > 0 ) {
			fifo->removePos( ndel );
		}
		//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);
		return (struct packet*) packet_list_ptr;
	}

	// �ͷ����ݰ�
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

// 808�������
class C808Spliter : public IPackSpliter
{
	static const int PACK_MIN_SIZE = 15;
public:
	C808Spliter() {};
	virtual ~C808Spliter(){};

	// ���808����
	struct packet* get_kfifo_packet( DataBuffer *fifo ) {
		int   pos = 0;
		int   len = fifo->getLength();
		char *ptr = fifo->getBuffer();
		if(len < PACK_MIN_SIZE) {
			return NULL;
		}

		if(len > MAX_SOCKET_BUF * 2) {
			fifo->resetBuf();
			return NULL;
		}

		struct packet *itemPtr = NULL;
		int            itemLen = 0;
		struct list_head *listPtr = NULL;

		int i;
		unsigned char tag, crc;
		unsigned char preCh, curCh;

		tag = crc = preCh = 0;
		for(i = 0; i < len; ++i) {
			curCh = ptr[i];

			if(curCh == 0x7d) {
				tag = 1;
				continue;
			}

			if(curCh != 0x7e) {
				crc ^= preCh;
				preCh = curCh;

				if(tag != 0) {
					switch(curCh) {
					case 0x01:
						preCh = 0x7d;
						break;
					case 0x02:
						preCh = 0x7e;
						break;
					}
				}

				tag = 0;
				continue;
			}

			if(itemLen == 0) {
				if(pos < i) {
					OUT_HEX(NULL, 0, "Parse808Err", ptr + pos, i - pos);
					pos = i;
				}
				itemLen = 1;
				tag = crc = preCh = 0;
				continue;
			}

			itemLen = i - pos + 1;
			if(itemLen < PACK_MIN_SIZE || crc != preCh) {
				OUT_HEX(NULL, 0, "Parse808Err", ptr + pos, i - pos);
				pos = i;
				itemLen = 1;
				tag = crc = preCh = 0;
				continue;
			}

			if(listPtr == NULL) {
				listPtr = (struct list_head*)malloc(sizeof(struct list_head));
				if(listPtr  == NULL) {
					break;
				}
				INIT_LIST_HEAD(listPtr);
			}

			itemPtr = (struct packet *) malloc(sizeof(struct packet));
			if(itemPtr == NULL) {
				break;
			}
			itemPtr->type = E_PROTO_OUT;
			itemPtr->len  = itemLen;
			itemPtr->data = (unsigned char*) malloc(itemLen);
			if(itemPtr->data == NULL) {
				free(itemPtr);
				break;
			}
			memcpy(itemPtr->data, ptr + pos, itemLen);
			list_add_tail(&itemPtr->list, listPtr);

			pos = i + 1;
			itemLen = 0;
			tag = crc = preCh = 0;
		}

		if(pos > 0) fifo->removePos(pos);

		return (struct packet*)listPtr;
	}

	// �ͷ�����
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

#endif /* INTERPACKER_H_ */
