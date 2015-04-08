/*
 * packspliter.h
 *
 *  Created on: 2011-11-10
 *      Author: humingqing
 *      ����ڴ�ֲ�ʽ����Э��ķְ�����
 */

#ifndef __PACKSPLITER_H__
#define __PACKSPLITER_H__

#include <stdio.h>
#include <protocol.h>
#include <SocketHandle.h>
#include <nodeheader.h>

// �ְ�����
class CPackSpliter : public IPackSpliter
{
public:
	CPackSpliter() {}
	virtual ~CPackSpliter() {}

	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ) {
		unsigned int len = fifo->getLength() ;
		if ( len < (unsigned int) sizeof(NodeHeader) ) {
			return NULL;
		}
		if ( len > MAX_PACK_LEN ) {
			fifo->resetBuf() ;
			return NULL ;
		}

		struct list_head *packet_list_ptr = NULL;
		char* p = (char *) fifo->getBuffer() ;

		int pos = 0 ;
		while( (unsigned int)( pos + sizeof(NodeHeader) ) <= len ) {
			// ȡ�ýӵ�������
			NodeHeader *header = ( NodeHeader *) ( p + pos ) ;
			if ( strncmp( (char*)header->tag, NODE_CTFO_TAG , 4 ) != 0 ) {
				// ������յ������ݴ���ֱ�Ӷ���
				fifo->resetBuf() ;
				break ;
			}

			// ȡ��������ĳ���
			int msg_len  = ntohl( header->len ) ;
			int pack_len = msg_len + (int)sizeof(NodeHeader) ;

			// ���ݳ��Ȳ���ȷ����Ҫ������
			if ( pack_len + pos > (int)len ) break ;

			struct packet *item = (struct packet *) malloc(sizeof(struct packet));
			if (item == NULL)
				return (struct packet *) packet_list_ptr;
			item->data = (unsigned char *) malloc(pack_len+1);
			memset(item->data, 0, pack_len+1);
			//!! begin copy data from the second '[' and end with first ']'
			memcpy( item->data, p+pos, pack_len ) ;
			item->len  = pack_len;
			item->type = E_PROTO_OUT;

			if (packet_list_ptr == NULL) {
				packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
				if (packet_list_ptr == NULL)
					return NULL;

				INIT_LIST_HEAD(packet_list_ptr);
			}

			list_add_tail(&item->list, packet_list_ptr);

			pos = pos + pack_len ;
		}

		// �������������ֱ���Ƴ��ѽ���������
		if( pos > 0 ) {
			// �Ƴ��ѽ���������
			fifo->removePos(pos);
		}
		//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);

		return (struct packet*) packet_list_ptr;
	}

	// �ͷ����ݰ�
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

// ���\r\n�������
class CBrPackSpliter : public IPackSpliter
{
public:
	CBrPackSpliter() {}
	virtual ~CBrPackSpliter() {}
	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ){
		unsigned int len = fifo->getLength() ;
		if ( len <= 0 || len > MAX_PACK_LEN ){
			fifo->resetBuf() ;  // ������С������BUF
			return NULL;
		}

		struct list_head *packet_list_ptr = NULL ;

		unsigned int dlen = 0 ;

		char *ptr = (char *)fifo->getBuffer() ;
		char *end = (char *)( ptr + len );
		char * p  = strstr( ptr, "\r\n" ) ;
		while ( p != NULL && p < end-1 ) {
			int pack_len = p - ptr ;
			dlen = dlen + pack_len + 2 ;

			struct packet *item = (struct packet *) malloc(sizeof(struct packet));
			if (item == NULL)
				return (struct packet *) packet_list_ptr;

			item->data = (unsigned char *) malloc(pack_len + 3);
			memset(item->data, 0, pack_len + 3);
			memcpy(item->data, ptr, pack_len);
			item->len = pack_len;

			if (packet_list_ptr == NULL){
				packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
				if (packet_list_ptr == NULL)
					return NULL;

				INIT_LIST_HEAD(packet_list_ptr);
			}
			list_add_tail(&item->list, packet_list_ptr);
			// �������Ҫ��־ͼ�������
			ptr  = p + 2 ;
			if ( ptr >= end || dlen >= len )  break ;
			p    = strstr( ptr, "\r\n" ) ;
		};

		if ( dlen > 0 ) {
			fifo->removePos( dlen ) ;
		}
		// printf("get packet total %d packets\n", counter);

		return (struct packet*) packet_list_ptr;
	}

	// �ͷ����ݰ�
	void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}
};

#endif /* PACKSPLITER_H_ */
