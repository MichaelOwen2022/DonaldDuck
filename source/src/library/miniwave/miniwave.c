/*
 * Copyright (c) 2022 - 2023, tangchunhui@coros.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "miniwave.h"

#define WAV_ERR(fmt, args...)	printf("[%s|%d]:" fmt "\r\n", __func__, __LINE__, ##args)
#define WAV_WRN(fmt, args...)	printf("[%s|%d]:" fmt "\r\n", __func__, __LINE__, ##args)
#define WAV_INF(fmt, args...)	printf("[%s|%d]:" fmt "\r\n", __func__, __LINE__, ##args)
#define WAV_DBG(fmt, args...)	printf("[%s|%d]:" fmt "\r\n", __func__, __LINE__, ##args)

/************************************************************************************************************************/

void miniwave_version(char *name, int *major, int *minor, char *date)
{
	if (name)
		strcpy(name, NAME_STRING);
	if (major)
		*major = VERSION_MAJOR;
	if (minor)
		*minor = VERSION_MINOR;
	if (date)
		strcpy(date, BUILD_DATE);
}

/************************************************************************************************************************/

struct RIFF_CHUNK
{
	char            riffType[4];    //4byte,资源交换文件标志:RIFF
    unsigned int    riffSize;       //4byte,从下个地址到文件结尾的总字节数
    char            waveType[4];    //4byte,wave文件标志:WAVE
};

struct FMTS_CHUNK
{
	char            formatType[4];  //4byte,波形文件标志:FMT
    unsigned int    formatSize;     //4byte,音频属性(compressionCode,numChannels,sampleRate,bytesPerSecond,blockAlign,bitsPerSample)所占字节数
    unsigned short  compressionCode;//2byte,编码格式(1-线性pcm-WAVE_FORMAT_PCM,WAVEFORMAT_ADPCM)
    unsigned short  numChannels;    //2byte,通道数
    unsigned int    sampleRate;     //4byte,采样率
    unsigned int    bytesPerSecond; //4byte,传输速率
    unsigned short  blockAlign;     //2byte,数据块的对齐
    unsigned short  bitsPerSample;  //2byte,采样精度
};

struct FACT_CHUNK
{
	char			factType[4];	// 4byte,
	unsigned int	factSize;
};

struct DATA_CHUNK
{
	char            dataType[4];    //4byte,数据标志:data
    unsigned int    dataSize;       //4byte,从下个地址到文件结尾的总字节数，即除了wav header以外的pcm data length
};

struct WAVE_HEADER
{
	struct RIFF_CHUNK	riff;
	struct FMTS_CHUNK	fmts;
	union {
		struct FACT_CHUNK fact;
		struct DATA_CHUNK data;
	};
};

#define RIFF_TYPE		"RIFF"
#define RIFF_TYPE_SIZE	strlen(RIFF_TYPE)
#define WAVE_TYPE		"WAVE"
#define WAVE_TYPE_SIZE	strlen(WAVE_TYPE)
#define FMTS_TYPE		"fmt "
#define FMTS_TYPE_SIZE	strlen(FMTS_TYPE)
#define FACT_TYPE		"fact"
#define FACT_TYPE_SIZE	strlen(FACT_TYPE)
#define DATA_TYPE		"data"
#define DATA_TYPE_SIZE	strlen(DATA_TYPE)

#define RIFF_CHUNK_SIZE	sizeof(struct RIFF_CHUNK)
#define FMTS_CHUNK_SIZE	sizeof(struct FMTS_CHUNK)
#define FACT_CHUNK_SIZE	sizeof(struct FACT_CHUNK)
#define DATA_CHUNK_SIZE	sizeof(struct DATA_CHUNK)

struct WAVE
{
	int file;
	unsigned int flags;
	struct WAVE_HEADER header;
	unsigned int dataOffset;
};

#define WAVE_O_INTERNAL  (1 << 31)

/************************************************************************************************************************/

static int wave2file_flags(int flags)
{
    if (flags & WAVE_O_WRONLY)
        flags = O_CREAT | O_TRUNC | O_WRONLY;
    else
        flags = O_RDONLY;

    return flags;
}

static int wave_chunk_read(int file, off_t offset, void *chunk, unsigned int size)
{
	int retval = 0;

	retval = lseek(file, offset, SEEK_SET);
	if (retval < 0)
	{
		WAV_ERR("lseek(%d, %d, SEEK_SET) fail[%d]", file, (int)offset, errno);
		return -errno;
	}

	retval = read(file, chunk, size);
	if (retval < 0)
	{
		WAV_ERR("read(%d, %p, %u) fail[%d]", file, chunk, size, errno);
		return -errno;
	}
	else if (retval != size)
	{
		WAV_WRN("read(%d, %p, %u) real[%d]", file, chunk, size, retval);
		return -EIO;
	}

	return retval;
}

static int wave_header_read(int file, struct WAVE_HEADER *header)
{
	struct RIFF_CHUNK *riff = &(header->riff);
	struct FMTS_CHUNK *fmts = &(header->fmts);
	struct FACT_CHUNK *fact = &(header->fact);
	struct DATA_CHUNK *data = &(header->data);
	off_t offset = 0;
	int retval = 0;

	retval = wave_chunk_read(file, offset, riff, RIFF_CHUNK_SIZE);
	if (retval < 0)
		return retval;

	if (memcmp(riff->riffType, RIFF_TYPE, RIFF_TYPE_SIZE))
	{
		WAV_ERR("Invalid riffType[%c%c%c%c]",
			riff->riffType[0], riff->riffType[1],
			riff->riffType[2], riff->riffType[3]);
		return -EPERM;
	}

	if (riff->riffSize <= sizeof(struct WAVE_HEADER) - 8)
	{
		WAV_ERR("Invalid riffSize[%u]", riff->riffSize);
		return -EPERM;
	}

	if (memcmp(riff->waveType, WAVE_TYPE, WAVE_TYPE_SIZE))
	{
		WAV_ERR("Invalid waveType[%c%c%c%c]",
			riff->waveType[0], riff->waveType[1],
			riff->waveType[2], riff->waveType[3]);
		return -EPERM;
	}

	offset += RIFF_CHUNK_SIZE;

	retval = wave_chunk_read(file, offset, fmts, FMTS_CHUNK_SIZE);
	if (retval < 0)
		return retval;

	if (memcmp(fmts->formatType, FMTS_TYPE, FMTS_TYPE_SIZE))
	{
		WAV_ERR("Invalid formatType[%c%c%c%c]",
			fmts->formatType[0], fmts->formatType[1],
			fmts->formatType[2], fmts->formatType[3]);
		return -EPERM;
	}

	if (fmts->formatSize < (FMTS_CHUNK_SIZE - 8))
	{
		WAV_ERR("Invalid formatSize[%u]", fmts->formatSize);
		return -EPERM;
	}

	offset += (fmts->formatSize + 8);

	retval = wave_chunk_read(file, offset, fact, FACT_CHUNK_SIZE);
	if (retval < 0)
		return retval;

	if (memcmp(fact->factType, FACT_TYPE, FACT_TYPE_SIZE) == 0)
	{
		offset += FACT_CHUNK_SIZE + fact->factSize;

		retval = wave_chunk_read(file, offset, data, DATA_CHUNK_SIZE);
		if (retval < 0)
			return retval;
	}

	if (memcmp(data->dataType, DATA_TYPE, DATA_TYPE_SIZE))
	{
		WAV_ERR("Invalid dataType[%c%c%c%c]",
				data->dataType[0], data->dataType[1],
				data->dataType[2], data->dataType[3]);
		return -EPERM;
	}

	offset += DATA_CHUNK_SIZE;

	return (int)offset;
}

static int wave_header_write(int file, struct WAVE_HEADER *header)
{
	int retval = 0;

	retval = lseek(file, 0, SEEK_SET);
	if (retval < 0)
	{
		WAV_ERR("lseek(%d, 0, SEEK_SET) fail[%d]", file, errno);
		retval = -errno;
		goto ERR_EXIT;
	}

	retval = write(file, header, sizeof(struct WAVE_HEADER));
	if (retval < 0)
	{
		WAV_ERR("write(%d, %p, %lu) fail[%d]", \
			file, header, sizeof(struct WAVE_HEADER), errno);
		retval = -errno;
	}
	else if (retval != sizeof(struct WAVE_HEADER))
	{
		WAV_WRN("write(%d, %p, %lu) real[%d]", \
			file, header, sizeof(struct WAVE_HEADER), retval);
		retval = -EIO;
	}

ERR_EXIT:
	retval = lseek(file, 0, SEEK_END);
	if (retval < 0)
	{
		WAV_ERR("lseek(%d, 0, SEEK_END) fail[%d]", file, errno);
		retval = -errno;
		goto ERR_EXIT;
	}

	return retval;
}

static void miniwave_dump(struct WAVE *wave)
{
	struct RIFF_CHUNK *riff = &(wave->header.riff);
	struct FMTS_CHUNK *fmts = &(wave->header.fmts);
	struct DATA_CHUNK *data = &(wave->header.data);

	WAV_INF("####################################");
	WAV_INF("wave file[%d]", wave->file);
	WAV_INF("wave flags[0x%08x]", wave->flags);

	WAV_INF("riffType[%c%c%c%c]",
			riff->riffType[0], riff->riffType[1],
			riff->riffType[2], riff->riffType[3]);
	WAV_INF("riffSize[%u]", riff->riffSize);
	WAV_INF("waveType[%c%c%c%c]",
			riff->waveType[0], riff->waveType[1],
			riff->waveType[2], riff->waveType[3]);

	WAV_INF("formatType[%c%c%c%c]",
			fmts->formatType[0], fmts->formatType[1],
			fmts->formatType[2], fmts->formatType[3]);
	WAV_INF("formatSize[%u]", fmts->formatSize);
	WAV_INF("compressionCode[%u]", fmts->compressionCode);
	WAV_INF("numChannels[%u]", fmts->numChannels);
	WAV_INF("sampleRate[%u]", fmts->sampleRate);
	WAV_INF("bytesPerSecond[%u]", fmts->bytesPerSecond);
	WAV_INF("blockAlign[%u]", fmts->blockAlign);
	WAV_INF("bitsPerSample[%u]", fmts->bitsPerSample);

	WAV_INF("dataType[%c%c%c%c]",
			data->dataType[0], data->dataType[1],
			data->dataType[2], data->dataType[3]);
	WAV_INF("dataSize[%u]", data->dataSize);

	WAV_INF("wave dataOffset[%u]", wave->dataOffset);
}

static void miniwave_attr_dump(WAV_ATTR *attr)
{
	WAV_INF("====================================");
	WAV_INF("samprate = %u", attr->samprate);
	WAV_INF("sampbits = %u", attr->sampbits);
	WAV_INF("channels = %u", attr->channels);
	WAV_INF("dataoffs = %u", attr->dataoffs);
	WAV_INF("datasize = %u", attr->datasize);
}

/************************************************************************************************************************/

WAV miniwave_open(const char *name, int flags, WAV_ATTR *attr)
{
    int file = 0;

    file = open(name, wave2file_flags(flags), 0755);
    if (file == 0)
    {
        WAV_ERR("open(%s, 0x%02x) fail[%d]",
                name, wave2file_flags(flags), file);
        return (WAV)NULL;
    }

    flags |= WAVE_O_INTERNAL;

    return miniwave_open_with_file(file, flags, attr);
}

WAV miniwave_open_with_file(int file, int flags, WAV_ATTR *attr)
{
    struct WAVE *wave = NULL;
    struct WAVE_HEADER *header = NULL;
	int retval = 0;

	if ((file < 0) || (attr == NULL))
	{
		WAV_ERR("Invalid file[%d] attr[%p]", file, attr);
		return (WAV)NULL;
	}

    wave = (struct WAVE *)malloc(sizeof(struct WAVE));
    if (wave == NULL)
    {
        WAV_ERR("malloc(%lu) fail", sizeof(struct WAVE));
        goto ERR_EXIT;
    }

    wave->file  = file;
    wave->flags = flags;

	retval = lseek(file, 0, SEEK_SET);
	if (retval < 0)
	{
		WAV_ERR("lseek(%d, 0, SEEK_SET) fail[%d]", file, errno);
		goto ERR_EXIT;
	}

    header = &(wave->header);

    if (flags & WAVE_O_WRONLY)
    {
        memcpy(header->riff.riffType, RIFF_TYPE, RIFF_TYPE_SIZE);
        header->riff.riffSize = sizeof(struct WAVE) - 8;
        memcpy(header->riff.waveType, WAVE_TYPE, WAVE_TYPE_SIZE);
        memcpy(header->fmts.formatType, FMTS_TYPE, FMTS_TYPE_SIZE);
        header->fmts.formatSize = 16;
        header->fmts.compressionCode = 1;
        header->fmts.numChannels = attr->channels;
        header->fmts.sampleRate = attr->samprate;
        header->fmts.bytesPerSecond = attr->samprate * \
			attr->channels * attr->sampbits / 8;
        header->fmts.blockAlign = attr->samprate * attr->channels / 8;
        header->fmts.bitsPerSample = attr->sampbits;
        memcpy(header->data.dataType, DATA_TYPE, DATA_TYPE_SIZE);
        header->data.dataSize = 0;

		retval = wave_header_write(file, header);
		if (retval < 0)
			goto ERR_EXIT;

		wave->dataOffset = sizeof(struct WAVE_HEADER);
    }
    else
    {
		retval = wave_header_read(file, header);
		if (retval < 0)
			goto ERR_EXIT;

		wave->dataOffset = retval;
    }

	miniwave_dump(wave);
	miniwave_attr((WAV)wave, attr);

	return (WAV)wave;

ERR_EXIT:
	if (flags & WAVE_O_INTERNAL)
		close(file);

	if (wave)
		free(wave);

	return (WAV)NULL;
}

int miniwave_attr(WAV wav, WAV_ATTR *attr)
{
	struct WAVE *wave = (struct WAVE *)wav;
	struct FMTS_CHUNK *fmts = NULL;
	struct DATA_CHUNK *data = NULL;
	unsigned int offset = 0;

	if ((wav == NULL) || (attr == NULL))
	{
		WAV_ERR("Invalid wav[%p] attr[%p]", wav, attr);
		return -EINVAL;
	}

	offset = lseek(wave->file, 0, SEEK_CUR);
	if (offset < 0)
	{
		WAV_ERR("lseek(%d, 0, SEEK_CUR) fail[%d]", wave->file, errno);
		return -errno;
	}

	fmts = &(wave->header.fmts);
	data = &(wave->header.data);

	attr->samprate = fmts->sampleRate;
	attr->channels = fmts->numChannels;
	attr->sampbits = fmts->bytesPerSecond / \
			fmts->sampleRate / fmts->numChannels * 8;
	attr->dataoffs = (offset > wave->dataOffset) ? \
			(offset - wave->dataOffset) : 0;
	attr->datasize = data->dataSize;

	miniwave_attr_dump(attr);

	return 0;
}

int miniwave_read(WAV wav, void *buf, int len)
{
	struct WAVE *wave = (struct WAVE *)wav;
	struct FMTS_CHUNK *fmts = NULL;
	struct DATA_CHUNK *data = NULL;
	int databytes = 0;
	int offset = 0;
	int retval = 0;

	if ((wav == NULL) || (buf == NULL) || (len <= 0))
	{
		WAV_ERR("Invalid wav[%p] buf[%p] len[%d]", wav, buf, len);
		return -EINVAL;
	}

	if (!(wave->flags & WAVE_O_RDONLY))
	{
		WAV_ERR("Can't read wave file");
		return -EPERM;
	}

	fmts = &(wave->header.fmts);
	data = &(wave->header.data);

	databytes = fmts->bytesPerSecond / \
				fmts->sampleRate / \
				fmts->numChannels;

	if (databytes <= 0 || databytes > 4)
	{
		WAV_ERR("Invald databytes[%d]", databytes);
		return -EINVAL;
	}

	if (len % (databytes * fmts->numChannels))
	{
		WAV_ERR("Invalid len[%d] databytes[%d] channels[%u]",
			len, databytes, fmts->numChannels);
		return -EINVAL;
	}

	offset = lseek(wave->file, 0, SEEK_CUR);
	if (offset < 0)
	{
		WAV_ERR("lseek(%d, 0, SEEK_CUR) fail[%d]", wave->file, errno);
		return -errno;
	}

	if (offset >= (wave->dataOffset + data->dataSize))
	{
		WAV_INF("end of read wave file");
		return 0;
	}

	if (len > (data->dataSize + wave->dataOffset - offset))
		len = (data->dataSize + wave->dataOffset - offset);

	retval = read(wave->file, buf, len);
	if (retval < 0)
	{
		WAV_ERR("read(%d, %p, %d) fail[%d]", wave->file, buf, len, errno);
		return -errno;
	}

	return retval;
}

int miniwave_write(WAV wav, void *buf, int len)
{
	struct WAVE *wave = (struct WAVE *)wav;
	struct RIFF_CHUNK *riff = NULL;
	struct FMTS_CHUNK *fmts = NULL;
	struct DATA_CHUNK *data = NULL;
	int databytes = 0;
	int second = 0;
	int retval = 0;

	if ((wav == NULL) || (buf == NULL) || (len <= 0))
	{
		WAV_ERR("Invalid wav[%p] buf[%p] len[%d]", wav, buf, len);
		return -EINVAL;
	}

	if (!(wave->flags & WAVE_O_WRONLY))
	{
		WAV_ERR("Can't write wave file");
		return -EPERM;
	}

	riff = &(wave->header.riff);
	fmts = &(wave->header.fmts);
	data = &(wave->header.data);

	databytes = fmts->bytesPerSecond / \
				fmts->sampleRate / \
				fmts->numChannels;

	if (databytes <= 0 || databytes > 4)
	{
		WAV_ERR("Invald databytes[%d]", databytes);
		return -EINVAL;
	}

	if (len % (databytes * fmts->numChannels))
	{
		WAV_ERR("Invalid len[%d] databytes[%d] channels[%u]",
			len, databytes, fmts->numChannels);
		return -EINVAL;
	}

	retval = write(wave->file, buf, len);
	if (retval < 0)
	{
		WAV_ERR("write(%d, %p, %d) fail[%d]", wave->file, buf, len, errno);
		return -errno;
	}

	second = data->dataSize / fmts->bytesPerSecond;

	riff->riffSize += retval;
	data->dataSize += retval;

	if ((data->dataSize / fmts->bytesPerSecond) != second)
	{
		wave_header_write(wave->file, &(wave->header));
	}

	return retval;
}

int miniwave_close(WAV wav)
{
	struct WAVE *wave = (struct WAVE *)wav;

	if (wav == NULL)
	{
		WAV_ERR("Invalid wav[%p]", wav);
		return -EINVAL;
	}

	if (wave->flags & WAVE_O_WRONLY)
		wave_header_write(wave->file, &(wave->header));

	if (wave->flags & WAVE_O_INTERNAL)
		close(wave->file);

	if (wave)
		free(wave);

	return 0;
}
