/*
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id$
 *
 * Authors:
 *   duolong <duolong@taobao.com>
 * Editor:
 *   humingqing <2011-12-21>
 */

#include "filequeue.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <comlog.h>

#define CTFO_EXIT_FAILURE  -1
#define CTFO_EXIT_SUCCESS   0

#define INFO   "INFO"
#define ERROR  "ERROR"
#define WARN   "WARN"
#define SYS_LOG( name , fmt, ... )   OUT_PRINT( NULL, 0, name, fmt, ## __VA_ARGS__ )

/**
 * rootPath: queue��root·��
 * queueName:
 * maxFileSize: ����ļ���С
 */
CFileQueue::CFileQueue(char *rootPath, char *queueName, int maxFileSize)
{
	char tmp[256];
	sprintf(tmp, "%s/%s", rootPath, queueName);
	m_queuePath = strdup(tmp);
	m_maxFileSize = maxFileSize;
	m_infoFd = -1;
	m_readFd = -1;
	m_writeFd = -1;

	// ����Ŀ¼
	if (CFileUtil::mkdirs(m_queuePath) == false) {
		SYS_LOG(ERROR, "create directory failed: %s", m_queuePath);
		return;
	}

	// ͷ�ļ�
	sprintf(tmp, "%s/header.dat", m_queuePath);
	m_infoFd = open(tmp, O_RDWR|O_CREAT, 0644);
	if (m_infoFd == -1) {
		SYS_LOG(ERROR, "open file failed: %s", tmp);
		return;
	}

	// ����ͷ��Ϣ
	if (read(m_infoFd, &m_head, sizeof(m_head)) != sizeof(m_head)) {
		memset(&m_head, 0, sizeof(m_head));
		m_head.read_seqno = 1;
		m_head.write_seqno = 1;
	}

	// ��д
	if (openWriteFile() == CTFO_EXIT_FAILURE) {
		return;
	}

	struct stat st;
	if (fstat(m_writeFd, &st) == 0) {
		m_head.write_filesize = st.st_size;
	}

	// �򿪶�
	if (openReadFile() == CTFO_EXIT_FAILURE) {
		close(m_writeFd);
		m_writeFd = -1;
		return;
	}

	// �ָ���¼
	if (m_head.exit_status == 0) {
		recoverRecord();
	}
	m_head.exit_status = 0;
}
/**
 * ����
 */
CFileQueue::~CFileQueue(void)
{
	m_head.exit_status = 1;
	memset(&m_head.pos[0], 0, CTFO_MAX_THREAD_COUNT*sizeof(unsettle));
	writeHead();
	if (m_queuePath) {
		free(m_queuePath);
		m_queuePath = NULL;
	}
	if (m_infoFd != -1) {
		close(m_infoFd);
		m_infoFd = -1;
	}
	if (m_readFd != -1) {
		close(m_readFd);
		m_readFd = -1;
	}
	if (m_writeFd != -1) {
		close(m_writeFd);
		m_writeFd = -1;
	}
}

/**
 * д��һ��¼
 */
int CFileQueue::push(void *data, int len)
{
	if (m_writeFd == -1) {
		openWriteFile();
		if (m_writeFd == -1) {
			SYS_LOG(WARN, "open file failed: %s:%u", m_queuePath, m_head.write_seqno);
			return CTFO_EXIT_FAILURE;
		}
	}
	if (data == NULL || len == 0) {
		SYS_LOG(WARN, "data: %p, len: %d", data, len);
		return CTFO_EXIT_FAILURE;
	}
	int size = sizeof(int) + sizeof(queue_item) + len;
	if (size > CTFO_MAX_FILE_SIZE) {
		SYS_LOG(WARN, "size: %d", size);
		return CTFO_EXIT_FAILURE;
	}
	char *buffer = (char*)malloc(size);
	assert(buffer != NULL);
	*((int*)buffer) = size;
	queue_item *item = (queue_item*) (buffer + sizeof(int));
	item->len = len;
	item->flag = CTFO_FILE_QUEUE_FLAG;
	memcpy(&(item->data[0]), data, len);

	// ����ļ����������´�һ��
	if (m_head.write_filesize >= m_maxFileSize) {
		m_head.write_seqno ++;
		m_head.write_filesize = 0;
		openWriteFile();
		writeHead();
	}
	item->pos.seqno = m_head.write_seqno;
	item->pos.offset = m_head.write_filesize;
	int ret = write(m_writeFd, buffer, size);
	if (ret > 0) {
		m_head.write_filesize += size;
	}
	free(buffer);
	if (ret != size) { // дʧ��
		SYS_LOG(WARN, "write faild: %s, fd: %d, len: %d, %d<>%d",
			m_queuePath, m_writeFd, len, ret, size);
		ret = size - ret;
		if (ret>0 && ret<=size && size<m_maxFileSize) {
			SYS_LOG(WARN, "skip %d byte", ret);
			lseek(m_writeFd, ret, SEEK_CUR);
		}
		return CTFO_EXIT_FAILURE;
	}
	m_head.queue_size ++;
	return CTFO_EXIT_SUCCESS;
}

/**
 * ����һ��¼, ���ó���Ҫ�����ͷ�
 */
queue_item *CFileQueue::pop(uint32_t index)
{
	if (m_readFd == -1) {
		openReadFile();
		if (m_readFd == -1) {
			SYS_LOG(WARN, "open file failed: %s:%d", m_queuePath, m_head.read_seqno);
			return NULL;
		}
	}
	int ret, size = 0;
	queue_item *item = NULL;
	int retryReadCount = 0;
	index %= CTFO_MAX_THREAD_COUNT;

	while(retryReadCount<3) {
		int retSize = read(m_readFd, &size, sizeof(int));
		if (retSize < 0) {
			SYS_LOG(ERROR, "read error, m_readFd:%d, %s(%d)", m_readFd, strerror(errno), errno);
			break;
		}
		if (retSize == sizeof(int)) {
			size -= sizeof(int);
			// ���size
			if (size < (int)sizeof(queue_item) || size > (int)CTFO_MAX_FILE_SIZE) {
				int curPos = (int)lseek(m_readFd, 0-retSize, SEEK_CUR);
				SYS_LOG(WARN, "read size error:%d,curPos:%d,readOff:%d,retry:%d,seqno:%u,m_readFd:%d",
					size, curPos, m_head.read_offset, retryReadCount, m_head.read_seqno,m_readFd);
				retryReadCount ++;
				if (m_writeFd != -1 && m_head.read_seqno == m_head.write_seqno) {
					fsync(m_writeFd);
				}
				continue;
			}
			// �����ڴ�
			item = (queue_item*) malloc(size);
			assert(item != NULL);
			if ((ret = read(m_readFd, item, size)) != size) {
				// ���¶�
				int64_t curPos = lseek(m_readFd, 0-ret-retSize, SEEK_CUR);
				SYS_LOG(WARN, "read file error:%d<>%d,curPos:%d,readOff:%d,retry:%d,seqno:%u,m_readFd:%d",
					ret, size, (int)curPos, m_head.read_offset, retryReadCount, m_head.read_seqno,m_readFd);
				retryReadCount ++;
				free(item);
				item = NULL;
				if (m_writeFd != -1 && m_head.read_seqno == m_head.write_seqno) {
					fsync(m_writeFd);
				}
				continue;
			}
			// ���flag
			if (item->flag != CTFO_FILE_QUEUE_FLAG) {
				 // ���¶�
				int64_t curPos = lseek(m_readFd, 0-ret-retSize, SEEK_CUR);
				SYS_LOG(WARN, "flag error:item->flag(%d)<>FLAG(%d),curPos:%d,readOff:%d,retry:%d,seqno:%u,m_readFd:%d",
					item->flag, CTFO_FILE_QUEUE_FLAG, (int)curPos, m_head.read_offset,
					retryReadCount, m_head.read_seqno,m_readFd);
				retryReadCount ++;
				free(item);
				item = NULL;
				if (m_writeFd != -1 && m_head.read_seqno == m_head.write_seqno) {
					fsync(m_writeFd);
				}
				continue;
			}
			// ���len
			if (item->len + (int)sizeof(queue_item) != size) {
				// ���¶�
				int64_t curPos = lseek(m_readFd, 0-ret-retSize, SEEK_CUR);
				SYS_LOG(WARN, "read len error:%d<>%d,curPos:%d,readOff:%d,retry:%d,seqno:%u,m_readFd:%d",
					(int)(item->len + sizeof(queue_item)), size, (int)curPos, m_head.read_offset,
					retryReadCount, m_head.read_seqno,m_readFd);
				retryReadCount ++;
				free(item);
				item = NULL;
				if (m_writeFd != -1 && m_head.read_seqno == m_head.write_seqno) {
					fsync(m_writeFd);
				}
				continue;
			}

			retryReadCount = 0;
			m_head.pos[index].seqno = m_head.read_seqno;
			m_head.pos[index].offset = m_head.read_offset;
			m_head.read_offset += (size + sizeof(int));
			break;
		} else if (m_head.write_seqno > m_head.read_seqno) {
			// ɾ����������ļ�
			deleteReadFile();
			m_head.read_seqno ++;
			m_head.read_offset = 0;
			openReadFile();
			if (m_readFd == -1) {
				m_head.queue_size = 0;
				break;
			}
		} else {
			if (retSize > 0) {
				SYS_LOG(WARN, "IO busy,retSize:%d,seqno:%u", retSize, m_head.read_seqno);
				lseek(m_readFd, 0-retSize, SEEK_CUR);
			}
			m_head.queue_size = 0;
			break;
		}
	}
	if (retryReadCount>=3) { // ����ʧ��������,�Ƶ����
		backup(index);
		if (m_head.write_seqno > m_head.read_seqno)
		{
			deleteReadFile();
			m_head.read_seqno ++;
			m_head.read_offset = 0;
			openReadFile();
		} else {
			clear();
		}
	}
	writeHead();
	return item;
}

/**
 * ��ն���
 */
int CFileQueue::clear()
{
	m_head.write_seqno ++;
	m_head.read_seqno = m_head.write_seqno;
	m_head.read_offset = 0;
	m_head.write_filesize = 0;
	m_head.queue_size = 0;
	openWriteFile();
	openReadFile();
	return CTFO_EXIT_SUCCESS;
}

/**
 * �Ƿ����
 */
int CFileQueue::isEmpty()
{
	return (m_head.queue_size == 0 ? 1 : 0);
}

// ��ǰ�ļ����еĴ�С
int CFileQueue::size()
{
	return m_head.queue_size;
}

/**
 * ����������
 */
void CFileQueue::finish(uint32_t index) {
	unsettle *pos = &(m_head.pos[index % CTFO_MAX_THREAD_COUNT]);
	pos->seqno = 0;
	pos->offset = 0;
}

// ������������
void CFileQueue::remove()
{
   // m_head.write_seqno ++;
   m_head.read_seqno = m_head.write_seqno;
   m_head.read_offset = 0;
   m_head.write_filesize = 0;
   m_head.queue_size = 0;
   deleteReadFile(m_head.read_seqno);
   openWriteFile();
   openReadFile();
}

/**
 * �����ļ������к������
 */
void CFileQueue::backup(uint32_t index) {
	int curPos = (int)lseek(m_readFd, 0, SEEK_CUR);
	char cmd[512];
	sprintf(cmd, "cp -f %s/%08u.dat %s/%08u.dat.backup >/dev/null 2>&1 0>&1",
		m_queuePath, m_head.read_seqno,
		m_queuePath, m_head.read_seqno);
	int rest = system(cmd);
	SYS_LOG(WARN, "%s, readpos: %u:%d, currpos: %u:%d, curPos: %d, m_readFd: %d result: %d", cmd,
		m_head.pos[index].seqno, m_head.pos[index].offset,
		m_head.read_seqno, m_head.read_offset, curPos, m_readFd, rest);
}

/******************************************
 * �������ļ�д
 */
int CFileQueue::openWriteFile()
{
	if (m_writeFd != -1) {
		fsync(m_writeFd);
		close(m_writeFd);
		m_writeFd = -1;
	}
	char tmp[256];
	sprintf(tmp, "%s/%08u.dat", m_queuePath, m_head.write_seqno);
	m_writeFd = open(tmp, O_WRONLY|O_CREAT|O_APPEND, 0600);
	if (m_writeFd == -1) {
		SYS_LOG(ERROR, "open file failed: %s", tmp);
		return CTFO_EXIT_FAILURE;
	}
	return CTFO_EXIT_SUCCESS;
}
/**
 * ɾ����������ļ�
 */
int CFileQueue::deleteReadFile( uint32_t seq )
{
	if (m_readFd != -1) {
		close(m_readFd);
		m_readFd = -1;
	}

	char tmp[256];
	// ������Ϊ����ȡ��ǰ���
	if ( seq == 0 ) {
		// ɾ����ǰ��ȡ���ļ�
		sprintf(tmp, "%s/%08u.dat", m_queuePath, m_head.read_seqno - 1 );
		unlink(tmp);
		// SYS_LOG(INFO, "delete : %s", tmp);
	} else {
		// ɾ�������ļ�
		while( seq > 0 ) {
			sprintf(tmp, "%s/%08u.dat", m_queuePath, seq );
			unlink(tmp);
			// SYS_LOG(INFO, "delete : %s", tmp);

			-- seq ;
		}
	}
	return CTFO_EXIT_SUCCESS;
}
/**
 * �������ļ���
 */
int CFileQueue::openReadFile()
{
	if (m_readFd != -1) {
		close(m_readFd);
		m_readFd = -1;
	}
	char tmp[256];
	sprintf(tmp, "%s/%08u.dat", m_queuePath, m_head.read_seqno);
	m_readFd = open(tmp, O_RDONLY, 0600);
	if (m_readFd == -1) {
		SYS_LOG(ERROR, "open file failed: %s", tmp);
		return CTFO_EXIT_FAILURE;
	}
	lseek(m_readFd, m_head.read_offset, SEEK_SET);
	// SYS_LOG(INFO, "read next file:%s,wseq:%u,rseq:%u,roffset:%d,m_readFd:%d", tmp, m_head.write_seqno, m_head.read_seqno, m_head.read_offset, m_readFd);
	return CTFO_EXIT_SUCCESS;
}

/**
 * дhead
 */
int CFileQueue::writeHead()
{
	if (m_infoFd != -1) {
		lseek(m_infoFd, 0, SEEK_SET);
		if (write(m_infoFd, &m_head, sizeof(m_head)) != sizeof(m_head)) {
			SYS_LOG(ERROR, "write failed: %s", m_queuePath);
		}
	}
	return CTFO_EXIT_SUCCESS;
}
/**
 * �ָ���¼
 */
void CFileQueue::recoverRecord() {
	char tmp[256];
	int fd, size, count = 0;
	for(int i=0; i<CTFO_MAX_THREAD_COUNT; i++) {
		unsettle *pos = &(m_head.pos[i]);
		if (pos->seqno == 0) continue;
		if (pos->seqno > m_head.read_seqno) continue;
		if (pos->offset >= m_head.read_offset) continue;

		sprintf(tmp, "%s/%08u.dat", m_queuePath, pos->seqno);
		fd = open(tmp, O_RDONLY, 0600);
		if (fd == -1) continue;
		lseek(fd, pos->offset, SEEK_SET);
		if (read(fd, &size, sizeof(int)) == sizeof(int) &&
			size >= (int)sizeof(queue_item) &&
			size < (int)CTFO_MAX_FILE_SIZE) {
			size -= sizeof(int);
			queue_item *item = (queue_item*) malloc(size);
			assert(item != NULL);
			if (read(fd, item, size) == size && item->flag == CTFO_FILE_QUEUE_FLAG) {
				push(&(item->data[0]), item->len);
				count ++;
			}
			free(item);
		}
		close(fd);
	}
	// SYS_LOG(INFO, "%s: recoverRecord: %d", m_queuePath, count);
	memset(&m_head.pos[0], 0, CTFO_MAX_THREAD_COUNT*sizeof(unsettle));
	writeHead();
}

//////////////////END
