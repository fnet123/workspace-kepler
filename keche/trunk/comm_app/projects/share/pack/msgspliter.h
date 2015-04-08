/*
 * msgspliter.h
 *
 *  Created on: 2013-1-7
 *      Author: humingqing
 *
 * ���ݰ���ַ�ʽ���д���
 */

#ifndef __MSGSPLITER_H__
#define __MSGSPLITER_H__

#include <protocol.h>
#include <SocketHandle.h>
#include <msgpack.h>

// �ְ�����
class CMsgSpliter : public IPackSpliter
{
public:
	CMsgSpliter() {}
	virtual ~CMsgSpliter() {}

	// �ְ�����
	struct packet * get_kfifo_packet( DataBuffer *fifo ) {
		unsigned int len = fifo->getLength() ;
		if ( len < (unsigned int) sizeof(CMsgHeader) ) {
			return NULL;
		}
		if ( len > MAX_PACK_LEN ) {
			fifo->resetBuf() ;
			return NULL ;
		}

		struct list_head *packet_list_ptr = NULL;
		char* p = (char *) fifo->getBuffer() ;

		int pos = 0 ;
		while( (unsigned int)( pos + sizeof(CMsgHeader) ) <= len ) {
			// ȡ�ýӵ�������
			CMsgHeader *header = ( CMsgHeader *) ( p + pos ) ;
			if ( ntohs(header->_ver) != MSG_VERSION ) {
				// ������յ������ݴ���ֱ�Ӷ���
				fifo->resetBuf() ;
				break ;
			}

			// ȡ��������ĳ���
			int msg_len  = ntohl( header->_len ) ;
			int pack_len = msg_len + (int)sizeof(CMsgHeader) ;

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

#endif /* MSGSPLITER_H_ */
